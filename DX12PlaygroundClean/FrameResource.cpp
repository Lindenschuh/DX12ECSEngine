#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device * device, u32 passCount, u32 objectCount, u32 materialCount, u32 waveVertCount)
{
	HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	PassCB = new UploadBuffer<PassConstants>(device, passCount, true);
	MaterialCB = new UploadBuffer<MaterialConstants>(device, materialCount, true);
	ObjecCB = new UploadBuffer<ObjectConstants>(device, objectCount, true);

	WavesVB = new UploadBuffer<Vertex1>(device, waveVertCount, false);
}

FrameResource::~FrameResource()
{
}