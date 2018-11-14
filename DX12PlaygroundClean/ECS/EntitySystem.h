#pragma once

#include "..\Core\Default.h"
#include "..\RenderCore\DXData.h"

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

struct VelocityComponent
{
	XMFLOAT3 Velocity;

	void Init(float minSpeed, float maxSpeed)
	{
		// random angle
		float angle = RandomFloat01() * 3.1415926f * 2;
		// random movement speed between given min & max
		float speed = RandomFloat(minSpeed, maxSpeed);
		// velocity x & y components
		Velocity.x = cosf(angle) * speed;
		Velocity.z = sinf(angle) * speed;
		Velocity.y = tanf(angle) * speed;
	}
};
struct RenderComponent
{
	XMFLOAT4X4 textureTransform = Identity4x4();
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	RenderLayer layer = RenderLayer::Opaque;
	u32 renderItemId = -1;
	MaterialID MatCBIndex = -1;
	TextureID texHeapIndex = -1;
	GeometryID GeoIndex = -1;
	bool IsDirty;
};

struct EntityManger
{
	enum
	{
		FlagPosition = 1 << 0,
		FlagRenderData = 1 << 1,
		FlagCamera = 1 << 2,
		FlagVeloctiy = 1 << 3,
	};
	std::vector<std::string> mNames;
	//Data
	std::vector<PositionComponent> mPositions;
	std::vector<RenderComponent> mRenderData;
	std::vector<CameraComponent> mCameras;
	std::vector<VelocityComponent> mVelocitys;

	//Flags for has it
	std::vector<u32> mFlags;

	void reserve(u32 n)
	{
		mNames.reserve(n);
		mPositions.reserve(n);
		mRenderData.reserve(n);
		mCameras.reserve(n);
		mVelocitys.reserve(n);
		mFlags.reserve(n);
	}

	EntityID addEntity(const std::string&& name)
	{
		EntityID id = mNames.size();
		mNames.emplace_back(name);
		mPositions.push_back(PositionComponent());
		mRenderData.push_back(RenderComponent());
		mCameras.push_back(CameraComponent());
		mVelocitys.push_back(VelocityComponent());
		mFlags.push_back(0);
		return id;
	}
};

static EntityManger gObjects;
