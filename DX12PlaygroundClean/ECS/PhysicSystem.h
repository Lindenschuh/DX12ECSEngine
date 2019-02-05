#pragma once

#include "EntitySystem.h"
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

class PhysicsSystem
{
public:
	PhysicsSystem();
	~PhysicsSystem();

private:
	physx::PxDefaultAllocator		mAllocator;
	physx::PxDefaultErrorCallback	mErrorCallback;
	physx::PxFoundation*			mFoundation;
};
