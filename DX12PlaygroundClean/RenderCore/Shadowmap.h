#pragma once
#include "../Core/Default.h"
#include "DXData.h"

struct ShadowPassData
{
	XMFLOAT4X4 LightView;
	XMFLOAT4X4 LightProj;
	XMFLOAT4X4 ShadowTransform;

	XMFLOAT3 LightPosWorld;
	float LightNearZ;
	float LightFarZ;
};

class ShadowMap
{
public:
	u32 mWidth;
	u32 mHeight;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuSrvShadowMapHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuSrvShadowMapHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuDsvShadowMapHandle;
	ComPtr<ID3D12Resource> mShadowMapResource;
	D3D12_VIEWPORT mViewPort;
	D3D12_RECT mScissorRec;

	ShadowMap(u32 width, u32 height, ID3D12Device* device,
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvShadowMapHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvShadowMapHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsvShadowMapHandle);

	void Resize(u32 width, u32 height, ID3D12Device* device);
private:
	void BuildDiscriptors(ID3D12Device* device);
	void BuildResource(ID3D12Device* device);
};

static void UpdateShadowTranformation(XMFLOAT3 lightDirection, BoundingSphere sceneBounds, ShadowPassData& passData)
{
	XMVECTOR lightDir = XMLoadFloat3(&lightDirection);
	XMVECTOR lightPos = -2.0f* sceneBounds.Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&sceneBounds.Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&passData.LightPosWorld, lightPos);

	XMFLOAT3 SphereCenterLS;
	XMStoreFloat3(&SphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	float frustLeft = SphereCenterLS.x - sceneBounds.Radius;
	float frustBottum = SphereCenterLS.y - sceneBounds.Radius;
	float frustNear = SphereCenterLS.z - sceneBounds.Radius;
	float frustRight = SphereCenterLS.x + sceneBounds.Radius;
	float frustTop = SphereCenterLS.y + sceneBounds.Radius;
	float frustFar = SphereCenterLS.z + sceneBounds.Radius;

	passData.LightFarZ = frustFar;
	passData.LightNearZ = frustNear;

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(frustLeft, frustRight,
		frustBottum, frustTop, frustNear, frustFar);

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX shadowT = lightView * lightProj* T;
	XMStoreFloat4x4(&passData.LightView, lightView);
	XMStoreFloat4x4(&passData.LightProj, lightProj);
	XMStoreFloat4x4(&passData.ShadowTransform, shadowT);
}

static void UpdateShadowPass(PassConstants& shadowPass, UploadBuffer<PassConstants>* passCB, ShadowPassData& shadowData, u32 width, u32 height)
{
	XMMATRIX view = XMLoadFloat4x4(&shadowData.LightView);
	XMMATRIX proj = XMLoadFloat4x4(&shadowData.LightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&shadowPass.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&shadowPass.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&shadowPass.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&shadowPass.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&shadowPass.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&shadowPass.InvViewProj, XMMatrixTranspose(invViewProj));
	shadowPass.EyePosW = shadowData.LightPosWorld;
	shadowPass.RenderTargetSize = XMFLOAT2((float)width, (float)height);
	shadowPass.InvRenderTargetSize = XMFLOAT2(1.0f / width, 1.0f / height);
	shadowPass.NearZ = shadowData.LightNearZ;
	shadowPass.FarZ = shadowData.LightFarZ;

	passCB->CopyData(PassConstantIndex::ShadowPass, shadowPass);
}