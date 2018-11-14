#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device * device, u32 passCount, u32 maxInstances, u32 materialCount)
{
	HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	PassCB = new UploadBuffer<PassConstants>(device, passCount, true);
	MaterialBuffer = new UploadBuffer<MaterialData>(device, materialCount, false);
	InstanceBuffer = new UploadBuffer<InstanceData>(device, maxInstances, false);
}

FrameResource::~FrameResource()
{
}