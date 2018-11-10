#include "transform.h"
#include "..\InitDX.h"

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
	DX12Render* renderer;

	XMFLOAT4X4 textureTransform = Identity4x4();
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	RenderLayer layer = RenderLayer::Opaque;
	u32 renderItemId = -1;
	u32 MatCBIndex = -1;
	u32 texHeapIndex = -1;
	u32 GeoIndex = -1;
	bool IsDirty;
public:
	OOPRenderCompoment(DX12Render* ren, OOPRenderItemDesc* desc);

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
	DX12Render* renderer;
	ImVec2 lastMousePosition;
	// Geerbt über IComponent
	OOPCameraComponent(DX12Render* ren);
	virtual void Init() override;
	virtual void Update(float time, float deltaTime) override;
};