#include "FrameResourceSystem.h"

FrameResourceSystem::FrameResourceSystem(u32 resourceCount, DX12Context * context,
	u32 passCount, u32 objectCount, u32 materialCount)
{
	mDXContext = context;
	mResourceCount = resourceCount;
	mFrameResources.reserve(resourceCount);
	for (int i = 0; i < resourceCount; i++)
	{
		mFrameResources[i] = FrameResource(mDXContext->mD3dDevice.Get(),
			passCount, objectCount, materialCount, 0);
	}
}

FrameResourceID FrameResourceSystem::SwitchFrameResource()
{
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mResourceCount;
	FrameResource& currentFrameResource = mFrameResources[mCurrentFrameIndex];

	if (currentFrameResource.Fence != 0 && mDXContext->mFence->GetCompletedValue() < currentFrameResource.Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		HR(mDXContext->mFence->SetEventOnCompletion(currentFrameResource.Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	return mCurrentFrameIndex;
}

FrameResourceID FrameResourceSystem::GetCurrentFrameResourceID()
{
	return mCurrentFrameIndex;
}

FrameResource & FrameResourceSystem::GetFrameResource(FrameResourceID id)
{
	return mFrameResources[id];
}

FrameResource & FrameResourceSystem::GetCurrentFrameResource()
{
	return  mFrameResources[mCurrentFrameIndex];
}