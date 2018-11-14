#pragma once
#include "../Core/Default.h"
#include "DXHelpers.h"
#include "DXData.h"

class FrameResource
{
public:
	FrameResource(ID3D12Device* device, u32 passCount,
		u32 maxInstances, u32 materialCount);

	ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	UploadBuffer<PassConstants>* PassCB = nullptr;

	UploadBuffer<MaterialData>* MaterialBuffer = nullptr;
	UploadBuffer<InstanceData>* InstanceBuffer = nullptr;

	u64 Fence = 0;

	~FrameResource();
};
