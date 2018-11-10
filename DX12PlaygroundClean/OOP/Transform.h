#pragma once
#include "IComponent.h"

class Transform : public IComponent
{
public:
	XMFLOAT3 Position;
	void Init() override;
	void Update(float time, float deltaTime) override;
	Transform();
};
