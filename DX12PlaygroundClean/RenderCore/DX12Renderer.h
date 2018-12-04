#pragma once
#include "../Core/Default.h"
#include "../Util/Timer.h"

#include "TextureSystem.h"
#include "MaterialSystem.h"
#include "GeometrySystem.h"
#include "FrameResourceSystem.h"
#include "ShaderSystem.h"
#include "PSOSystem.h"

#include "DXHelpers.h"
#include "DXData.h"
#include "Camera.h"
#include "DX12Context.h"

class DX12Renderer
{
public:
	DX12Renderer(u32 width, u32 height, const char* windowName);
	GameTimer* mTimer;
	bool IsWindowActive(void);
	void Draw();
	void Update();
	void FinishSetup();
	void AddRenderItem(std::string name, RenderItem r, RenderLayer rl);
	void SetMainCamera(XMFLOAT3 position, XMFLOAT4X4 view);
	void SetFogData(XMFLOAT4 fogColor, float fogStart, float fogRange);

	RenderItem* GetRenderItem(std::string name);

	std::vector<RenderItem> mRItems[RenderLayer::Count];

	TextureSystem* mTextureSystem;
	MaterialSystem* mMaterialSystem;
	GeometrySystem* mGeometrySystem;
	ShaderSystem* mShaderSystem;
	PSOSystem* mPSOSystem;
	FrameResourceSystem* mFrameResourceSystem;

private:
	std::unordered_map<std::string, std::pair<RenderLayer, u32>> mRenderItemPair;
	ComPtr<ID3D12RootSignature> mRootSignature;
	DX12Context* mDXCon;

	FogData mFogData;
	Camera mMainCam;
	XMFLOAT4X4 mProj = Identity4x4();
	PassConstants mMainPassCB;

	//Methods
private:
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentbackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	ID3D12Resource * CurrentBackbuffer();

	void ProcessGlobalEvents();
	void CalculateFrameStats();
	void BuildRootSignature();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList,
		std::vector<RenderItem>&rItems);
};
