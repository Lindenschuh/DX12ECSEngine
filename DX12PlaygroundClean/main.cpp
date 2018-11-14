#include "RenderCore/DX12Renderer.h"
#include "OOP/GameObject.h"
#include "OOP/Components.h"
#include "ECS/RenderSystem.h"
#include "Util/GeometryGenerator.h"

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
	system->LoadTexture("grassTex", L"Textures/grass.dds", DefaultTextureOptions());
	system->LoadTexture("waterTex", L"Textures/water1.dds", DefaultTextureOptions());
	system->LoadTexture("fenceTex", L"Textures/WoodCrate01.dds", DefaultTextureOptions());
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

	Submesh sm = { box.Indicies.size(),0,0 };
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
	MaterialConstants grass;
	//"grass";
	grass.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass.FresnelR0 = { 0.01f, 0.01f, 0.01f };
	grass.Roughness = 0.125f;
	system->BuildMaterial("grass", 0, grass);

	MaterialConstants water;
	water.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	water.FresnelR0 = { 0.2f, 0.2f, 0.2f };
	water.Roughness = 0.0f;
	system->BuildMaterial("water", 1, water);

	MaterialConstants wirefence;
	wirefence.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	wirefence.Roughness = 0.25f;
	system->BuildMaterial("wirefence", 2, wirefence);
}

static void CreatePSO(DX12Renderer* ren)
{
	ren->mShaderSystem->LoadShader("standardVS", L"Shaders\\color.hlsl", VertexShader);
	ren->mShaderSystem->LoadShader("opaquePS", L"Shaders\\color.hlsl", PixelShader);
	ren->mPSOSystem->BuildPSO("opaque", "standardVS", "opaquePS", DefaultPSOOptions());
}

//ECS
// int main()
// {
// 	DX12Render* render = new DX12Render(1280, 720, "Winnidow");
//
// 	buildBoxGeo(render->mGeometrySystem);
// 	loadTextures(render->mTextureSystem);
// 	buildMaterials(render->mMaterialSystem);
// 	CreatePSO(render);
// 	int boxCount = 100000;
// 	int maxWidth = (boxCount / 100);
// 	int height = 0;
// 	int width = 0;
// 	float maxSpeed = 8;
// 	float minSpeed = 3;
// 	GlobalMovement globalMovement(&gObjects);
//
// 	for (int i = 0; i < boxCount; i++)
// 	{
// 		EntityID eId = gObjects.addEntity("box");
// 		gObjects.mVelocitys[eId].Init(minSpeed, maxSpeed);
// 		gObjects.mFlags[eId] |= (gObjects.FlagPosition | gObjects.FlagRenderData);
//
// 		if (width > maxWidth)
// 		{
// 			width = 0;
// 			height += 10;
// 		}
// 		gObjects.mPositions[eId].Position = XMFLOAT3(width, 0.0f, height);
// 		width += 10;
// 		RenderItemDesc desc;
// 		desc.GeometryName = "boxGeo";
// 		desc.MaterialName = "wirefence";
// 		desc.SubMeshName = "box";
// 		desc.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
// 		desc.Layer = RenderLayer::Opaque;
// 		CreateRenderItem(&desc, render, i, &gObjects);
// 		globalMovement.AddToSystem(eId);
// 	}
//
// 	render->FinishSetup();
// 	EntityID cameraId = gObjects.addEntity("camera");
// 	gObjects.mFlags[cameraId] |= gObjects.FlagCamera;
//
// 	CameraSystem cameraSystem(&gObjects, render);
// 	cameraSystem.AddObjectToSystem(cameraId);
//
// 	GuiComponent guiSystem;
// 	RenderSystem renderSystem(&gObjects, render);
// 	DebugWindowSystem debugSystem(&renderSystem, &gObjects);
// 	debugSystem.registerItem(2);
//
// 	while (render->IsWindowActive())
// 	{
// 		guiSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
// 		debugSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
// 		globalMovement.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
// 		cameraSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
// 		renderSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
// 		render->Update();
// 		render->Draw();
// 	}
//
// 	return 0;
// }

// OOP
int main()
{
	DX12Renderer* render = new DX12Renderer(1280, 720, "Winnidow");

	buildBoxGeo(render->mGeometrySystem);
	loadTextures(render->mTextureSystem);
	buildMaterials(render->mMaterialSystem);
	CreatePSO(render);

	std::vector<GameObject *> AllGameObjects;

	int boxCount = 100000;
	int maxWidth = (boxCount / 100);
	int height = 0;
	int width = 0;
	float maxSpeed = 8;
	float minSpeed = 3;

	GameObject* gui = new GameObject();
	gui->RegisterCompoment(new OOPGuiComponent());

	AllGameObjects.push_back(gui);

	GameObject* cam = new GameObject();
	cam->RegisterCompoment(new OOPCameraComponent(render));
	AllGameObjects.push_back(cam);

	for (int i = 0; i < boxCount; i++)
	{
		GameObject* g = new GameObject();
		AllGameObjects.push_back(g);

		if (width > maxWidth)
		{
			width = 0;
			height += 10;
		}
		g->transFormComp->Position = XMFLOAT3(width, 0.0f, height);
		width += 10;
		OOPRenderItemDesc desc;
		desc.GeometryName = "boxGeo";
		desc.MaterialName = "wirefence";
		desc.SubMeshName = "box";
		desc.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		desc.Layer = RenderLayer::Opaque;
		g->RegisterCompoment(new OOPRenderCompoment(render, &desc));
		g->RegisterCompoment(new OOPMovementCompomenty());
	}

	render->FinishSetup();

	for (int i = 0; i < AllGameObjects.size(); i++)
	{
		AllGameObjects[i]->Init();
	}
	while (render->IsWindowActive())
	{
		for (int i = 0; i < AllGameObjects.size(); i++)
		{
			AllGameObjects[i]->Update(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		}
		render->Update();
		render->Draw();
	}
}