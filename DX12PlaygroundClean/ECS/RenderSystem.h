#pragma once
#include "EntitySystem.h"
#include "../InitDX.h"
#include "../Waves.h"

class RenderSystem
{
private:
	DX12Render* renderer;
	EntityManger* eManager;
public:
	RenderSystem(EntityManger* manager, DX12Render* render);
	void SetRenderer(DX12Render* ren);
	Material* GetMaterial(EntityID id);
	MeshGeometry* GetGeometry(EntityID id);
	FrameResource* GetCurrentFrameResource();
	void UpdateSystem(float time, float deltaTime);
};

class GuiSystem
{
public:
	void UpdateSystem(float time, float deltaTime);
};

class ColorChangerSystem
{
private:
	RenderSystem* rSystem;
	EntityManger* eManager;
	std::vector<EntityID> changeableObjects;
	XMFLOAT4 color;
public:
	ColorChangerSystem(RenderSystem* system, EntityManger* manager);
	void registerItem(EntityID eId);
	void UpdateSystem(float time, float deltaTime);
};

class CameraSystem
{
private:
	std::vector<EntityID> entities;
	DX12Render* renderer;
	EntityManger* mEManager;

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;
	ImVec2 lastMousePosition;
public:
	CameraSystem(EntityManger* eManager, DX12Render* ren);
	void AddObjectToSystem(EntityID id);
	void UpdateSystem(float time, float deltaTime);
};

class WaterAnimationSystem
{
private:
	std::vector<EntityID> entities;
	EntityManger* mEManager;
	RenderSystem* mRs;
	float baseTime = 0.0f;
	Waves* mWaves;
public:
	WaterAnimationSystem(EntityManger* eManager, Waves* waves, RenderSystem* rs);
	void AddToSystem(EntityID id);
	void UpdateSystem(float time, float deltaTime);
};

struct RenderItemDesc
{
	std::string GeometryName;
	std::string SubMeshName;
	std::string MaterialName;
	RenderLayer Layer;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType;
};

void static CreateRenderItem(RenderItemDesc* desc, DX12Render* render, EntityID eId, EntityManger* eManger)
{
	static u32 Objindex = 0;
	MeshGeometry* md = render->GetGeometry(desc->GeometryName);
	Material* mat = render->GetMaterial(desc->MaterialName);
	Submesh* sMesh = &md->Submeshes[desc->SubMeshName];
	RenderItem ritem;
	ritem.WorldPos = Identity4x4();
	ritem.ObjCBIndex = Objindex++;
	ritem.MatCBIndex = mat->MatCBIndex;
	ritem.texHeapIndex = mat->DiffuseSrvHeapIndex;
	ritem.GeoIndex = md->GeometryIndex;
	ritem.PrimitiveType = desc->PrimitiveType;
	ritem.IndexCount = sMesh->IndexCount;
	ritem.StartIndexLocation = sMesh->StartIndexLocation;
	ritem.baseVertexLocation = sMesh->BaseVertexLocation;

	RenderComponent& rComp = eManger->mRenderData[eId];
	rComp.worldPos = ritem.WorldPos;
	rComp.PrimitiveType = ritem.PrimitiveType;
	rComp.layer = desc->Layer;
	rComp.GeoIndex = ritem.GeoIndex;
	rComp.texHeapIndex = ritem.texHeapIndex;
	rComp.MatCBIndex = ritem.MatCBIndex;
	rComp.renderItemId = render->mRItems[desc->Layer].size();
	rComp.IsDirty = false;
	render->AddRenderItem(eManger->mNames[eId], ritem, desc->Layer);
}