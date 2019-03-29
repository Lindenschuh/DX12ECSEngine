#pragma once

#include "EnitityData.h"

struct EntityManager
{
	enum
	{
		FlagPosition = 1 << 0,
		FlagRenderData = 1 << 1,
		FlagCamera = 1 << 2,
		FlagVeloctiy = 1 << 3,
		FlagFog = 1 << 4,
		FlagDynamicPhysic = 1 << 5,
		FlagStaticPhysic = 1 << 6,
	};
	std::vector<std::string> mNames;
	//Data
	std::vector<PositionComponent> mPositions;
	std::vector<RenderComponent> mRenderData;
	std::vector<CameraComponent> mCameras;
	std::vector<VelocityComponent> mVelocitys;
	std::vector<FogComponent> mFogs;
	std::vector<DynamicPhysicsComponent> mDynamicPhysics;
	std::vector<StaticPhysicsComponent> mStaticPhysics;

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
		mFogs.reserve(n);
		mDynamicPhysics.reserve(n);
		mStaticPhysics.reserve(n);
	}

	EntityID addEntity(const std::string&& name)
	{
		EntityID id = mNames.size();
		mNames.emplace_back(name);
		mPositions.push_back(PositionComponent());
		mRenderData.push_back(RenderComponent());
		mCameras.push_back(CameraComponent());
		mVelocitys.push_back(VelocityComponent());
		mFlags.push_back(FlagPosition);
		mFogs.push_back(FogComponent());
		mDynamicPhysics.push_back(DynamicPhysicsComponent());
		mStaticPhysics.push_back(StaticPhysicsComponent());
		return id;
	}
};

static EntityManager gObjects;
