#include "PhysicSystem.h"

PhysicsSystem::PhysicsSystem()
{
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
}

PhysicsSystem::~PhysicsSystem()
{
	mFoundation->release();
}