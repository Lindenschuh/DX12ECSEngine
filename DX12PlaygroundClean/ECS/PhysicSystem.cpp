#include "PhysicSystem.h"

using namespace physx;

#define PVD_HOST "127.0.0.1"

PhysicsSystem::PhysicsSystem(EntityManger* eManager)
{
	mEManger = eManager;

	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
	mPVD = PxCreatePvd(*mFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	mPVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(),
		true, mPVD);
	PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	mDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = mDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	mScene = mPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	mDefMat = mPhysics->createMaterial(0.5f, 0.5f, 0.5f);
}

void PhysicsSystem::AddDynamicToSystem(EntityID eId)
{
	mEntities.push_back(eId);
	mEManger->mFlags[eId] |= mEManger->FlagDynamicPhysic;
	DynamicPhysicsComponent& dy = mEManger->mDynamicPhysics[eId];
	PositionComponent pos = mEManger->mPositions[eId];
	RenderComponent renComp = mEManger->mRenderData[eId];

	PxTransform transForm(PxVec3(pos.Position.x, pos.Position.y, pos.Position.z));
	//TODO: Platz halter
	PxShape* shape = mPhysics->createShape(PxBoxGeometry(4.0f, 4.0f, 4.0f), *mDefMat);
	dy.DynamicRigidBody = mPhysics->createRigidDynamic(transForm);
	dy.DynamicRigidBody->attachShape(*shape);
	mScene->addActor(*dy.DynamicRigidBody);
}

void PhysicsSystem::AddStaticToSystem(EntityID eId)
{
	mEntities.push_back(eId);
	mEManger->mFlags[eId] |= mEManger->FlagStaticPhysic;
}

void PhysicsSystem::RemoveFromSystem(EntityID eId)
{
	for (int i = 0; i < mEntities.size(); i++)
	{
		if (mEntities[i] == eId)
		{
			mEntities.erase(mEntities.begin() + i);
			DynamicPhysicsComponent dy = mEManger->mDynamicPhysics[eId];
			StaticPhysicsComponent st = mEManger->mStaticPhysics[eId];
			if (dy.DynamicRigidBody)
			{
				mScene->removeActor(*dy.DynamicRigidBody);
				dy.DynamicRigidBody->release();
				dy.DynamicRigidBody = nullptr;
				mEManger->mFlags[eId] ^= mEManger->FlagDynamicPhysic;
			}
			if (st.StaticRigidBody)
			{
				mScene->removeActor(*st.StaticRigidBody);
				st.StaticRigidBody->release();
				st.StaticRigidBody = nullptr;
				mEManger->mFlags[eId] ^= mEManger->FlagStaticPhysic;
			}
		}
		else
		{
			return;
		}
	}
}

void PhysicsSystem::UpdateSystem(float time, float deltaTime)
{
	mScene->simulate(deltaTime);
	mScene->fetchResults(true);

	//update Pos for the Moment later full Px
	for (int i = 0; i < mEntities.size(); i++)
	{
		EntityID eId = mEntities[i];
		if (mEManger->mFlags[eId] & mEManger->FlagDynamicPhysic)
		{
			PxRigidDynamic* rigi = mEManger->mDynamicPhysics[eId].DynamicRigidBody;
			PositionComponent& pos = mEManger->mPositions[eId];
			PxTransform trans = rigi->getGlobalPose();
			pos.Position = XMFLOAT3(trans.p.x, trans.p.y, trans.p.z);
		}
	}
}

PhysicsSystem::~PhysicsSystem()
{
	mScene->release();
	mDispatcher->release();
	mPhysics->release();
	PxPvdTransport* transport = mPVD->getTransport();
	mPVD->release();
	transport->release();

	mFoundation->release();
}