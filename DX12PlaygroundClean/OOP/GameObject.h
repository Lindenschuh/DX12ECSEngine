#pragma once

#include "Transform.h"

class GameObject
{
public:

	GameObject();
	void Init();
	void Update(float time, float deltaTime);
	void RegisterCompoment(IComponent* component);
	void RegisterCompoments(IComponent** comps, u32 count);
	Transform* transFormComp;
private:
	std::vector<IComponent*> components;
};
