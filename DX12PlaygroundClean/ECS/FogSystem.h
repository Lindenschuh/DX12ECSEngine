#pragma once
#include "EntitySystem.h"
#include "../RenderCore/DX12Renderer.h"

class FogSystem
{
private:
	DX12Renderer* renderer;
	EntityManger* eManager;
	std::vector<EntityID> entities;
public:
	FogSystem(EntityManger* manager, DX12Renderer* render);
	void AddEntity(EntityID eId);
	void RemoveEntity(EntityID eId);
	void UpdateSystem(float time, float deltaTime);
};