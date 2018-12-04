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
	RenderLayer layer = RenderLayer::Opaque;
	XMFLOAT4X4 textureTransform = Identity4x4();
	u32 renderItemID = -1;
	GeometryID GeoIndex = -1;
	u32 instanceID = -1;
	MaterialID MatCBIndex = -1;
};

struct FogComponent
{
	XMFLOAT4 FogColor = { 0.0f,0.0f,0.0f,1.0f };
	float FogStart = 0.0f;
	float FogRange = 0.0f;
};