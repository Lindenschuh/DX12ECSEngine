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

class GuiComponent
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

class CameraSystem
{
private:
	std::vector<EntityID> entities;
	DX12Renderer* renderer;
	EntityManger* mEManager;

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;
	ImVec2 lastMousePosition;
public:
	CameraSystem(EntityManger* eManager, DX12Renderer* ren);
	void AddObjectToSystem(EntityID id);
	void UpdateSystem(float time, float deltaTime);
};

class ControllSystem
{
private:
	std::vector<EntityID> entities;
	EntityManger* mEManager;
	float speed = 10.0f;
public:
	ControllSystem(EntityManger* eManager);
	void AddToSystem(EntityID id);
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

struct RenderItemDesc
{
	std::string GeometryName;
	std::string SubMeshName;
	std::string MaterialName;
	RenderLayer Layer;
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
	rComp.instanceID = instanceid;
	rComp.MatCBIndex = mat.MatCBIndex;
	rComp.renderItemID = Objindex;
}
