#include "Components.h"

OOPRenderCompoment::OOPRenderCompoment(DX12Renderer* ren, OOPRenderItemDesc* desc)
{
	renderer = ren;
	MeshGeometry& md = renderer->mGeometrySystem->GetMeshGeomerty(desc->GeometryName);
	Material& mat = renderer->mMaterialSystem->GetMaterial(desc->MaterialName);
	Submesh& sMesh = md.Submeshes[desc->SubMeshName];

	std::vector<RenderItem>& rItems = ren->mRItems[desc->Layer];
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

		ren->AddRenderItem("box", tmpItem, desc->Layer);

		ritem = &rItems[tmpItem.ObjCBIndex];
		Objindex = tmpItem.ObjCBIndex;
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

	this->layer = desc->Layer;
	this->GeoIndex = ritem->GeoIndex;
	this->instanceID = instanceid;
	this->MatCBIndex = mat.MatCBIndex;
	this->renderItemID = Objindex;
}

void OOPRenderCompoment::Update(float time, float deltaTime)
{
	XMFLOAT3& pos = transFormComp->Position;
	InstanceData&  instance = renderer->mRItems[layer][renderItemID].Instances[instanceID];

	XMStoreFloat4x4(&instance.World,
		XMMatrixMultiply(XMMatrixIdentity(), XMMatrixTranslation(transFormComp->Position.x,
			transFormComp->Position.y, transFormComp->Position.z)));

	instance.MaterialIndex = MatCBIndex;
	instance.TexTransform = textureTransform;
}
void OOPGuiComponent::Init()
{
}

void OOPGuiComponent::Update(float time, float deltaTime)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();
}

void OOPMovementCompomenty::Init()
{
	float angle = RandomFloat01() * 3.1415926f * 2;
	// random movement speed between given min & max
	float speed = RandomFloat(3, 8);
	// velocity x & y components
	velo.x = cosf(angle) * speed;
	velo.z = sinf(angle) * speed;
	velo.y = tanf(angle) * speed;
}

void OOPMovementCompomenty::Update(float time, float deltaTime)
{
	XMFLOAT3& pos = transFormComp->Position;
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

OOPCameraComponent::OOPCameraComponent(DX12Renderer * ren)
{
	renderer = ren;
}

void OOPCameraComponent::Init()
{
}

void OOPCameraComponent::Update(float time, float deltaTime)
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

	XMFLOAT3& eyePos = transFormComp->Position;

	eyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
	eyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
	eyePos.y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&ViewMat, view);
	if (isMain)
		renderer->SetMainCamera(eyePos, ViewMat);
}