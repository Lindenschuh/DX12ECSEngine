#pragma once
#include "../Core/Default.h"
#include "EntitySystem.h"
class PositionSystem
{
private:
	EntityManager* mEManager;

public:
	PositionSystem(EntityManager * eManager);

	void Strafe(EntityID id, float d);
	void Walk(EntityID id, float d);

	void Pitch(EntityID id, float angle);
	void RotateY(EntityID id, float angle);
	void CalculateOrientation(EntityID id);
};