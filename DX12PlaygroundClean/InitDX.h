#pragma once
#include "Default.h"
#include "Timer.h"
#include "DXHelpers.h"
#include "DXData.h"
#include "FrameResource.h"
#include "GeometryGenerator.h"
#include "Waves.h"
#include "DX12Context.h"
#include "Camera.h"
#include "ECS/EntitySystem.h"
#include "Ext/DDSTextureLoader.h"

class DX12Render
{
public:
	GameTimer* mTimer;
	DX12Render(DX12Context* context, Waves* wave);
	bool isWindowActive(void);
	void Draw();
	void Update();

	void AddMaterial(std::string name, Material m);
	void AddGeometry(std::string name, MeshGeometry g);
	void AddTexture(std::string name, Texture t);
	void AddRenderItem(std::string name, RenderItem r, RenderLayer rl);

	void SetMainCamera(XMFLOAT3 position, XMFLOAT4X4 view);

	MeshGeometry* GetGeometry(std::string name);
	Material*	GetMaterial(std::string name);
	Texture*	GetTexture(std::string name);
	RenderItem* GetRenderItem(std::string name);
	FrameResource* mCurrentFrameResource;
	Waves* gWaves;

	std::vector<RenderItem> mRItems[RenderLayer::Count];
	std::vector<MeshGeometry> mAllGeometry;
	std::vector<Material> mAllMaterials;
	std::vector<Texture> mAllTextures;

	void FinishSetup();
private:
	DX12Context* mDXCon;
	Camera mMainCam;
	std::vector<FrameResource> mFrameResources;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map<std::string, u32> mGeometeriesIndex;
	std::unordered_map<std::string, u32> mMaterialsIndex;
	std::unordered_map<std::string, u32> mTextureIndex;
	std::unordered_map<std::string, std::pair<RenderLayer, u32>> mRenderItemPair;

	ComPtr<ID3D12RootSignature> mRootSignature;

	D3D12_INPUT_ELEMENT_DESC gInputLayout[3];

	u32 mPassCbvOffset = 0;

	s32 mCurrentFrameResourceIndex = 0;

	PassConstants mMainPassCB;

	UploadBuffer<ObjectConstants>* mObjectConstBuffer = nullptr;

	XMFLOAT4X4 mProj = Identity4x4();

	//Methods
private:
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentbackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	ID3D12Resource * CurrentBackbuffer();

	void processGlobalEvents();
	void calculateFrameStats();
	void buildRootSignature();
	void buildDescriptorHeaps();
	void buildShaders();
	void buildPSO();
	void buildFrameResources();
	void drawRenderItems(ID3D12GraphicsCommandList* cmdList,
		std::vector<RenderItem>&rItems);
};

struct GeoBuildInfo
{
	std::string Name;
	std::string* SubmeshNames;

	Submesh* submeshs;
	Vertex* verts;
	u16* indicies;

	u32 vertCount;
	u32 indiceCount;
	u32 submeshCount;
};

struct DynamicGeoBuildInfo
{
	std::string Name;
	std::string* SubmeshNames;

	Submesh* submeshs;
	Vertex* verts;
	u16* indicies;

	u32 vertCount;
	u32 indiceCount;
	u32 indexByteSize;
	u32 vertexByteSize;
	u32 submeshCount;
};

static void CreateGeometry(GeoBuildInfo* geoInfo,
	DX12Context* dxC, DX12Render* rd)
{
	MeshGeometry geo;

	const u32 vbByteSize = geoInfo->vertCount * sizeof(Vertex);
	const u32 ibByteSize = geoInfo->indiceCount * sizeof(u16);

	HR(D3DCreateBlob(vbByteSize, &geo.VertexBufferCPU));
	CopyMemory(geo.VertexBufferCPU->GetBufferPointer(), geoInfo->verts, vbByteSize);

	HR(D3DCreateBlob(ibByteSize, &geo.IndexBufferCPU));
	CopyMemory(geo.IndexBufferCPU->GetBufferPointer(), geoInfo->indicies, ibByteSize);

	geo.VertexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
		geoInfo->verts, vbByteSize, geo.VertexBufferUploader);

	geo.IndexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
		geoInfo->indicies, ibByteSize, geo.IndexBufferUploader);

	geo.VertexByteStride = sizeof(Vertex);
	geo.VertexBufferByteSize = vbByteSize;
	geo.IndexFormat = DXGI_FORMAT_R16_UINT;
	geo.IndexBufferByteSize = ibByteSize;

	for (int i = 0; i < geoInfo->submeshCount; i++)
	{
		geo.Submeshes[geoInfo->SubmeshNames[i]] = geoInfo->submeshs[i];
	}
	rd->AddGeometry(geoInfo->Name, geo);
}
static void CreateDynamicGeometry(DynamicGeoBuildInfo* geoInfo,
	DX12Context* dxC, DX12Render* rd)
{
	MeshGeometry geo;
	if (geoInfo->vertCount > 0)
	{
		HR(D3DCreateBlob(geoInfo->vertexByteSize, &geo.VertexBufferCPU));
		CopyMemory(geo.VertexBufferCPU->GetBufferPointer(), geoInfo->verts, geoInfo->vertexByteSize);

		geo.VertexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
			geoInfo->verts, geoInfo->vertexByteSize, geo.VertexBufferUploader);
	}

	if (geoInfo->indiceCount > 0)
	{
		HR(D3DCreateBlob(geoInfo->indexByteSize, &geo.IndexBufferCPU));
		CopyMemory(geo.IndexBufferCPU->GetBufferPointer(), geoInfo->indicies, geoInfo->indexByteSize);

		geo.IndexBufferGPU = CreateDefaultBuffer(dxC->mD3dDevice.Get(), dxC->mCmdList.Get(),
			geoInfo->indicies, geoInfo->indexByteSize, geo.IndexBufferUploader);
	}
	geo.VertexByteStride = sizeof(Vertex);
	geo.VertexBufferByteSize = geoInfo->vertexByteSize;
	geo.IndexFormat = DXGI_FORMAT_R16_UINT;
	geo.IndexBufferByteSize = geoInfo->indexByteSize;

	for (int i = 0; i < geoInfo->submeshCount; i++)
	{
		geo.Submeshes[geoInfo->SubmeshNames[i]] = geoInfo->submeshs[i];
	}
	rd->AddGeometry(geoInfo->Name, geo);
}

void static LoadTextureFromFile(std::string name, std::wstring path,
	DX12Context* dx, DX12Render* rd)
{
	Texture tex;
	tex.Filename = path;
	HR(CreateDDSTextureFromFile12(dx->mD3dDevice.Get(),
		dx->mCmdList.Get(), tex.Filename.c_str(),
		tex.Resource, tex.UploadHeap));

	rd->AddTexture(name, tex);
}