#include "InitDX.h"

#include "ECS/RenderSystem.h"

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

static void loadTextures(DX12Render* rd, DX12Context* dx)
{
	LoadTextureFromFile("grassTex", L"Textures/grass.dds", dx, rd);
	LoadTextureFromFile("waterTex", L"Textures/water1.dds", dx, rd);
	LoadTextureFromFile("fenceTex", L"Textures/WoodCrate01.dds", dx, rd);
}

static void buildRenderItems(DX12Render* rd)
{
	RenderItemDesc desc;
	desc.GeometryName = "waterGeo";
	desc.MaterialName = "water";
	desc.SubMeshName = "grid";
	desc.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	desc.Layer = RenderLayer::Opaque;
	CreateRenderItem(&desc, rd, 0, &gObjects);

	desc.GeometryName = "landGeo";
	desc.MaterialName = "grass";
	CreateRenderItem(&desc, rd, 1, &gObjects);

	desc.GeometryName = "boxGeo";
	desc.MaterialName = "wirefence";
	desc.SubMeshName = "box";
	CreateRenderItem(&desc, rd, 2, &gObjects);

	//XMStoreFloat4x4(&boxRitem.WorldPos, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
}

static void buildBoxGeo(DX12Render* rd, DX12Context* dxC)
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

	Submesh sm = { box.Indicies.size(),0,0 };
	std::string subMeshName = "box";
	GeoBuildInfo gInfo;
	gInfo.Name = "boxGeo";
	gInfo.verts = vertices.data();
	gInfo.vertCount = vertices.size();
	gInfo.indicies = box.Indicies.data();
	gInfo.indiceCount = box.Indicies.size();
	gInfo.SubmeshNames = &subMeshName;
	gInfo.submeshs = &sm;
	gInfo.submeshCount = 1;

	CreateGeometry(&gInfo, dxC, rd);
}

static void buildLandGeometry(DX12Render* rd, DX12Context* dxC)
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

	Submesh sm = { grid.Indicies.size(),0,0 };
	std::string subMeshName = "grid";
	GeoBuildInfo gInfo;
	gInfo.Name = "landGeo";
	gInfo.verts = vertices.data();
	gInfo.vertCount = vertices.size();
	gInfo.indicies = grid.Indicies.data();
	gInfo.indiceCount = grid.Indicies.size();
	gInfo.SubmeshNames = &subMeshName;
	gInfo.submeshCount = 1;
	gInfo.submeshs = &sm;
	CreateGeometry(&gInfo, dxC, rd);
}

static void buildMaterials(DX12Render* rd)
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

	rd->AddMaterial("grass", grass);
	rd->AddMaterial("water", water);
	rd->AddMaterial("wirefence", wirefence);
}

static void buildWavesGeometryBuffers(DX12Render* rd, DX12Context* dxC, Waves* gWaves)
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

	Submesh submesh;
	submesh.IndexCount = indicesCount;
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	std::string submeshName = "grid";
	DynamicGeoBuildInfo info;

	info.Name = "waterGeo";
	info.SubmeshNames = &submeshName;
	info.submeshs = &submesh;
	info.verts = nullptr;
	info.vertCount = 0;

	info.indicies = indices.data();
	info.indiceCount = indices.size();
	info.indexByteSize = ibByteSize;
	info.vertexByteSize = vbByteSize;
	info.submeshCount = 1;

	CreateDynamicGeometry(&info, dxC, rd);
}

void setUpECS()
{
	gObjects.addEntity("water");
	gObjects.addEntity("Land");
	gObjects.addEntity("box");
	gObjects.addEntity("camera");
}

int main()
{
	Waves wave(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	DX12Context* dxC = new DX12Context(1280, 720, "Winnidow");

	DX12Render* render = new DX12Render(dxC, &wave);

	buildLandGeometry(render, dxC);
	buildWavesGeometryBuffers(render, dxC, &wave);
	buildBoxGeo(render, dxC);
	loadTextures(render, dxC);
	buildMaterials(render);
	setUpECS();
	buildRenderItems(render);

	render->FinishSetup();

	GuiSystem gui;
	RenderSystem rSys(&gObjects, render);
	ColorChangerSystem color(&rSys, &gObjects);
	CameraSystem cs(&gObjects, render);
	WaterAnimationSystem WaterAnimation(&gObjects, render->gWaves, &rSys);
	color.registerItem(0);
	color.registerItem(2);
	cs.AddObjectToSystem(3);
	WaterAnimation.AddToSystem(0);

	while (render->isWindowActive())
	{
		gui.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		color.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		WaterAnimation.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		cs.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		rSys.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
	}

	return 0;
}