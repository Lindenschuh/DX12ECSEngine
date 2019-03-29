#pragma once
#include "../Core/Default.h"
#include "EntitySystem.h"
#include "PositionSystem.h"

class ControllSystem
{
private:
	std::vector<EntityID> mEntities;
	PositionSystem* mPositionSystem;
	EntityManager* mEManager;
	int mLastMouseX;
	int mLastMouseY;

	float speed = 10.0f;
public:
	ControllSystem(EntityManager* eManager, PositionSystem* pSystem);
	void AddToSystem(EntityID id);
	void RemoveFromSystem(EntityID id);
	void UpdateSystem(float time, float deltaTime);
};