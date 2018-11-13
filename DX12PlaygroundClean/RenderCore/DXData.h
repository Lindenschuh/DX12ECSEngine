#pragma once
#include "../Core/Default.h"
#include "Camera.h"
#include "DX12Context.h"
#include "../Util/Timer.h"
#include "../Util/Waves.h"
#include "DXHelpers.h"
enum RenderLayer
{
	Opaque = 0,
	Count
};

struct RenderItem
{
	XMFLOAT4X4 WorldPos = Identity4x4();
	XMFLOAT4X4 TextureTransform = Identity4x4();

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	u32 ObjCBIndex = -1;
	u32 MatCBIndex = -1;
	u32 texHeapIndex = -1;
	u32 GeoIndex = -1;

	//IndexParameters
	u32 IndexCount = 0;
	u32 StartIndexLocation = 0;
	s32 baseVertexLocation = 0;
};

typedef struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
} Vertex1;

typedef struct Vertex2
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex0;
	XMFLOAT2 Tex1;
} Vertex2;

typedef struct Submesh
{
	u32 IndexCount;
	u32 StartIndexLocation;
	s32 BaseVertexLocation;

	DirectX::BoundingBox Bounds;
} Submesh;

typedef struct ObjectConstants
{
	XMFLOAT4X4 World = Identity4x4();
	XMFLOAT4X4 TextureTransform = Identity4x4();
} ObjectConstants;

typedef struct MeshGeometry
{
	u32 GeometryIndex;
	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	u32 VertexByteStride = 0;
	u32 VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	u32 IndexBufferByteSize = 0;

	std::unordered_map<std::string, Submesh> Submeshes;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
} MeshGeometry;

struct Light
{
	XMFLOAT3 Strength = { 0.5f,0.5f,0.5f };
	float FalloffStart = 1.0f;
	XMFLOAT3 Direction = { 0.0f,-1.0f,0.0f };
	float FalloffEnd = 10.0f;
	XMFLOAT3 Position = { 0.0f,0.0f,0.0f };
	float SpotPower = 64.0f;
};

#define MaxLights 16

struct MaterialConstants
{
	XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = 0.25f;

	XMFLOAT4X4 MatTransform = Identity4x4();
};

struct Material
{
	MaterialID MatCBIndex = -1;

	s32 DiffuseSrvHeapIndex = -1;

	s32 NormalSrvHeapIndex = -1;

	s32 NumFramesDirty = gNumFrameResources;

	XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = 0.25f;

	XMFLOAT4X4 MatTransform = Identity4x4();

	void setDiffuseAlbedo(XMFLOAT4 color)
	{
		DiffuseAlbedo = color;
		NumFramesDirty = gNumFrameResources;
	}

	void setFresnelR0(XMFLOAT3 fres)
	{
		FresnelR0 = fres;
		NumFramesDirty = gNumFrameResources;
	}

	void setRoughness(float rough)
	{
		Roughness = rough;
		NumFramesDirty = gNumFrameResources;
	}

	void setTransformMat(XMFLOAT4X4 mat)
	{
		MatTransform = mat;
		NumFramesDirty = gNumFrameResources;
	}
};

struct Texture
{
	std::wstring Filename;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

struct PassConstants
{
	XMFLOAT4X4 View = Identity4x4();
	XMFLOAT4X4 InvView = Identity4x4();
	XMFLOAT4X4 Proj = Identity4x4();
	XMFLOAT4X4 InvProj = Identity4x4();
	XMFLOAT4X4 ViewProj = Identity4x4();
	XMFLOAT4X4 InvViewProj = Identity4x4();
	XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	XMFLOAT2 RenderTargetSize = { 0.0f ,0.0f };
	XMFLOAT2 InvRenderTargetSize = { 0.0f,0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	XMFLOAT4 AmbientLight = { 0.0f,0.0f,0.0f,1.0f };

	Light Lights[MaxLights];
};

void static UpdateMaterialCBs(std::vector<Material>& allMaterials,
	UploadBuffer<MaterialConstants>* currMat)
{
	for (int i = 0; i < allMaterials.size(); i++)
	{
		Material* mat = &allMaterials[i];

		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConst;
			matConst.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConst.FresnelR0 = mat->FresnelR0;
			matConst.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConst.MatTransform, XMMatrixTranspose(matTransform));
			currMat->CopyData(mat->MatCBIndex, matConst);

			mat->NumFramesDirty--;
		}
	}
}

void static UpdateObjectPassCB(std::vector<RenderItem>& rItems,
	UploadBuffer<ObjectConstants>* currentObjectCB)
{
	for (int i = 0; i < rItems.size(); i++)
	{
		XMMATRIX world = XMLoadFloat4x4(&rItems[i].WorldPos);
		XMMATRIX texTransform = XMLoadFloat4x4(&rItems[i].TextureTransform);

		ObjectConstants objConst;
		XMStoreFloat4x4(&objConst.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objConst.TextureTransform, XMMatrixTranspose(texTransform));
		currentObjectCB->CopyData(rItems[i].ObjCBIndex, objConst);
	}
}

void static UpdateMainPassCB(PassConstants& mainPassCB,
	UploadBuffer<PassConstants>* passCB, Camera* mainCamera,
	DX12Context* mDXCon, GameTimer* mTimer, XMFLOAT4X4 mProj)
{
	XMMATRIX view = XMLoadFloat4x4(&mainCamera->View);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mainPassCB.EyePosW = mainCamera->EyePos;
	mainPassCB.RenderTargetSize = XMFLOAT2(mDXCon->Window->width, mDXCon->Window->height);
	mainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mDXCon->Window->width, 1.0f / mDXCon->Window->height);
	mainPassCB.NearZ = 1.0f;
	mainPassCB.FarZ = 1000.0f;
	mainPassCB.DeltaTime = mTimer->mDeltaTime;
	mainPassCB.TotalTime = mTimer->GetGameTime();
	mainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.9f };
	mainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mainPassCB.Lights[1].Strength = { 0.5f, 0.5f, 0.5f };
	mainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	passCB->CopyData(0, mainPassCB);
}
