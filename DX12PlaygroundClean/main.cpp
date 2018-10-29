#include "InitDX.h"
#include "DynamicArray.h"

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

static void buildRenderItems(DX12Render& rd)
{
	MeshGeometry* md = rd.GetGeometry("waterGeo");
	RenderItem wavesRitem;
	wavesRitem.WorldPos = Identity4x4();
	wavesRitem.ObjCBIndex = 0;
	wavesRitem.MatCBIndex = rd.GetMaterial("water")->MatCBIndex;
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
	gridRitem.GeoIndex = md->GeometryIndex;
	gridRitem.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem.IndexCount = md->Submeshes["grid"].IndexCount;
	gridRitem.StartIndexLocation = md->Submeshes["grid"].StartIndexLocation;
	gridRitem.baseVertexLocation = md->Submeshes["grid"].BaseVertexLocation;

	rd.AddRenderItem("gridItem", gridRitem, RenderLayer::Opaque);
}

static void buildLandGeometry(DX12Render& rd, DX12Context* dxC)
{
	GeometryGenerator geoGen;
	MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	std::vector<Vertex1> vertices;
	for (u32 i = 0; i < grid.Vertices.size(); i++)
	{
		Vertex1 v;
		v.Pos = grid.Vertices[i].Position;
		v.Pos.y = GetHillsHeight(v.Pos.x, v.Pos.z);
		v.Normal = GetHillsNormal(v.Pos.x, v.Pos.z);
		vertices.push_back(v);
	}

	const u32 vbByteSize = vertices.size() * sizeof(Vertex1);
	const u32 ibByteSize = grid.Indicies.size() * sizeof(u16);

	MeshGeometry geo;

	HR(D3DCreateBlob(vbByteSize, &geo.VertexBufferCPU));
	CopyMemory(geo.VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	HR(D3DCreateBlob(ibByteSize, &geo.IndexBufferCPU));
	CopyMemory(geo.IndexBufferCPU->GetBufferPointer(), grid.Indicies.data(), ibByteSize);

	geo.VertexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
		vertices.data(), vbByteSize, geo.VertexBufferUploader);

	geo.IndexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
		grid.Indicies.data(), ibByteSize, geo.IndexBufferUploader);

	geo.VertexByteStride = sizeof(Vertex1);
	geo.VertexBufferByteSize = vbByteSize;
	geo.IndexFormat = DXGI_FORMAT_R16_UINT;
	geo.IndexBufferByteSize = ibByteSize;

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
	grass.DiffuseAlbedo = { 0.2f, 0.6f, 0.2f, 1.0f };
	grass.FresnelR0 = { 0.01f, 0.01f, 0.01f };
	grass.Roughness = 0.125f;

	Material water;
	// 	water.Name = "water";
	water.MatCBIndex = 1;
	water.DiffuseAlbedo = { 0.0f, 0.2f, 0.6f, 1.0f };
	water.FresnelR0 = { 0.1f, 0.1f, 0.1f };
	water.Roughness = 0.0f;

	rd.AddMaterial("grass", grass);
	rd.AddMaterial("water", water);
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

	u32 vbByteSize = gWaves->VertexCount() * sizeof(Vertex1);
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

	geo.VertexByteStride = sizeof(Vertex1);
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
	DX12Context* dxC = new DX12Context(1280, 720, "Winnidow");

	DX12Render* render = new DX12Render(dxC);

	buildLandGeometry(*render, dxC);
	buildMaterials(*render);
	buildWavesGeometryBuffers(*render, dxC);
	buildRenderItems(*render);
	render->FinishSetup();

	while (render->isWindowActive())
	{
		render->Update();
		render->Draw();
	}

	return 0;
}