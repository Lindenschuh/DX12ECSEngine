#pragma once
#include "..\Core\Default.h"
#include "..\RenderCore\DXData.h"
#include <PxPhysicsAPI.h>

typedef u32 EntityID;

struct PositionComponent
{
	XMFLOAT3 Position;
	XMFLOAT3 Right = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 Up = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 Forward = { 0.0f, 0.0f, 1.0f };
};
struct CameraComponent
{
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float Aspect = 0.0f;
	float FovY = 0.0f;
	float NearWindowHeight;
	float FarWindowHeight;

	XMFLOAT4X4 ViewMat = Identity4x4();
	XMFLOAT4X4 ProjMat = Identity4x4();

	BoundingFrustum FrustrumBounds;
};

struct DynamicPhysicsComponent
{
	physx::PxRigidDynamic* DynamicRigidBody = nullptr;
};

struct StaticPhysicsComponent
{
	physx::PxRigidStatic* StaticRigidBody = nullptr;
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
	RenderLayer::RenderLayer layer = RenderLayer::Opaque;
	XMFLOAT4X4 textureTransform = Identity4x4();
	u32 renderItemID = -1;
	GeometryID GeoIndex = -1;
	MaterialID MatCBIndex = -1;
};

struct FogComponent
{
	XMFLOAT4 FogColor = { 0.0f,0.0f,0.0f,1.0f };
	float FogStart = 0.0f;
	float FogRange = 0.0f;
};