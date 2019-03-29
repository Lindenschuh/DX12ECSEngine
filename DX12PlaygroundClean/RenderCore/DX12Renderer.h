#pragma once
#include "../Core/Default.h"
#include "../Util/Timer.h"
#include "../Util/GeometryGenerator.h"

#include "TextureSystem.h"
#include "MaterialSystem.h"
#include "GeometrySystem.h"
#include "FrameResourceSystem.h"
#include "ShaderSystem.h"
#include "PSOSystem.h"
#include "Shadowmap.h"

#include "DXHelpers.h"
#include "DXData.h"
#include "DX12Context.h"
#include "..\ECS\CameraSystem.h"

class DX12Renderer
{
public:
	DX12Renderer(u32 width, u32 height, const char* windowName, EntityManager* eManager);
	GameTimer* mTimer;
	bool IsWindowActive(void);
	void Draw();
	void Update(float time, float deltaTime);
	void FinishSetup();
	void AddGeometryBatch(std::string name, GeometryBatch r, RenderLayer::RenderLayer rl);
	void SetFogData(XMFLOAT4 fogColor, float fogStart, float fogRange);
	void SetLayerPSO(std::string psoName, RenderLayer::RenderLayer layer);
	void MainLightData(XMFLOAT3 direction, XMFLOAT3 strength);
	std::string GetLayerPSO(RenderLayer::RenderLayer layer);

	GeometryBatch* GetGeoBatch(std::string name);

	std::vector<GeometryBatch> mAllGeometryBatches[RenderLayer::Count];

	TextureSystem* mTextureSystem;
	MaterialSystem* mMaterialSystem;
	GeometrySystem* mGeometrySystem;
	ShaderSystem* mShaderSystem;
	PSOSystem* mPSOSystem;
	FrameResourceSystem* mFrameResourceSystem;
	CameraSystem* mCameraSystem;
	ShadowMap* mShadowMap;
private:
	std::unordered_map<std::string, std::pair<RenderLayer::RenderLayer, u32>> mRenderItemPair;
	ComPtr<ID3D12RootSignature> mRootSignature;
	DX12Context* mDXCon;
	std::string LayerPSO[RenderLayer::Count];
	FogData mFogData;
	ShadowPassData mSPData;
	BoundingSphere mSceneBounds;
	PassConstants mMainPassCB;
	PassConstants mShadowPass;
	Light mDirectinalLight;
	void BuildSkyBox();
	//Methods
private:
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentbackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	ID3D12Resource * CurrentBackbuffer();

	void ProcessGlobalEvents();
	void CalculateFrameStats();
	void BuildRootSignature();
	void DrawShadowMap();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList,
		std::vector<GeometryBatch>&geoBatches, u32& INOUToffset);
};
