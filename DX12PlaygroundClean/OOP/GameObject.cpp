#include "GameObject.h"

GameObject::GameObject()
{
	components.push_back(new Transform());
	transFormComp = (Transform*)(components[0]);
}

void GameObject::Init()
{
	for (int i = 0; i < components.size(); i++)
	{
		components[i]->Init();
	}
}

void GameObject::Update(float time, float deltaTime)
{
	for (int i = 0; i < components.size(); i++)
	{
		components[i]->Update(time, deltaTime);
	}
}

void GameObject::RegisterCompoment(IComponent* component)
{
	component->Register(this, transFormComp);
	components.push_back(component);
}

void GameObject::RegisterCompoments(IComponent** comps, u32 count)
{
	for (int i = 0; i < count; i++)
	{
		RegisterCompoment(comps[i]);
	}
}