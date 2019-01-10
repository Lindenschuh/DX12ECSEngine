#pragma once
#include "../Core/Default.h"
#include <string.h>
#include <unordered_map>
#include <vector>

enum ResourceType
{
	RWTexture,
	RWStructuredBuffer,
	Texture,
	ConstantBuffer,
	StructuredBuffer,
};

struct GPUResource
{
	ResourceType Type;
	ComPtr<ID3D12Resource> Resource;
};

class GPUResourceSystem
{
public:
	GPUResourceSystem();

	GPUResourceID AllocateResource();

private:
	std::vector<GPUResource> mAllResources;
	std::unordered_map < std::string, GPUResourceID > mResourceMap;
};