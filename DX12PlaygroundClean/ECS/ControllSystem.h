#pragma once
#include "../Core/Default.h"
#include "EntitySystem.h"
#include "PositionSystem.h"

class ControllSystem
{
private:
	std::vector<EntityID> entities;
	PositionSystem* mPositionSystem;
	EntityManger* mEManager;
	int mLastMouseX;
	int mLastMouseY;

	float speed = 10.0f;
public:
	ControllSystem(EntityManger* eManager, PositionSystem* pSystem);
	void AddToSystem(EntityID id);
	void UpdateSystem(float time, float deltaTime);
};