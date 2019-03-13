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

	XMVECTOR roationQuaternion = XMQuaternionRotationAxis(XMLoadFloat3(&comp.Right), angle);
	XMVECTOR resQ = XMQuaternionMultiply(
		XMLoadFloat4(&comp.RoationQuat), roationQuaternion);

	XMStoreFloat4(&comp.RoationQuat, resQ);
	CalculateOrientation(id);
}

void PositionSystem::RotateY(EntityID id, float angle)
{
	PositionComponent& comp = mEManager->mPositions[id];

	XMVECTOR roationQuaternion = XMQuaternionRotationMatrix(XMMatrixRotationY(angle));
	XMVECTOR resQ = XMQuaternionMultiply(
		XMLoadFloat4(&comp.RoationQuat), roationQuaternion);

	XMStoreFloat4(&comp.RoationQuat, resQ);
	CalculateOrientation(id);
}

void PositionSystem::CalculateOrientation(EntityID id)
{
	PositionComponent& comp = mEManager->mPositions[id];

	FXMVECTOR quat = XMLoadFloat4(&comp.RoationQuat);
	XMStoreFloat3(&comp.Up, CalculateUp(quat));
	XMStoreFloat3(&comp.Forward, CalculateForward(quat));
	XMStoreFloat3(&comp.Right, CalculateRight(quat));
}