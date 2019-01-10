#pragma once
#include "../Core/Default.h"
#include "EntitySystem.h"
class PositionSystem
{
private:
	EntityManger* mEManager;

public:
	PositionSystem(EntityManger * eManager);

	void Strafe(EntityID id, float d);
	void Walk(EntityID id, float d);

	void Pitch(EntityID id, float angle);
	void RotateY(EntityID id, float angle);
};