#pragma once
#include "Default.h"

typedef struct Vertex1
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
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
	s32 MatCBIndex = -1;

	s32 DiffuseSrvHeapIndex = -1;

	s32 NormalSrvHeapIndex = -1;

	s32 NumFramesDirty = gNumFrameResources;

	XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = 0.25f;

	XMFLOAT4X4 MatTransform = Identity4x4();
};

struct Texture
{
	std::wstring Filename;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};