#pragma once
#include "Default.h"
#include "DXHelpers.h"
#include "DXData.h"
typedef struct ObjectConstants
{
	XMFLOAT4X4 World = Identity4x4();
} ObjectConstants;

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

struct VertexSimple
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class FrameResource
{
public:
	FrameResource(ID3D12Device* device, u32 passCount, u32 objectCount, u32 materialCount, u32 waveVertCount);

	ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	UploadBuffer<PassConstants>* PassCB = nullptr;
	UploadBuffer<MaterialConstants>* MaterialCB = nullptr;
	UploadBuffer<ObjectConstants>* ObjecCB = nullptr;

	UploadBuffer<Vertex1>* WavesVB = nullptr;

	u64 Fence = 0;

	~FrameResource();
};
