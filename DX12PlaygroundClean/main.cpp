#include "RenderCore/DX12Renderer.h"
#include "OOP/GameObject.h"
#include "OOP/Components.h"
#include "ECS/RenderSystem.h"

#include "ECS/FogSystem.h"
#include "ECS/CameraSystem.h"
#include "ECS/PositionSystem.h"
#include "ECS/ControllSystem.h"

static float GetHillsHeight(float x, float z)
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}

static XMFLOAT3 GetHillsNormal(float x, float z)
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

static void loadTextures(TextureSystem* system)
{
	system->LoadTexture("crateTex", L"Textures/WoodCrate01.dds", DefaultTextureOptions());
	system->LoadTexture("waterTex", L"Textures/water1.dds", DefaultTextureOptions());
	system->LoadTexture("defaultTex", L"Textures/white1x1.dds", DefaultTextureOptions());
}

static void buildBoxGeo(GeometrySystem* system)
{
	GeometryGenerator geoGen;
	MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexCoord;
	}

	XMFLOAT3 boxCenterAndExtend = { 4.0f,4.0f,4.0f };
	BoundingBox boxBound(boxCenterAndExtend, boxCenterAndExtend);

	Submesh sm = { box.Indicies.size(),0,0,boxBound };

	std::string subMeshName = "box";
	GeoInfo gInfo;
	gInfo.Name = "boxGeo";
	gInfo.verts = vertices.data();
	gInfo.vertCount = vertices.size();
	gInfo.indicies = box.Indicies.data();
	gInfo.indiceCount = box.Indicies.size();
	gInfo.SubmeshNames = &subMeshName;
	gInfo.submeshs = &sm;
	gInfo.submeshCount = 1;

	system->LoadGeometry(gInfo);
}

static void buildMaterials(MaterialSystem* system)
{
	MaterialConstants crate;
	crate.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	crate.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	crate.Roughness = 0.25f;
	system->BuildMaterial("crate", 0, crate);

	MaterialConstants water;
	water.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	water.FresnelR0 = { 0.2f, 0.2f, 0.2f };
	water.Roughness = 0.0f;
	system->BuildMaterial("water", 1, water);

	MaterialConstants wirefence;
	wirefence.DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.1f, 1.0f);
	wirefence.FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	wirefence.Roughness = 0.1f;
	system->BuildMaterial("metal", 2, wirefence);
}

static void CreatePSO(DX12Renderer* ren)
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL,NULL
	};
	ren->mShaderSystem->LoadShader("standardVS", L"Shaders\\color.hlsl", VertexShader);
	ren->mShaderSystem->LoadShader("opaquePS", L"Shaders\\color.hlsl", PixelShader);
	ren->mShaderSystem->LoadShader("alphaTestPS", L"Shaders\\color.hlsl", PixelShader, alphaTestDefines);

	PSOOptions AlphaTestOptions = DefaultPSOOptions();

	ren->mPSOSystem->BuildPSO("opaque", "standardVS", "opaquePS", DefaultPSOOptions());
	ren->mPSOSystem->BuildTransparentPSO("transparent", "standardVS", "opaquePS", DefaultPSOOptions());
	ren->mPSOSystem->BuildTransparentPSO("alphaTest", "standardVS", "alphaTestPS", DefaultPSOOptions());

	ren->SetLayerPSO("transparent", RenderLayer::Opaque);
	ren->SetLayerPSO("transparent", RenderLayer::Transparent);
	ren->SetLayerPSO("alphaTest", RenderLayer::AlphaTest);
}

//ECS
int main()
{
	DX12Renderer* render = new DX12Renderer(1280, 720, "Winnidow", &gObjects);

	buildBoxGeo(render->mGeometrySystem);
	loadTextures(render->mTextureSystem);
	buildMaterials(render->mMaterialSystem);
	CreatePSO(render);

	PositionSystem PosSystem(&gObjects);
	GlobalMovement globalMovement(&gObjects);
	GuiSystem guiSystem;
	RenderSystem renderSystem(&gObjects, render);
	FogSystem fogSystem(&gObjects, render);
	ControllSystem ConSystem(&gObjects, &PosSystem);

	int boxCount = 1000;
	int maxWidth = (boxCount / 100);
	int height = 0;
	int width = 0;
	float maxSpeed = 8;
	float minSpeed = 3;

	for (int i = 0; i < boxCount; i++)
	{
		EntityID eId = gObjects.addEntity("box");
		gObjects.mVelocitys[eId].Init(minSpeed, maxSpeed);
		gObjects.mFlags[eId] |= gObjects.FlagRenderData;
		if (width > maxWidth)
		{
			width = 0;
			height += 10;
		}
		gObjects.mPositions[eId].Position = XMFLOAT3(width, 0.0f, height);
		width += 10;
		RenderItemDesc desc;
		desc.GeometryName = "boxGeo";
		desc.MaterialName = "metal";
		desc.SubMeshName = "box";
		desc.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		desc.Layer = RenderLayer::AlphaTest;
		CreateRenderItem(&desc, render, eId, &gObjects);
		globalMovement.AddToSystem(eId);
	}

	EntityID cameraId = gObjects.addEntity("camera");
	render->mCameraSystem->AddObjectToSystem(cameraId);
	ConSystem.AddToSystem(cameraId);

	render->FinishSetup();

	EntityID fogId = gObjects.addEntity("Fog");
	fogSystem.AddEntity(fogId);

	while (render->IsWindowActive())
	{
		guiSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);

		globalMovement.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		//fogSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);

		ConSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		renderSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);

		render->Update(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		render->Draw();
	}

	return 0;
}