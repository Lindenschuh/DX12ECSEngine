#include "Components.h"

OOPRenderCompoment::OOPRenderCompoment(DX12Renderer* ren, OOPRenderItemDesc* desc)
{
	renderer = ren;
	static u32 Objindex = 0;
	MeshGeometry& md = renderer->mGeometrySystem->GetMeshGeomerty(desc->GeometryName);
	Material& mat = renderer->mMaterialSystem->GetMaterial(desc->MaterialName);
	Submesh& sMesh = md.Submeshes[desc->SubMeshName];
	RenderItem ritem;
	ritem.WorldPos = Identity4x4();
	ritem.ObjCBIndex = Objindex++;
	ritem.MatCBIndex = mat.MatCBIndex;
	ritem.texHeapIndex = mat.DiffuseSrvHeapIndex;
	ritem.GeoIndex = md.GeometryIndex;
	ritem.PrimitiveType = desc->PrimitiveType;
	ritem.IndexCount = sMesh.IndexCount;
	ritem.StartIndexLocation = sMesh.StartIndexLocation;
	ritem.baseVertexLocation = sMesh.BaseVertexLocation;

	PrimitiveType = ritem.PrimitiveType;
	layer = desc->Layer;
	GeoIndex = ritem.GeoIndex;
	texHeapIndex = ritem.texHeapIndex;
	MatCBIndex = ritem.MatCBIndex;
	renderItemId = renderer->mRItems[desc->Layer].size();
	IsDirty = false;
	renderer->AddRenderItem("box", ritem, desc->Layer);
}

void OOPRenderCompoment::Update(float time, float deltaTime)
{
	XMFLOAT3& pos = transFormComp->Position;
	RenderItem& rItem = renderer->mRItems[layer][renderItemId];
	XMMATRIX Mat = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixTranslation(pos.x, pos.y, pos.z));
	XMStoreFloat4x4(&rItem.WorldPos, Mat);

	rItem.GeoIndex = GeoIndex;
	rItem.MatCBIndex = MatCBIndex;
	rItem.PrimitiveType = PrimitiveType;
	rItem.TextureTransform = textureTransform;
	rItem.texHeapIndex = texHeapIndex;
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