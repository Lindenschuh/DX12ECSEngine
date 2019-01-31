#include "LightSystem.h"

LightSystem::LightSystem(DX12Renderer * render)
{
	mRenderer = render;
}

void LightSystem::UpdateSystem(float time, float deltaTime)
{
	ImGui::Begin("Light Setting");
	ImGui::DragFloat3("Direction", &mDirection.x, 0.01f, -1.0f, 1.0f);
	ImGui::DragFloat3("Strength", &mStrenght.x, 0.1f, 0.0f, 1.0f);
	ImGui::End();

	mRenderer->MainLightData(mDirection, mStrenght);
}