#include "PositionSystem.h"

PositionSystem::PositionSystem(EntityManger * eManager)
{
	mEManager = eManager;
}

void PositionSystem::Strafe(EntityID id, float d)
{
	PositionComponent& comp = mEManager->mPositions[id];

	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&comp.Right);
	XMVECTOR p = XMLoadFloat3(&comp.Position);
	XMStoreFloat3(&comp.Position, XMVectorMultiplyAdd(s, r, p));
}

void PositionSystem::Walk(EntityID id, float d)
{
	PositionComponent& comp = mEManager->mPositions[id];

	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR f = XMLoadFloat3(&comp.Forward);
	XMVECTOR p = XMLoadFloat3(&comp.Position);
	XMStoreFloat3(&comp.Position, XMVectorMultiplyAdd(s, f, p));
}

void PositionSystem::Pitch(EntityID id, float angle)
{
	PositionComponent& comp = mEManager->mPositions[id];

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&comp.Right), angle);

	XMStoreFloat3(&comp.Up, XMVector3TransformNormal(XMLoadFloat3(&comp.Up), R));
	XMStoreFloat3(&comp.Forward, XMVector3TransformNormal(XMLoadFloat3(&comp.Forward), R));
}

void PositionSystem::RotateY(EntityID id, float angle)
{
	PositionComponent& comp = mEManager->mPositions[id];

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&comp.Right, XMVector3TransformNormal(XMLoadFloat3(&comp.Right), R));
	XMStoreFloat3(&comp.Up, XMVector3TransformNormal(XMLoadFloat3(&comp.Up), R));
	XMStoreFloat3(&comp.Forward, XMVector3TransformNormal(XMLoadFloat3(&comp.Forward), R));
}