#pragma once
#include "EntitySystem.h"
#include "../RenderCore/DX12Renderer.h"
#include "../Util/Waves.h"

class RenderSystem
{
private:
	DX12Renderer* renderer;
	EntityManger* eManager;
public:
	RenderSystem(EntityManger* manager, DX12Renderer* render);
	void SetRenderer(DX12Renderer* ren);
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

class DebugWindowSystem
{
private:
	RenderSystem* rSystem;
	EntityManger* eManager;
	std::vector<EntityID> changeableObjects;
	XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
public:
	DebugWindowSystem(RenderSystem* system, EntityManger* manager);
	void registerItem(EntityID eId);
	void UpdateSystem(float time, float deltaTime);
};

class GlobalMovement
{
private:
	std::vector<EntityID> entities;
	EntityManger* mEManager;
	XMFLOAT3 bounds = { 500.0f,500.0f,500.0f };

public:
	GlobalMovement(EntityManger* eManager);
	void AddToSystem(EntityID eId);
	void UpdateSystem(float time, float deltaTime);
};

class VisibilitySystem
{
private:
	EntityManger* mEManger;
	std::vector<EntityID> entities;
public:
	void AddToSystem(EntityID eId);
	VisibilitySystem(EntityManger* eMangager);
	void UpdateSystem(float time, float deltaTime);
};

struct RenderItemDesc
{
	std::string GeometryName;
	std::string SubMeshName;
	std::string MaterialName;
	RenderLayer::RenderLayer Layer;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType;
};

void static CreateRenderItem(RenderItemDesc* desc, DX12Renderer* render, EntityID eId, EntityManger* eManger)
{
	MeshGeometry& md = render->mGeometrySystem->GetMeshGeomerty(desc->GeometryName);
	Material& mat = render->mMaterialSystem->GetMaterial(desc->MaterialName);
	Submesh& sMesh = md.Submeshes[desc->SubMeshName];

	std::vector<RenderItem>& rItems = render->mRItems[desc->Layer];
	s32 Objindex = -1;

	for (int i = 0; i < rItems.size(); i++)
	{
		RenderItem& ri = rItems[i];
		if (ri.GeoIndex == md.GeometryIndex)
		{
			if (ri.IndexCount == sMesh.IndexCount && ri.baseVertexLocation == sMesh.BaseVertexLocation
				&& ri.StartIndexLocation == sMesh.StartIndexLocation)
			{
				Objindex = i;
				break;
			}
		}
	}

	RenderItem* ritem;

	if (Objindex == -1)
	{
		RenderItem tmpItem;
		tmpItem.ObjCBIndex = rItems.size();
		tmpItem.GeoIndex = md.GeometryIndex;

		tmpItem.PrimitiveType = desc->PrimitiveType;
		tmpItem.Bounds = sMesh.Bounds;
		tmpItem.IndexCount = sMesh.IndexCount;
		tmpItem.StartIndexLocation = sMesh.StartIndexLocation;
		tmpItem.baseVertexLocation = sMesh.BaseVertexLocation;

		render->AddRenderItem("box", tmpItem, desc->Layer);

		Objindex = tmpItem.ObjCBIndex;
		ritem = &rItems[tmpItem.ObjCBIndex];
	}
	else
	{
		ritem = &rItems[Objindex];
	}

	InstanceData id;
	id.MaterialIndex = mat.MatCBIndex;
	id.TexTransform = Identity4x4();
	id.World = Identity4x4();
	u32 instanceid = ritem->Instances.size();
	ritem->Instances.push_back(id);

	RenderComponent& rComp = eManger->mRenderData[eId];
	rComp.layer = desc->Layer;
	rComp.GeoIndex = ritem->GeoIndex;
	rComp.MatCBIndex = mat.MatCBIndex;
	rComp.renderItemID = Objindex;
}
