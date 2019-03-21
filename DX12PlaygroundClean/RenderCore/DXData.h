#pragma once
#include "../Core/Default.h"
#include "DX12Context.h"
#include "../Util/Timer.h"
#include "../Util/Waves.h"
#include "DXHelpers.h"
namespace PassConstantIndex
{
	enum  PassConstantIndex
	{
		MainPass = 0,
		ShadowPass,
		Count
	};
}
namespace RenderLayer
{
	enum RenderLayer
	{
		Opaque = 0,
		Skybox,
		Mirrors,
		Reflected,
		AlphaTest,
		Transparent,
		Shadow,
		ShadowDebug,
		Count
	};
}
struct InstanceData
{
	XMFLOAT4X4 World = Identity4x4();
	XMFLOAT4X4 TexTransform = Identity4x4();
	UINT MaterialIndex;
	UINT InstancePad0;
	UINT InstancePad1;
	UINT InstancePad2;
};
struct FogData
{
	XMFLOAT4 FogColor = { 0.0f,0.0f,0.0f,1.0f };
	float FogStart = 0.0f;
	float FogRange = 0.0f;
	XMFLOAT2 padding = {};
};

struct MaterialData
{
	XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = 0.25f;

	XMFLOAT4X4 MatTransform = Identity4x4();

	UINT DiffuseMapIndex = 0;
	UINT NormalMapIndex = 0;
	UINT MaterialPad0;
	UINT MaterialPad1;
};

struct RenderItem
{
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	u32 ObjCBIndex = -1;
	u32 GeoIndex = -1;

	std::vector<InstanceData> Instances;

	BoundingBox Bounds;
	u32 InstancesVisible = 0;
	u32 InstanceUpdated = 0;

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
	XMFLOAT3 Tangent;
} Vertex;

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
	XMFLOAT3 Strength = { 0.9f, 0.9f, 0.9f };
	float FalloffStart = 1.0f;
	XMFLOAT3 Direction = { 0.57735f, -0.57735f, 0.57735f };
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

	TextureID DiffuseSrvHeapIndex = -1;

	TextureID NormalSrvHeapIndex = -1;

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
	XMFLOAT4X4 ShadowTransform = Identity4x4();
	XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	XMFLOAT2 RenderTargetSize = { 0.0f ,0.0f };
	XMFLOAT2 InvRenderTargetSize = { 0.0f,0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	XMFLOAT4 AmbientLight = { 0.0f,0.0f,0.0f,1.0f };

	FogData FogData = {};

	Light Lights[MaxLights];
};

void static UpdateObjectPassCB(std::vector<RenderItem>* allRItems,
	UploadBuffer<InstanceData>* currentInstanceBuffer, XMFLOAT4X4 viewMat, BoundingFrustum CamFrustum)
{
	XMMATRIX view = XMLoadFloat4x4(&viewMat);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

	u32 instanceCount = 0;
	for (int lay = 0; lay < RenderLayer::Count; lay++)
	{
		std::vector<RenderItem>& rItems = allRItems[lay];

		if (lay == RenderLayer::Skybox || lay == RenderLayer::ShadowDebug)
		{
			if (rItems.size() == 0)
				continue;

			RenderItem& rItem = rItems[0];
			XMMATRIX world = XMLoadFloat4x4(&rItem.Instances[0].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&rItem.Instances[0].TexTransform);

			InstanceData data;
			XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
			data.MaterialIndex = rItem.Instances[0].MaterialIndex;
			currentInstanceBuffer->CopyData(instanceCount++, data);
			rItem.InstancesVisible = 1;
			continue;
		}

		for (int i = 0; i < rItems.size(); i++)
		{
			RenderItem& rItem = rItems[i];
			u32 instancesPerRItem = 0;

			for (int j = 0; j < rItem.InstanceUpdated; j++)
			{
				XMMATRIX world = XMLoadFloat4x4(&rItem.Instances[j].World);
				XMMATRIX texTransform = XMLoadFloat4x4(&rItem.Instances[j].TexTransform);

				XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);
				XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

				BoundingFrustum localFrustum;

				CamFrustum.Transform(localFrustum, viewToLocal);

				if (localFrustum.Contains(rItem.Bounds) != DISJOINT)
				{
					InstanceData data;
					XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
					XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
					data.MaterialIndex = rItem.Instances[j].MaterialIndex;
					currentInstanceBuffer->CopyData(instanceCount++, data);
					instancesPerRItem++;
				}
			}
			rItem.InstancesVisible = instancesPerRItem;
		}
	}
}

void static UpdateMainPassCB(PassConstants& mainPassCB,
	UploadBuffer<PassConstants>* passCB, XMFLOAT4X4 viewMat, XMFLOAT3 eyePos, XMFLOAT4X4 mProj,
	DX12Context* mDXCon, GameTimer* mTimer, FogData fogData, XMFLOAT4X4 shadowTransform, Light mainLight)
{
	XMMATRIX view = XMLoadFloat4x4(&viewMat);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMMATRIX shaTransformMat = XMLoadFloat4x4(&shadowTransform);

	XMStoreFloat4x4(&mainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&mainPassCB.ShadowTransform, XMMatrixTranspose(shaTransformMat));
	mainPassCB.EyePosW = eyePos;
	mainPassCB.RenderTargetSize = XMFLOAT2(mDXCon->Window->width, mDXCon->Window->height);
	mainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mDXCon->Window->width, 1.0f / mDXCon->Window->height);
	mainPassCB.NearZ = 1.0f;
	mainPassCB.FarZ = 1000.0f;
	mainPassCB.DeltaTime = mTimer->mDeltaTime;
	mainPassCB.TotalTime = mTimer->GetGameTime();
	mainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mainPassCB.Lights[0].Direction = mainLight.Direction;
	mainPassCB.Lights[0].Strength = mainLight.Strength;

	mainPassCB.FogData = fogData;

	passCB->CopyData(PassConstantIndex::MainPass, mainPassCB);
}
