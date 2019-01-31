#pragma once
#include"EntitySystem.h"
#include "../RenderCore/DX12Renderer.h"
class LightSystem
{
private:
	DX12Renderer* mRenderer;
	XMFLOAT3 mDirection = { 0.57735f, -0.57735f, 0.57735f };;
	XMFLOAT3 mStrenght = { 0.9f, 0.9f, 0.9f };

public:
	LightSystem(DX12Renderer* render);
	void UpdateSystem(float time, float deltaTime);
};
