#pragma once
#include "EntitySystem.h"

class CameraSystem
{
private:
	std::vector<EntityID> Entities;

	EntityID MainCamera = MAXUINT32;
	EntityManager* mEManager;

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;
	ImVec2 lastMousePosition;

	void LookAt(PositionComponent& comp, FXMVECTOR pos, FXMVECTOR targetPos, FXMVECTOR worldUp);
public:

	CameraSystem(EntityManager* eManager);
	void AddObjectToSystem(EntityID id);
	void SetFrustum(EntityID id, float fovY, float aspect, float zNear, float zFar);
	void LookAt(EntityID id, XMFLOAT3 targetPos, XMFLOAT3 worldUp);
	void LookAt(EntityID id, XMFLOAT3 newPos, XMFLOAT3 targetPos, XMFLOAT3 worldUp);
	void LookAt(EntityID id, EntityID targetId, XMFLOAT3 worldUp);
	void DrawDebugMenu();
	void SetMainCamera(EntityID id);
	EntityID GetMainCamera();
	CameraComponent& GetMainCameraComp();
	XMFLOAT3 GetMainCameraPos() const;
	void UpdateSystem(float time, float deltaTime);
};
