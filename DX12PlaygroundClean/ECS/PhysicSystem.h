#pragma once

#include "EntitySystem.h"

class PhysicsSystem
{
public:
	PhysicsSystem(EntityManger* eManager);
	void AddDynamicToSystem(EntityID eId);
	void AddStaticToSystem(EntityID eId);
	void RemoveFromSystem(EntityID eId);
	void UpdateSystem(float time, float deltaTime);
	~PhysicsSystem();

private:
	std::vector<EntityID> mEntities;
	EntityManger* mEManger;

	physx::PxDefaultAllocator		mAllocator;
	physx::PxDefaultErrorCallback	mErrorCallback;

	physx::PxFoundation*			mFoundation = NULL;
	physx::PxPhysics*				mPhysics = NULL;

	physx::PxDefaultCpuDispatcher*		mDispatcher = NULL;
	physx::PxScene*					mScene = NULL;

	physx::PxMaterial*				mDefMat = NULL;

	physx::PxPvd *					mPVD = NULL;
};
