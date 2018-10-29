#pragma once
#include "Default.h"
#include "Timer.h"
#include "DXHelpers.h"
#include "DXData.h"
#include "FrameResource.h"
#include "GeometryGenerator.h"
#include "Waves.h"
#include "RenderElements.h"
#include "DX12Context.h"

class DX12Render
{
public:
	GameTimer* mTimer;
	DX12Render(DX12Context* context);
	bool isWindowActive(void);
	void Draw();
	void Update();

	void AddMaterial(std::string name, Material m);
	void AddGeometry(std::string name, MeshGeometry g);
	void AddTexture(std::string name, Texture t);

	MeshGeometry* GetGeometry(std::string name);
	Material*	GetMaterial(std::string name);
	Texture*	GetTexture(std::string name);
	RenderItem* GetRenderItem(std::string name);

	void AddRenderItem(std::string name, RenderItem r, RenderLayer rl);
	void FinishSetup();
private:
	DX12Context* mDXCon;

	std::vector<FrameResource> mFrameResources;
	FrameResource* mCurrentFrameResource;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map<std::string, u32> mGeometeriesIndex;
	std::unordered_map<std::string, u32> mMaterialsIndex;
	std::unordered_map<std::string, u32> mTextureIndex;
	std::unordered_map<std::string, std::pair<RenderLayer, u32>> mRenderItemPair;

	std::vector<RenderItem> mRItems[RenderLayer::Count];
	std::vector<MeshGeometry> mAllGeometry;
	std::vector<Material> mAllMaterials;
	std::vector<Texture> mAllTextures;

	ComPtr<ID3D12RootSignature> mRootSignature;

	D3D12_INPUT_ELEMENT_DESC gInputLayout[2];

	u32 mPassCbvOffset = 0;

	s32 mCurrentFrameResourceIndex = 0;

	PassConstants mMainPassCB;

	UploadBuffer<ObjectConstants>* mObjectConstBuffer = nullptr;

	bool mIsWireFrame = false;

	POINT mLastMousePosition;

	XMFLOAT4X4 mWorld = Identity4x4();
	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = Identity4x4();
	XMFLOAT4X4 mProj = Identity4x4();

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	float mSunTheta = 1.25f * XM_PI;
	float mSunPhi = XM_PIDIV4;

	//Methods
private:
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentbackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	ID3D12Resource * CurrentBackbuffer();

	void processGlobalEvents();
	void calculateFrameStats();
	void buildRootSignature();
	void buildShaders();
	void buildPSO();
	void buildFrameResources();
	void drawRenderItems(ID3D12GraphicsCommandList* cmdList,
		std::vector<RenderItem>&rItems);

	void UpdateCamera();
	void UpdateMainPassCB(PassConstants& mainPassCB, UploadBuffer<PassConstants>* passCB);
	void UpdateObjectPassCB(std::vector<RenderItem>& rItems, UploadBuffer<ObjectConstants>* currentObjectCB);
	void UpdateMaterialCBs(std::vector<Material>& allMaterials, UploadBuffer<MaterialConstants>* currMat);

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyBoardInput();
};
