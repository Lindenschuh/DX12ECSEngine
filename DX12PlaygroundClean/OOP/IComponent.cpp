#include "IComponent.h"

void IComponent::Register(GameObject * parrent, Transform * transform)
{
	transFormComp = transform;
	Parent = parrent;
}

IComponent::IComponent()
{
}