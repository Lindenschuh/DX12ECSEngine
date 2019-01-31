#include "Shadowmap.h"

ShadowMap::ShadowMap(u32 width, u32 height, ID3D12Device * device,
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvShadowMapHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvShadowMapHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsvShadowMapHandle)
{
	mCpuSrvShadowMapHandle = cpuSrvShadowMapHandle;
	mGpuSrvShadowMapHandle = gpuSrvShadowMapHandle;
	mCpuDsvShadowMapHandle = cpuDsvShadowMapHandle;
	Resize(width, height, device);
}

void ShadowMap::Resize(u32 width, u32 height, ID3D12Device * device)
{
	mWidth = width;
	mHeight = height;
	mViewPort = { 0.0f,0.0f,(float)width, (float)height, 0.0f, 1.0f };
	mScissorRec = { 0 , 0,(int)width, (int)height };

	BuildResource(device);
	BuildDiscriptors(device);
}

void ShadowMap::BuildDiscriptors(ID3D12Device* device)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	device->CreateShaderResourceView(mShadowMapResource.Get(), &srvDesc, mCpuSrvShadowMapHandle);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(mShadowMapResource.Get(), &dsvDesc, mCpuDsvShadowMapHandle);
}

void ShadowMap::BuildResource(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	HR(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&mShadowMapResource)
	));
}