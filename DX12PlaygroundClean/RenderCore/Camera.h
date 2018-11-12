#pragma once
#include "../Core/Default.h"

struct Camera
{
	XMFLOAT3 EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 View = Identity4x4();
};

void static UpdateCameras(Camera* cams, u32 cameraCount)
{
	for (int i = 0; i < cameraCount; i++)
	{
		XMFLOAT3 eyePos = cams[i].EyePos;

		XMVECTOR pos = XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&cams[i].View, view);
	}
}