#include "transform.h"
#include "..\RenderCore\DX12Renderer.h"

struct OOPRenderItemDesc
{
	std::string GeometryName;
	std::string SubMeshName;
	std::string MaterialName;
	RenderLayer Layer;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType;
};

class OOPRenderCompoment : public IComponent
{
private:
	DX12Renderer* renderer;

	RenderLayer layer = RenderLayer::Opaque;
	XMFLOAT4X4 textureTransform = Identity4x4();
	u32 renderItemID = -1;
	GeometryID GeoIndex = -1;
	u32 instanceID = -1;
	MaterialID MatCBIndex = -1;

public:
	OOPRenderCompoment(DX12Renderer* ren, OOPRenderItemDesc* desc);

	virtual void Init() override {}

	virtual void Update(float time, float deltaTime) override;
};

class OOPGuiComponent : public IComponent
{
public:
	// Geerbt über IComponent
	virtual void Init() override;
	virtual void Update(float time, float deltaTime) override;
};

class OOPMovementCompomenty : public IComponent
{
public:
	XMFLOAT3 velo;
	XMFLOAT3 bounds = { 500.0f,500.0f,500.0f };
	// Geerbt über IComponent
	virtual void Init() override;

	virtual void Update(float time, float deltaTime) override;
};

class OOPCameraComponent :public IComponent
{
public:

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;
	bool isMain = true;
	XMFLOAT4X4 ViewMat = Identity4x4();
	DX12Renderer* renderer;
	ImVec2 lastMousePosition;
	// Geerbt über IComponent
	OOPCameraComponent(DX12Renderer* ren);
	virtual void Init() override;
	virtual void Update(float time, float deltaTime) override;
};