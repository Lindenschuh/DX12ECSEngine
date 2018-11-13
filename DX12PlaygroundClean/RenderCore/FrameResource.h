#pragma once
#include "../Core/Default.h"
#include "DXHelpers.h"
#include "DXData.h"

class FrameResource
{
public:
	FrameResource(ID3D12Device* device, u32 passCount,
		u32 objectCount, u32 materialCount, u32 waveVertCount);

	ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	UploadBuffer<PassConstants>* PassCB = nullptr;
	UploadBuffer<MaterialConstants>* MaterialCB = nullptr;
	UploadBuffer<ObjectConstants>* ObjecCB = nullptr;

	UploadBuffer<Vertex>* WavesVB = nullptr;

	u64 Fence = 0;

	~FrameResource();
};
