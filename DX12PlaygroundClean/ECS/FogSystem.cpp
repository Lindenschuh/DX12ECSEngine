#include "FogSystem.h"

FogSystem::FogSystem(EntityManger * manager, DX12Renderer * render)
{
	eManager = manager;
	renderer = render;
}

void FogSystem::AddEntity(EntityID eId)
{
	eManager->mFlags[eId] |= eManager->FlagFog;
	entities.push_back(eId);
}

//could be better
void FogSystem::RemoveEntity(EntityID eId)
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i] == eId)
		{
			entities.erase(entities.begin() + i);
			eManager->mFlags[eId] &= ~(eManager->FlagFog);
		}
	}
}

void FogSystem::UpdateSystem(float time, float deltaTime)
{
	for (int i = 0; i < entities.size(); i++)
	{
		FogComponent& fogD = eManager->mFogs[entities[i]];

		ImGui::Begin("Fog Manipulator");
		ImGui::ColorPicker4("Fog Color", &fogD.FogColor.x);
		ImGui::SliderFloat("Fog Start", &fogD.FogStart, 1.0f, 200.0f);
		ImGui::SliderFloat("Fog Range", &fogD.FogRange, 1.0f, 800.0f);
		ImGui::End();

		renderer->SetFogData(fogD.FogColor, fogD.FogStart, fogD.FogRange);
	}
}