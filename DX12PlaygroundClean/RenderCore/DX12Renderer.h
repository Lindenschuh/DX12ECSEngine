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
#include "DX12Context.h"
#include "..\ECS\CameraSystem.h"

class DX12Renderer
{
public:
	DX12Renderer(u32 width, u32 height, const char* windowName, EntityManger* eManager);
	GameTimer* mTimer;
	bool IsWindowActive(void);
	void Draw();
	void Update(float time, float deltaTime);
	void FinishSetup();
	void AddRenderItem(std::string name, RenderItem r, RenderLayer rl);
	void SetFogData(XMFLOAT4 fogColor, float fogStart, float fogRange);

	void SetLayerPSO(std::string psoName, RenderLayer layer);
	std::string GetLayerPSO(RenderLayer layer);

	RenderItem* GetRenderItem(std::string name);

	std::vector<RenderItem> mRItems[RenderLayer::Count];

	TextureSystem* mTextureSystem;
	MaterialSystem* mMaterialSystem;
	GeometrySystem* mGeometrySystem;
	ShaderSystem* mShaderSystem;
	PSOSystem* mPSOSystem;
	FrameResourceSystem* mFrameResourceSystem;
	CameraSystem* mCameraSystem;
private:
	std::unordered_map<std::string, std::pair<RenderLayer, u32>> mRenderItemPair;
	ComPtr<ID3D12RootSignature> mRootSignature;
	DX12Context* mDXCon;
	std::string LayerPSO[RenderLayer::Count];
	FogData mFogData;
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
		std::vector<RenderItem>&rItems, u32& INOUToffset);
};
