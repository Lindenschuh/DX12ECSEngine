#include "CameraSystem.h"

void CameraSystem::LookAt(PositionComponent& comp, FXMVECTOR pos, FXMVECTOR targetPos, FXMVECTOR worldUp)
{
	XMVECTOR forwardVector = XMVector3Normalize(XMVectorSubtract(targetPos, pos));
	XMVECTOR dot = XMVector3Dot(FORWARD, forwardVector);

	XMVECTOR rotationAngle = XMVectorACos(dot);
	XMVECTOR rotationAxis = XMVector3Normalize(XMVector3Cross(FORWARD, forwardVector));

	float loadedRotAngle = 0;
	XMStoreFloat(&loadedRotAngle, rotationAngle);

	XMStoreFloat3(&comp.Position, pos);
	XMStoreFloat4(&comp.RoationQuat, XMQuaternionRotationAxis(rotationAxis, loadedRotAngle));

	FXMVECTOR quat = XMLoadFloat4(&comp.RoationQuat);
	XMStoreFloat3(&comp.Up, CalculateUp(quat));
	XMStoreFloat3(&comp.Forward, CalculateForward(quat));
	XMStoreFloat3(&comp.Right, CalculateRight(quat));
}

CameraSystem::CameraSystem(EntityManger * eManager)
{
	mEManager = eManager;
}

void CameraSystem::AddObjectToSystem(EntityID id)
{
	Entities.emplace_back(id);
	mEManager->mFlags[id] |= mEManager->FlagCamera;
	SetFrustum(id, 0.25f*XM_PI, 1.0f, 1.0f, 1000.0f);
	if (MainCamera == MAXUINT32)
		MainCamera = id;
}

void CameraSystem::SetFrustum(EntityID id, float fovY, float aspect, float zNear, float zFar)
{
	CameraComponent& comp = mEManager->mCameras[id];

	comp.FovY = fovY;
	comp.Aspect = aspect;
	comp.FarZ = zFar;
	comp.NearZ = zNear;

	comp.NearWindowHeight = 2.0f * zNear * tanf(0.5f*fovY);
	comp.FarWindowHeight = 2.0f * zFar *  tanf(0.5f*fovY);

	XMMATRIX p = XMMatrixPerspectiveFovLH(fovY, aspect, zNear, zFar);
	BoundingFrustum::CreateFromMatrix(comp.FrustrumBounds, p);
	XMStoreFloat4x4(&comp.ProjMat, p);
}

void CameraSystem::LookAt(EntityID id, XMFLOAT3 targetPos, XMFLOAT3 worldUp)
{
	PositionComponent& posComp = mEManager->mPositions[id];
	CameraComponent& camComp = mEManager->mCameras[id];

	XMVECTOR p = XMLoadFloat3(&posComp.Position);
	XMVECTOR t = XMLoadFloat3(&targetPos);
	XMVECTOR u = XMLoadFloat3(&worldUp);

	LookAt(posComp, p, t, u);
}

void CameraSystem::LookAt(EntityID id, XMFLOAT3 newPos, XMFLOAT3 targetPos, XMFLOAT3 worldUp)
{
}

void CameraSystem::LookAt(EntityID id, EntityID targetId, XMFLOAT3 worldUp)
{
	PositionComponent& posComp = mEManager->mPositions[id];
	PositionComponent& targetComp = mEManager->mPositions[targetId];
	CameraComponent& camComp = mEManager->mCameras[id];

	XMVECTOR p = XMLoadFloat3(&posComp.Position);
	XMVECTOR t = XMLoadFloat3(&targetComp.Position);
	XMVECTOR u = XMLoadFloat3(&worldUp);

	LookAt(posComp, p, t, u);
}

void CameraSystem::SetMainCamera(EntityID id)
{
	MainCamera = id;
}

EntityID CameraSystem::GetMainCamera()
{
	return MainCamera;
}

CameraComponent & CameraSystem::GetMainCameraComp()
{
	return mEManager->mCameras[MainCamera];
}

XMFLOAT3 CameraSystem::GetMainCameraPos() const
{
	return  mEManager->mPositions[MainCamera].Position;
}

void CameraSystem::UpdateSystem(float time, float deltaTime)
{
	for (int i = 0; i < Entities.size(); i++)
	{
		EntityID eId = Entities[i];
		CameraComponent& camComp = mEManager->mCameras[eId];

		PositionComponent& posComp = mEManager->mPositions[eId];

		XMVECTOR R = XMLoadFloat3(&posComp.Right);
		XMVECTOR U = XMLoadFloat3(&posComp.Up);
		XMVECTOR F = XMLoadFloat3(&posComp.Forward);
		XMVECTOR P = XMLoadFloat3(&posComp.Position);

		F = XMVector3Normalize(F);
		U = XMVector3Normalize(XMVector3Cross(F, R));

		R = XMVector3Cross(U, F);

		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, F));

		XMStoreFloat3(&posComp.Right, R);
		XMStoreFloat3(&posComp.Up, U);
		XMStoreFloat3(&posComp.Forward, F);

		camComp.ViewMat(0, 0) = posComp.Right.x;
		camComp.ViewMat(1, 0) = posComp.Right.y;
		camComp.ViewMat(2, 0) = posComp.Right.z;
		camComp.ViewMat(3, 0) = x;

		camComp.ViewMat(0, 1) = posComp.Up.x;
		camComp.ViewMat(1, 1) = posComp.Up.y;
		camComp.ViewMat(2, 1) = posComp.Up.z;
		camComp.ViewMat(3, 1) = y;

		camComp.ViewMat(0, 2) = posComp.Forward.x;
		camComp.ViewMat(1, 2) = posComp.Forward.y;
		camComp.ViewMat(2, 2) = posComp.Forward.z;
		camComp.ViewMat(3, 2) = z;

		camComp.ViewMat(0, 3) = 0.0f;
		camComp.ViewMat(1, 3) = 0.0f;
		camComp.ViewMat(2, 3) = 0.0f;
		camComp.ViewMat(3, 3) = 1.0f;
	}
}