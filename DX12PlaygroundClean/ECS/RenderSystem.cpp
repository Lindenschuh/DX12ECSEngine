#include "RenderSystem.h"

RenderSystem::RenderSystem(EntityManger * manager, DX12Renderer * render)
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
			InstanceData&  instance = renderer->mRItems[rComp.layer][rComp.renderItemID].Instances[rComp.instanceID];

			XMStoreFloat4x4(&instance.World,
				XMMatrixMultiply(XMMatrixIdentity(), XMMatrixTranslation(pComp.Position.x, pComp.Position.y, pComp.Position.z)));
			instance.MaterialIndex = rComp.MatCBIndex;
			instance.TexTransform = rComp.textureTransform;
		}
	}
}

void GuiComponent::UpdateSystem(float time, float deltaTime)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();
}

DebugWindowSystem::DebugWindowSystem(RenderSystem* system, EntityManger* manager)
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

CameraSystem::CameraSystem(EntityManger * eManager, DX12Renderer* ren)
{
	mEManager = eManager;
	renderer = ren;
}

void CameraSystem::AddObjectToSystem(EntityID id)
{
	entities.emplace_back(id);
}

void CameraSystem::UpdateSystem(float time, float deltaTime)
{
	if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
		lastMousePosition = ImGui::GetMousePos();

	if (ImGui::IsMouseDown(0) && ImGui::IsKeyDown(VK_SPACE))
	{
		float dx = XMConvertToRadians(0.25f * (ImGui::GetMousePos().x - lastMousePosition.x));
		float dy = XMConvertToRadians(0.25f * (ImGui::GetMousePos().y - lastMousePosition.y));

		mTheta += dx;
		mPhi += dy;

		mPhi = Clamp(mPhi, 0.1f, XM_PI - 0.1f);

		lastMousePosition = ImGui::GetMousePos();
	}
	else if (ImGui::IsMouseDown(1) && ImGui::IsKeyDown(VK_SPACE))
	{
		float dx = 0.05f * (ImGui::GetMousePos().x - lastMousePosition.x);
		float dy = 0.05f * (ImGui::GetMousePos().y - lastMousePosition.y);

		mRadius += dx - dy;
		mRadius = Clamp(mRadius, 5.0f, 150.0f);
		lastMousePosition = ImGui::GetMousePos();
	}

	for (int i = 0; i < entities.size(); i++)
	{
		EntityID eId = entities[i];
		XMFLOAT3& eyePos = mEManager->mPositions[eId].Position;

		eyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
		eyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
		eyePos.y = mRadius * cosf(mPhi);

		XMVECTOR pos = XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mEManager->mCameras[eId].ViewMat, view);
		if (&mEManager->mCameras[eId].isMain)
			renderer->SetMainCamera(eyePos, mEManager->mCameras[eId].ViewMat);
	}
}

ControllSystem::ControllSystem(EntityManger * eManager)
{
	mEManager = eManager;
}

void ControllSystem::AddToSystem(EntityID id)
{
	entities.emplace_back(id);
}

void ControllSystem::UpdateSystem(float time, float deltaTime)
{
	for (int i = 0; i < entities.size(); i++)
	{
		EntityID eId = entities[i];
		PositionComponent& pos = mEManager->mPositions[eId];

		if (ImGui::IsKeyDown('W'))
		{
			pos.Position.x += speed * deltaTime;
		}

		if (ImGui::IsKeyDown('S'))
		{
			pos.Position.x -= speed * deltaTime;
		}

		if (ImGui::IsKeyDown('A'))
		{
			pos.Position.z += speed * deltaTime;
		}

		if (ImGui::IsKeyDown('D'))
		{
			pos.Position.z -= speed * deltaTime;
		}

		if (ImGui::IsKeyDown('E'))
		{
			pos.Position.y += speed * deltaTime;
		}

		if (ImGui::IsKeyDown('Q'))
		{
			pos.Position.y -= speed * deltaTime;
		}
	}
}

GlobalMovement::GlobalMovement(EntityManger * eManager)
{
	mEManager = eManager;
}

void GlobalMovement::AddToSystem(EntityID eId)
{
	entities.emplace_back(eId);
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