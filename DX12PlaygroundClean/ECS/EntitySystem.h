#pragma once

#include "..\Default.h"
#include "..\DXData.h"

typedef u32 EntityID;

struct PositionComponent
{
	XMFLOAT3 Position;
};
struct CameraComponent
{
	bool isMain = true;
	XMFLOAT4X4 ViewMat = Identity4x4();
};

struct RenderComponent
{
	XMFLOAT4X4 worldPos = Identity4x4();
	XMFLOAT4X4 textureTransform = Identity4x4();
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	RenderLayer layer = RenderLayer::Opaque;
	u32 renderItemId = -1;
	u32 MatCBIndex = -1;
	u32 texHeapIndex = -1;
	u32 GeoIndex = -1;
	bool IsDirty;
};

struct EntityManger
{
	enum
	{
		FlagPosition = 1 << 0,
		FlagRenderData = 1 << 1,
		FlagCamera = 1 << 2,
	};
	std::vector<std::string> mNames;
	//Data
	std::vector<PositionComponent> mPositions;
	std::vector<RenderComponent> mRenderData;
	std::vector<CameraComponent> mCameras;
	//Flags for has it
	std::vector<u32> mFlags;

	void reserve(u32 n)
	{
		mNames.reserve(n);
		mPositions.reserve(n);
		mRenderData.reserve(n);
		mCameras.reserve(n);
		mFlags.reserve(n);
	}

	EntityID addEntity(const std::string&& name)
	{
		EntityID id = mNames.size();
		mNames.emplace_back(name);
		mPositions.push_back(PositionComponent());
		mRenderData.push_back(RenderComponent());
		mCameras.push_back(CameraComponent());
		mFlags.push_back(0);
		return id;
	}
};

static EntityManger gObjects;
