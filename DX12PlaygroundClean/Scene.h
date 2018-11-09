#pragma once
#include "Default.h"
#include "DXData.h"
#include "FrameResource.h"

class Scene
{
public:
	XMFLOAT4X4 mProj = Identity4x4();
	GameTimer mSceneTimer;
	FrameResource* mCurrentFrameResource;

	std::vector<FrameResource> mFrameResources;
	std::vector<RenderItem> mRItems[RenderLayer::Count];
	std::vector<MeshGeometry> mAllGeometry;
	std::vector<Material> mAllMaterials;
	std::vector<Texture> mAllTextures;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map<std::string, u32> mGeometeriesIndex;
	std::unordered_map<std::string, u32> mMaterialsIndex;
	std::unordered_map<std::string, u32> mTextureIndex;
	std::unordered_map<std::string, std::pair<RenderLayer, u32>> mRenderItemPair;

	s32 mCurrentFrameResourceIndex = 0;

	void AddMaterials(std::string* names, Material* mats, u32 count);
	void AddGeometries(std::string* names, MeshGeometry geos, u32 count);
	void AddTextures(std::string* names, Texture* t, u32 count);
	void AddRenderItem(std::string* names, RenderItem* r, RenderLayer* rl, u32 count);

	void AddMaterial(std::string name, Material m);
	void AddGeometry(std::string name, MeshGeometry g);
	void AddTexture(std::string name, Texture t);
	void AddRenderItem(std::string name, RenderItem r, RenderLayer rl);

	MeshGeometry* GetGeometry(std::string name);
	Material*	GetMaterial(std::string name);
	Texture*	GetTexture(std::string name);
	RenderItem* GetRenderItem(std::string name);
	Scene();

	~Scene();
};

void InitScene(Scene* scene);
void UpdateScene(Scene* scene);
