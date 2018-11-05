#include "InitDX.h"
#include "Ext/DDSTextureLoader.h"

static float GetHillsHeight(float x, float z)
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}

static XMFLOAT3 GetHillsNormal(float x, float z)
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

static void loadTextures(DX12Render& rd, DX12Context& dx)
{
	Texture grassTex;
	grassTex.Filename = L"Textures/grass.dds";
	HR(CreateDDSTextureFromFile12(dx.mD3dDevice.Get(),
		dx.mCmdList.Get(), grassTex.Filename.c_str(),
		grassTex.Resource, grassTex.UploadHeap));

	Texture waterTex;
	waterTex.Filename = L"Textures/water1.dds";
	HR(CreateDDSTextureFromFile12(dx.mD3dDevice.Get(),
		dx.mCmdList.Get(), waterTex.Filename.c_str(),
		waterTex.Resource, waterTex.UploadHeap));

	Texture fenceTex;
	fenceTex.Filename = L"Textures/WoodCrate01.dds";
	HR(CreateDDSTextureFromFile12(dx.mD3dDevice.Get(),
		dx.mCmdList.Get(), fenceTex.Filename.c_str(),
		fenceTex.Resource, fenceTex.UploadHeap));

	rd.AddTexture("grassTex", grassTex);
	rd.AddTexture("waterTex", waterTex);
	rd.AddTexture("fenceTex", fenceTex);
}

static void buildRenderItems(DX12Render& rd)
{
	MeshGeometry* md = rd.GetGeometry("waterGeo");
	RenderItem wavesRitem;
	wavesRitem.WorldPos = Identity4x4();
	wavesRitem.ObjCBIndex = 0;
	wavesRitem.MatCBIndex = rd.GetMaterial("water")->MatCBIndex;
	wavesRitem.texHeapIndex = rd.GetMaterial("water")->DiffuseSrvHeapIndex;
	wavesRitem.GeoIndex = md->GeometryIndex;
	wavesRitem.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem.IndexCount = md->Submeshes["grid"].IndexCount;
	wavesRitem.StartIndexLocation = md->Submeshes["grid"].StartIndexLocation;
	wavesRitem.baseVertexLocation = md->Submeshes["grid"].BaseVertexLocation;

	rd.AddRenderItem("wavesRitem", wavesRitem, RenderLayer::Opaque);
	gWavesRItem = rd.GetRenderItem("wavesRitem");

	md = rd.GetGeometry("landGeo");
	RenderItem gridRitem;
	gridRitem.WorldPos = Identity4x4();
	gridRitem.ObjCBIndex = 1;
	gridRitem.MatCBIndex = rd.GetMaterial("grass")->MatCBIndex;
	gridRitem.texHeapIndex = rd.GetMaterial("grass")->DiffuseSrvHeapIndex;
	gridRitem.GeoIndex = md->GeometryIndex;
	gridRitem.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem.IndexCount = md->Submeshes["grid"].IndexCount;
	gridRitem.StartIndexLocation = md->Submeshes["grid"].StartIndexLocation;
	gridRitem.baseVertexLocation = md->Submeshes["grid"].BaseVertexLocation;

	rd.AddRenderItem("gridItem", gridRitem, RenderLayer::Opaque);

	md = rd.GetGeometry("boxGeo");
	RenderItem boxRitem;
	XMStoreFloat4x4(&boxRitem.WorldPos, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	boxRitem.ObjCBIndex = 2;
	boxRitem.MatCBIndex = rd.GetMaterial("wirefence")->MatCBIndex;
	boxRitem.texHeapIndex = rd.GetMaterial("wirefence")->DiffuseSrvHeapIndex;
	boxRitem.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem.GeoIndex = md->GeometryIndex;
	boxRitem.IndexCount = md->Submeshes["box"].IndexCount;
	boxRitem.StartIndexLocation = md->Submeshes["box"].StartIndexLocation;
	boxRitem.baseVertexLocation = md->Submeshes["box"].BaseVertexLocation;

	rd.AddRenderItem("boxItem", boxRitem, RenderLayer::Opaque);
}

struct GeoBuildInfo
{
	Vertex* verts;
	u16* indicies;
	u32 vertCount;
	u32 indiceCount;
};

static void AllocateGeometry(GeoBuildInfo* geoInfo, MeshGeometry* meshGeo, DX12Context* dxC)
{
	const u32 vbByteSize = geoInfo->vertCount * sizeof(Vertex);
	const u32 ibByteSize = geoInfo->indiceCount * sizeof(u16);

	HR(D3DCreateBlob(vbByteSize, &meshGeo->VertexBufferCPU));
	CopyMemory(meshGeo->VertexBufferCPU->GetBufferPointer(), geoInfo->verts, vbByteSize);

	HR(D3DCreateBlob(ibByteSize, &meshGeo->IndexBufferCPU));
	CopyMemory(meshGeo->IndexBufferCPU->GetBufferPointer(), geoInfo->indicies, ibByteSize);

	meshGeo->VertexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
		geoInfo->verts, vbByteSize, meshGeo->VertexBufferUploader);

	meshGeo->IndexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
		geoInfo->indicies, ibByteSize, meshGeo->IndexBufferUploader);

	meshGeo->VertexByteStride = sizeof(Vertex);
	meshGeo->VertexBufferByteSize = vbByteSize;
	meshGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	meshGeo->IndexBufferByteSize = ibByteSize;
}
static void buildBoxGeo(DX12Render& rd, DX12Context* dxC)
{
	GeometryGenerator geoGen;
	MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexCoord;
	}

	MeshGeometry geo;
	GeoBuildInfo gInfo = { vertices.data(),box.Indicies.data(),vertices.size(),box.Indicies.size() };
	AllocateGeometry(&gInfo, &geo, dxC);

	Submesh submesh;
	submesh.IndexCount = box.Indicies.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo.Submeshes["box"] = submesh;

	rd.AddGeometry("boxGeo", geo);
}
static void buildLandGeometry(DX12Render& rd, DX12Context* dxC)
{
	GeometryGenerator geoGen;
	MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	std::vector<Vertex> vertices;
	for (u32 i = 0; i < grid.Vertices.size(); i++)
	{
		Vertex v;
		v.Pos = grid.Vertices[i].Position;
		v.Pos.y = GetHillsHeight(v.Pos.x, v.Pos.z);
		v.Normal = GetHillsNormal(v.Pos.x, v.Pos.z);
		v.TexC = grid.Vertices[i].TexCoord;
		vertices.push_back(v);
	}

	MeshGeometry geo;

	GeoBuildInfo gInfo = { vertices.data(),grid.Indicies.data(),vertices.size(),grid.Indicies.size() };
	AllocateGeometry(&gInfo, &geo, dxC);

	Submesh submesh;
	submesh.IndexCount = grid.Indicies.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo.Submeshes["grid"] = submesh;

	rd.AddGeometry("landGeo", geo);
}

static void buildMaterials(DX12Render& rd)
{
	Material grass;
	//"grass";
	grass.MatCBIndex = 0;
	grass.DiffuseSrvHeapIndex = 0;
	grass.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass.FresnelR0 = { 0.01f, 0.01f, 0.01f };
	grass.Roughness = 0.125f;

	Material water;
	// 	water.Name = "water";
	water.MatCBIndex = 1;
	water.DiffuseSrvHeapIndex = 1;
	water.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	water.FresnelR0 = { 0.2f, 0.2f, 0.2f };
	water.Roughness = 0.0f;

	Material wirefence;
	wirefence.MatCBIndex = 2;
	wirefence.DiffuseSrvHeapIndex = 2;
	wirefence.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	wirefence.Roughness = 0.25f;

	rd.AddMaterial("grass", grass);
	rd.AddMaterial("water", water);
	rd.AddMaterial("wirefence", wirefence);
}

static void buildWavesGeometryBuffers(DX12Render& rd, DX12Context* dxC)
{
	u32 indicesCount = 3 * gWaves->TriangleCount();
	std::vector<u16> indices(indicesCount);
	s32 m = gWaves->RowCount();
	s32 n = gWaves->ColumnCount();
	s32 k = 0;

	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	u32 vbByteSize = gWaves->VertexCount() * sizeof(Vertex);
	u32 ibByteSize = indicesCount * sizeof(u16);

	MeshGeometry geo;
	/*geo->Name = "waterGeo";*/

	// Set dynamically.
	geo.VertexBufferCPU = nullptr;
	geo.VertexBufferGPU = nullptr;

	HR(D3DCreateBlob(ibByteSize, &geo.IndexBufferCPU));
	CopyMemory(geo.IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo.IndexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(),
		dxC->mCmdList.Get(), indices.data(), ibByteSize, geo.IndexBufferUploader);

	geo.VertexByteStride = sizeof(Vertex);
	geo.VertexBufferByteSize = vbByteSize;
	geo.IndexFormat = DXGI_FORMAT_R16_UINT;
	geo.IndexBufferByteSize = ibByteSize;

	Submesh submesh;
	submesh.IndexCount = indicesCount;
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo.Submeshes["grid"] = submesh;

	rd.AddGeometry("waterGeo", geo);
}

int main()
{
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	DX12Context* dxC = new DX12Context(1280, 720, "Winnidow");

	DX12Render* render = new DX12Render(dxC);

	buildLandGeometry(*render, dxC);
	buildWavesGeometryBuffers(*render, dxC);
	buildBoxGeo(*render, dxC);
	loadTextures(*render, *dxC);
	buildMaterials(*render);
	buildRenderItems(*render);
	render->FinishSetup();
	Material* grass = render->GetMaterial("grass");
	XMFLOAT4 grassColor = grass->DiffuseAlbedo;
	while (render->isWindowActive())
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX12_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);
			ImGui::ColorPicker4("Pick Color", &grassColor.x);
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		grass->DiffuseAlbedo = grassColor;
		grass->NumFramesDirty = gNumFrameResources;
		render->Update();
		render->Draw();
	}

	return 0;
}