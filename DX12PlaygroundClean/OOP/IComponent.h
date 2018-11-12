#pragma once
#include "../Core/Default.h"

class GameObject;
class Transform;

class IComponent
{
public:
	void Register(GameObject* parrent, Transform* transform);
	virtual void Init() = 0;
	virtual void Update(float time, float deltaTime) = 0;

	IComponent();

	GameObject* Parent;
	Transform* transFormComp;
};
