#include "RenderSystem.h"

RenderSystem::RenderSystem(EntityManager * manager, DX12Renderer * render)
{
	eManager = manager;
	renderer = render;
}

void RenderSystem::SetRenderer(DX12Renderer * ren)
{
	renderer = ren;
}

Material* RenderSystem::GetMaterial(EntityID id)
{
	return &renderer->mMaterialSystem->GetMaterial(eManager->mRenderData[id].MatCBIndex);
}

MeshGeometry* RenderSystem::GetGeometry(EntityID id)
{
	return &renderer->mGeometrySystem->GetMeshGeomerty(eManager->mRenderData[id].GeoIndex);
}

FrameResource* RenderSystem::GetCurrentFrameResource()
{
	return &renderer->mFrameResourceSystem->GetCurrentFrameResource();
}

void RenderSystem::UpdateSystem(float time, float deltaTime)
{
	for (int i = 0; i < eManager->mFlags.size(); i++)
	{
		EntityID eId = i;
		if ((eManager->mFlags[eId] & eManager->FlagRenderData) && (eManager->mFlags[eId] & eManager->FlagPosition))
		{
			RenderComponent& rComp = eManager->mRenderData[eId];
			PositionComponent& pComp = eManager->mPositions[eId];
			GeometryBatch& rItem = renderer->mAllGeometryBatches[rComp.layer][rComp.renderItemID];
			InstanceData&  instance = rItem.Instances[rItem.InstanceUpdated++];
			FXMVECTOR position = XMLoadFloat3(&pComp.Position);
			FXMVECTOR rotation = XMLoadFloat4(&pComp.RoationQuat);
			FXMVECTOR scaling = XMLoadFloat3(&pComp.Scaling);

			XMStoreFloat4x4(
				&instance.World,
				XMMatrixAffineTransformation(
					scaling,
					XMQuaternionIdentity(),
					rotation,
					position
				));

			instance.MaterialIndex = rComp.MatCBIndex;
			instance.TexTransform = rComp.textureTransform;
		}
	}
}

void GuiSystem::UpdateSystem(float time, float deltaTime)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();
}

DebugWindowSystem::DebugWindowSystem(RenderSystem* system, EntityManager* manager)
{
	rSystem = system;
	eManager = manager;
}
void DebugWindowSystem::registerItem(EntityID eId)
{
	changeableObjects.emplace_back(eId);
}
void DebugWindowSystem::UpdateSystem(float time, float deltaTime)
{
	//ui
	{
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("Number Of Entities: %i", eManager->mNames.size()); // Display some text (you can use a format strings too)     // Edit bools storing our window open/close state

		ImGui::ColorPicker4("Pick Color", &color.x);         // Edit 1 float using a slider from 0.0f to 1.0f

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	for (int i = 0; i < changeableObjects.size(); i++)
	{
		EntityID eId = changeableObjects[i];

		rSystem->GetMaterial(eId)->setDiffuseAlbedo(color);
	}
}

GlobalMovement::GlobalMovement(EntityManager * eManager)
{
	mEManager = eManager;
}

void GlobalMovement::AddToSystem(EntityID eId)
{
	entities.emplace_back(eId);
	mEManager->mFlags[eId] |= mEManager->FlagVeloctiy;
}

void GlobalMovement::UpdateSystem(float time, float deltaTime)
{
	for (int i = 0; i < entities.size(); i++)
	{
		EntityID eId = entities[i];

		XMFLOAT3& pos = mEManager->mPositions[eId].Position;
		XMFLOAT3& velo = mEManager->mVelocitys[eId].Velocity;
		pos.x += velo.x * deltaTime;
		pos.y += velo.y * deltaTime;
		pos.z += velo.z * deltaTime;

		if (pos.x < (-bounds.x))
		{
			velo = XMFLOAT3(-velo.x, -velo.y, -velo.z);
			pos.x = -bounds.x;
		}
		if (pos.x > bounds.x)
		{
			velo = XMFLOAT3(-velo.x, -velo.y, -velo.z);
			pos.x = bounds.x;
		}

		if (pos.y < (-bounds.y))
		{
			velo = XMFLOAT3(-velo.x, -velo.y, -velo.z);
			pos.y = -bounds.y;
		}
		if (pos.y > bounds.y)
		{
			velo = XMFLOAT3(-velo.x, -velo.y, -velo.z);
			pos.y = bounds.y;
		}

		if (pos.z < (-bounds.z))
		{
			velo = XMFLOAT3(-velo.x, -velo.y, -velo.z);
			pos.z = -bounds.z;
		}
		if (pos.z > bounds.z)
		{
			velo = XMFLOAT3(-velo.x, -velo.y, -velo.z);
			pos.z = bounds.z;
		}
	}
}

VisibilitySystem::VisibilitySystem(EntityManager * eMangager)
{
	mEManger = eMangager;
}

void VisibilitySystem::UpdateSystem(float time, float deltaTime)
{
	if (ImGui::IsKeyPressed('V'))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			EntityID eId = entities[i];
			mEManger->mFlags[eId] ^= EntityManager::FlagRenderData;
		}
	}
}

void VisibilitySystem::AddToSystem(EntityID eId)
{
	entities.push_back(eId);
}