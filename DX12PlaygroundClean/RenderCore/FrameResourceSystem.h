#pragma once
#include "../Core/Default.h"
#include "FrameResource.h"
#include "DX12Context.h"

class FrameResourceSystem
{
private:
	std::vector<FrameResource> mFrameResources;
	FrameResourceID mCurrentFrameIndex;
	u32 mResourceCount;
	DX12Context* mDXContext;
public:
	FrameResourceSystem(u32 resourceCount, DX12Context* context, u32 passCount,
		u32 objectCount, u32 materialCount);

	FrameResourceID SwitchFrameResource();
	FrameResourceID GetCurrentFrameResourceID();
	FrameResource& GetFrameResource(FrameResourceID id);
	FrameResource& GetCurrentFrameResource();
};