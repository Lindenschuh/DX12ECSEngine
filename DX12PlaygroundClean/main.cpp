#include "RenderCore/DX12Renderer.h"
#include "OOP/GameObject.h"
#include "OOP/Components.h"
#include "ECS/RenderSystem.h"

#include "ECS/FogSystem.h"
#include "ECS/CameraSystem.h"
#include "ECS/PositionSystem.h"
#include "ECS/ControllSystem.h"
#include "ECS/LightSystem.h"
#include "ECS/PhysicSystem.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "EXT/tiny_obj_loader.h"

static bool IsSameVertex(Vertex& v1, Vertex& v2)
{
	bool posBool = ((v1.Pos.x == v2.Pos.x) && (v1.Pos.y == v2.Pos.y) && (v1.Pos.z == v2.Pos.z));
	bool NormalBool = ((v1.Normal.x == v2.Normal.x) && (v1.Normal.y == v2.Normal.y) && (v1.Normal.z == v2.Normal.z));
	bool TexBool = ((v1.TexC.x == v2.TexC.x) && (v1.TexC.y == v2.TexC.y));

	return (posBool && NormalBool && TexBool);
}

static void GenerateIndex(std::vector<Vertex>& INverticies, std::vector<u16>& OUTindicies, std::vector<Vertex>& OUTverticies)
{
	for (int i = 0; i < INverticies.size(); i++)
	{
		Vertex v = INverticies[i];
		u16 index = 0;
		bool found = false;
		for (int j = 0; j < OUTverticies.size(); j++)
		{
			if (IsSameVertex(v, OUTverticies[j]))
			{
				found = true;
				break;
			}
			index++;
		}

		if (found)
		{
			OUTindicies.push_back(index);
			found = false;
		}
		else
		{
			OUTindicies.push_back(index);
			OUTverticies.push_back(v);
		}
		index = 0;
	}
}

static void GenerateTagent(std::vector<Vertex>& INOUTverticies, std::vector<u16>& INindicies)
{
	u32 vertexCount = INOUTverticies.size();

	std::vector<XMFLOAT3> tanA(vertexCount, XMFLOAT3(0.0f, 0.0f, 0.0f));
	std::vector<XMFLOAT3> tanB(vertexCount, XMFLOAT3(0.0f, 0.0f, 0.0f));

	u32 indexCount = INindicies.size();
	u32 triangleCount = (indexCount / 3);
	for (int i = 0; i < triangleCount; i++)
	{
		u16 i0 = INindicies[0 + i * 3];
		u16 i1 = INindicies[1 + i * 3];
		u16 i2 = INindicies[2 + i * 3];

		XMVECTOR pos0 = XMLoadFloat3(&INOUTverticies[i0].Pos);
		XMVECTOR pos1 = XMLoadFloat3(&INOUTverticies[i1].Pos);
		XMVECTOR pos2 = XMLoadFloat3(&INOUTverticies[i2].Pos);

		XMVECTOR tex0 = XMLoadFloat2(&INOUTverticies[i0].TexC);
		XMVECTOR tex1 = XMLoadFloat2(&INOUTverticies[i1].TexC);
		XMVECTOR tex2 = XMLoadFloat2(&INOUTverticies[i2].TexC);

		XMFLOAT3 edge0;
		XMFLOAT3 edge1;
		XMFLOAT2 uv0;
		XMFLOAT2 uv1;

		XMStoreFloat3(&edge0, XMVectorSubtract(pos1, pos0));
		XMStoreFloat3(&edge1, XMVectorSubtract(pos2, pos0));
		XMStoreFloat2(&uv0, XMVectorSubtract(tex1, tex0));
		XMStoreFloat2(&uv1, XMVectorSubtract(tex2, tex0));

		float r = 1.0f / (uv0.x * uv1.y - uv0.y * uv1.x);

		XMVECTOR tangent = XMVectorSet
		(
			((edge0.x * uv1.y) - (edge1.x * uv0.y)) * r,
			((edge0.y * uv1.y) - (edge1.y * uv0.y)) * r,
			((edge0.z * uv1.y) - (edge1.z * uv0.y)) * r,
			0.0f
		);

		XMVECTOR bitangent = XMVectorSet
		(
			((edge0.x * uv1.x) - (edge1.x * uv0.x)) * r,
			((edge0.y * uv1.x) - (edge1.y * uv0.x)) * r,
			((edge0.z * uv1.x) - (edge1.z * uv0.x)) * r,
			0.0f
		);

		XMVECTOR tanA0 = XMLoadFloat3(&tanA[i0]);
		XMVECTOR tanA1 = XMLoadFloat3(&tanA[i1]);
		XMVECTOR tanA2 = XMLoadFloat3(&tanA[i2]);

		XMVECTOR tanB0 = XMLoadFloat3(&tanB[i0]);
		XMVECTOR tanB1 = XMLoadFloat3(&tanB[i1]);
		XMVECTOR tanB2 = XMLoadFloat3(&tanB[i2]);

		XMStoreFloat3(&tanA[i0], XMVectorAdd(tanA0, tangent));
		XMStoreFloat3(&tanA[i1], XMVectorAdd(tanA1, tangent));
		XMStoreFloat3(&tanA[i2], XMVectorAdd(tanA2, tangent));

		XMStoreFloat3(&tanB[i0], XMVectorAdd(tanB0, bitangent));
		XMStoreFloat3(&tanB[i1], XMVectorAdd(tanB1, bitangent));
		XMStoreFloat3(&tanB[i2], XMVectorAdd(tanB2, bitangent));
	}

	for (int i = 0; i < vertexCount; i++)
	{
		XMVECTOR n = XMLoadFloat3(&INOUTverticies[i].Normal);
		XMVECTOR t0 = XMLoadFloat3(&tanA[i]);
		XMVECTOR t1 = XMLoadFloat3(&tanB[i]);

		XMVECTOR t = XMVectorSubtract(t0, XMVectorMultiply(n, XMVector3Dot(n, t0)));
		t = XMVector3Normalize(t);

		XMStoreFloat3(&INOUTverticies[i].Tangent, t);
	}
}

static BoundingBox GenerateBoundingBox(std::vector<Vertex> verticies)
{
	XMFLOAT3 minBounds = {};
	XMFLOAT3 maxBounds = {};

	for (int i = 0; i < verticies.size(); i++)
	{
		Vertex v = verticies[i];

		if (maxBounds.x < v.Pos.x) maxBounds.x = v.Pos.x;
		if (maxBounds.y < v.Pos.y) maxBounds.y = v.Pos.y;
		if (maxBounds.z < v.Pos.z) maxBounds.z = v.Pos.z;

		if (minBounds.x > v.Pos.x) minBounds.x = v.Pos.x;
		if (minBounds.y > v.Pos.y) minBounds.y = v.Pos.y;
		if (minBounds.z > v.Pos.z) minBounds.z = v.Pos.z;
	}

	XMFLOAT3 centerExtend = { (maxBounds.x + minBounds.x) / 2, (maxBounds.y + minBounds.y) / 2, (maxBounds.x + minBounds.z) / 2 };

	return BoundingBox(centerExtend, centerExtend);
}

static void loadOBJ(std::string&& path, std::string&& name, GeometrySystem* geoSys)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string error;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, path.c_str());

	if (!error.empty()) {
		printf("%s\n", error.c_str());
	}

	if (!ret) {
		return;
	}

	std::vector<Vertex> allVerts;
	std::vector<u16> allIndicies;
	std::vector<Submesh> allSubmeshes;
	std::vector<std::string>AllNames;

	u32 currentStartIndex = 0;
	u32 currentStartVertex = 0;

	for (int i = 0; i < shapes.size(); i++)
	{
		u32 indexoffset = 0;

		std::vector<Vertex> baseVerticies;

		std::vector<Vertex> indexedVerticies;
		std::vector<u16> indicies;

		for (int j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++)
		{
			int faceVert = shapes[i].mesh.num_face_vertices[j];

			for (int k = 0; k < faceVert; k++)
			{
				Vertex v = {};
				tinyobj::index_t idx = shapes[i].mesh.indices[indexoffset + k];
				v.Pos.x = attrib.vertices[3 * idx.vertex_index + 0];
				v.Pos.y = attrib.vertices[3 * idx.vertex_index + 1];
				v.Pos.z = attrib.vertices[3 * idx.vertex_index + 2];

				if (idx.normal_index != -1)
				{
					v.Normal.x = attrib.normals[3 * idx.normal_index + 0];
					v.Normal.y = attrib.normals[3 * idx.normal_index + 1];
					v.Normal.z = attrib.normals[3 * idx.normal_index + 2];
				}

				if (idx.texcoord_index != -1)
				{
					v.TexC.x = attrib.texcoords[2 * idx.texcoord_index + 0];
					v.TexC.y = attrib.texcoords[2 * idx.texcoord_index + 1];
				}
				baseVerticies.push_back(v);
			}

			indexoffset += faceVert;
		}

		GenerateIndex(baseVerticies, indicies, indexedVerticies);
		GenerateTagent(indexedVerticies, indicies);
		BoundingBox box = GenerateBoundingBox(indexedVerticies);
		Submesh sm;
		sm.Bounds = box;
		sm.BaseVertexLocation = currentStartVertex;
		sm.StartIndexLocation = currentStartIndex;
		sm.IndexCount = indicies.size();

		currentStartVertex += indexedVerticies.size();
		currentStartIndex += indicies.size();

		allVerts.insert(allVerts.end(), indexedVerticies.begin(), indexedVerticies.end());
		allIndicies.insert(allIndicies.end(), indicies.begin(), indicies.end());
		allSubmeshes.push_back(sm);
		AllNames.push_back(shapes[i].name);
	}

	GeoInfo info;
	info.Name = name;
	info.verts = allVerts.data();
	info.vertCount = allVerts.size();
	info.indicies = allIndicies.data();
	info.indiceCount = allIndicies.size();
	info.submeshs = allSubmeshes.data();
	info.submeshCount = allSubmeshes.size();
	info.SubmeshNames = AllNames.data();

	geoSys->LoadGeometry(info);
}

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
	system->LoadTexture("crateTex", L"Textures/bricks2.dds", DefaultTextureOptions());
	system->LoadTexture("waterTex", L"Textures/bricks2_nmap.dds", DefaultTextureOptions());
	system->LoadTexture("defaultTex", L"Textures/white1x1.dds", DefaultTextureOptions());
	system->LoadTexture("defaultTexNormal", L"Textures/default_nmap.dds", DefaultTextureOptions());
	system->LoadTexture("fenceTex", L"Textures/WireFence.dds", DefaultTextureOptions());
}

static void buildBoxGeo(GeometrySystem* system)
{
	GeometryGenerator geoGen;
	MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);
	MeshData quad = geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	MeshData plane = geoGen.CreateGrid(1000.0f, 1000.0f, 200, 200);

	std::vector<Vertex> vertices(box.Vertices.size() + quad.Vertices.size() + plane.Vertices.size());
	std::vector<u16> indicies;

	indicies.insert(indicies.end(), box.Indicies.begin(), box.Indicies.end());
	indicies.insert(indicies.end(), quad.Indicies.begin(), quad.Indicies.end());
	indicies.insert(indicies.end(), plane.Indicies.begin(), plane.Indicies.end());

	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexCoord;
		vertices[i].Tangent = box.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < quad.Vertices.size(); ++i)
	{
		auto& p = quad.Vertices[i].Position;
		vertices[i + box.Vertices.size()].Pos = p;
		vertices[i + box.Vertices.size()].Normal = quad.Vertices[i].Normal;
		vertices[i + box.Vertices.size()].TexC = quad.Vertices[i].TexCoord;
		vertices[i + box.Vertices.size()].Tangent = quad.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < quad.Vertices.size(); ++i)
	{
		auto& p = quad.Vertices[i].Position;
		vertices[i + box.Vertices.size()].Pos = p;
		vertices[i + box.Vertices.size()].Normal = quad.Vertices[i].Normal;
		vertices[i + box.Vertices.size()].TexC = quad.Vertices[i].TexCoord;
		vertices[i + box.Vertices.size()].Tangent = quad.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < plane.Vertices.size(); ++i)
	{
		auto& p = plane.Vertices[i].Position;
		vertices[i + box.Vertices.size() + quad.Vertices.size()].Pos = p;
		vertices[i + box.Vertices.size() + quad.Vertices.size()].Normal = plane.Vertices[i].Normal;
		vertices[i + box.Vertices.size() + quad.Vertices.size()].TexC = plane.Vertices[i].TexCoord;
		vertices[i + box.Vertices.size() + quad.Vertices.size()].Tangent = plane.Vertices[i].TangentU;
	}

	XMFLOAT3 boxCenterAndExtend = { 4.0f,4.0f,4.0f };
	BoundingBox boxBound(boxCenterAndExtend, boxCenterAndExtend);

	XMFLOAT3 planeCenterAndExtend = { 500.0f,0.001f,500.0f };
	BoundingBox planeBox(planeCenterAndExtend, planeCenterAndExtend);

	Submesh smBox = { box.Indicies.size(),0,0,boxBound };
	Submesh smQuad = { quad.Indicies.size(), box.Indicies.size(), box.Vertices.size() };

	Submesh smPlane = { plane.Indicies.size(), box.Indicies.size() + quad.Indicies.size(),
		box.Vertices.size() + quad.Vertices.size(), planeBox };

	std::string subMeshBox = "box";
	std::string subMeshQuad = "quad";
	std::string subMeshPlane = "plane";

	GeoInfo gInfo;
	gInfo.Name = "boxGeo";
	gInfo.verts = vertices.data();
	gInfo.vertCount = vertices.size();
	gInfo.indicies = indicies.data();
	gInfo.indiceCount = indicies.size();

	std::string subMeshName[] = { subMeshBox , subMeshQuad,subMeshPlane };
	gInfo.SubmeshNames = subMeshName;
	Submesh sms[] = { smBox, smQuad,smPlane };
	gInfo.submeshs = sms;
	gInfo.submeshCount = 3;

	system->LoadGeometry(gInfo);

	loadOBJ("Models\\cornell_box.obj", "OBJCornell", system);
}

static void buildMaterials(MaterialSystem* system)
{
	MaterialConstants crate;
	crate.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	crate.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	crate.Roughness = 0.25f;
	system->BuildMaterial("crate", 0, 0, crate);

	MaterialConstants water;
	water.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	water.FresnelR0 = { 0.2f, 0.2f, 0.2f };
	water.Roughness = 0.0f;
	system->BuildMaterial("water", 1, 0, water);

	MaterialConstants metall;
	metall.DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.1f, 1.0f);
	metall.FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	metall.Roughness = 0.1f;
	system->BuildMaterial("metal", 2, 3, metall);

	MaterialConstants brick;
	brick.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	brick.FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	brick.Roughness = 0.1f;
	system->BuildMaterial("brick", 0, 1, brick);

	MaterialConstants wirefence;
	wirefence.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	wirefence.Roughness = 0.25f;
	system->BuildMaterial("wirefence", 4, 0, wirefence);
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
	ren->mShaderSystem->LoadShader("shadowVS", L"Shaders\\shadow.hlsl", VertexShader);
	ren->mShaderSystem->LoadShader("shadowPS", L"Shaders\\shadow.hlsl", PixelShader);

	ren->mShaderSystem->LoadShader("shadowDebugVS", L"Shaders\\shadowDebug.hlsl", VertexShader);
	ren->mShaderSystem->LoadShader("shadowDebugPS", L"Shaders\\shadowDebug.hlsl", PixelShader);

	PSOOptions AlphaTestOptions = DefaultPSOOptions();

	ren->mPSOSystem->BuildPSO("opaque", "standardVS", "opaquePS", DefaultPSOOptions());
	ren->mPSOSystem->BuildTransparentPSO("transparent", "standardVS", "opaquePS", DefaultPSOOptions());
	ren->mPSOSystem->BuildTransparentPSO("alphaTest", "standardVS", "alphaTestPS", DefaultPSOOptions());
	ren->mPSOSystem->BuildShadowPSO("shadow", "shadowVS", "shadowPS", DefaultPSOOptions());
	ren->mPSOSystem->BuildPSO("debug", "shadowDebugVS", "shadowDebugPS", DefaultPSOOptions());

	ren->SetLayerPSO("opaque", RenderLayer::Opaque);
	ren->SetLayerPSO("transparent", RenderLayer::Transparent);
	ren->SetLayerPSO("alphaTest", RenderLayer::AlphaTest);
	ren->SetLayerPSO("shadow", RenderLayer::Shadow);
	ren->SetLayerPSO("debug", RenderLayer::ShadowDebug);
}

static void CreateShadowDebugView(DX12Renderer* render)
{
	EntityID eIdDebug = gObjects.addEntity("debug");
	RenderItemDesc desc;
	desc.GeometryName = "boxGeo";
	desc.MaterialName = "brick";
	desc.SubMeshName = "quad";
	desc.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	desc.Layer = RenderLayer::ShadowDebug;
	CreateRenderItem(&desc, render, eIdDebug, &gObjects);
}

static void OptionalThings(PhysicsSystem& PSystem, DX12Renderer* render)
{
	EntityID floor = gObjects.addEntity("floor");
	gObjects.mFlags[floor] |= gObjects.FlagRenderData;
	gObjects.mPositions[floor].Position = { 0.0f,2.0f,0.0f };
	RenderItemDesc descF;
	descF.GeometryName = "OBJCornell";
	descF.MaterialName = "metal";
	descF.SubMeshName = "floor";
	descF.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descF.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descF, render, floor, &gObjects);
	PSystem.AddStaticToSystem(floor);

	EntityID light = gObjects.addEntity("light");
	gObjects.mFlags[light] |= gObjects.FlagRenderData;
	gObjects.mPositions[light].Position = { 0.0f,2.0f,0.0f };
	RenderItemDesc descL;
	descL.GeometryName = "OBJCornell";
	descL.MaterialName = "metal";
	descL.SubMeshName = "light";
	descL.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descL.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descL, render, light, &gObjects);
	PSystem.AddStaticToSystem(light);

	EntityID ceiling = gObjects.addEntity("ceiling");
	gObjects.mFlags[ceiling] |= gObjects.FlagRenderData;
	gObjects.mPositions[ceiling].Position = { 0.0f,2.0f,0.0f };
	RenderItemDesc descC;
	descC.GeometryName = "OBJCornell";
	descC.MaterialName = "metal";
	descC.SubMeshName = "ceiling";
	descC.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descC.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descC, render, ceiling, &gObjects);
	PSystem.AddStaticToSystem(ceiling);

	EntityID back_wall = gObjects.addEntity("back_wall");
	gObjects.mFlags[back_wall] |= gObjects.FlagRenderData;
	gObjects.mPositions[back_wall].Position = { 0.0f,2.0f,0.0f };
	RenderItemDesc descb;
	descb.GeometryName = "OBJCornell";
	descb.MaterialName = "metal";
	descb.SubMeshName = "back_wall";
	descb.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descb.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descb, render, back_wall, &gObjects);
	PSystem.AddStaticToSystem(back_wall);

	EntityID green_wall = gObjects.addEntity("green_wall");
	gObjects.mFlags[green_wall] |= gObjects.FlagRenderData;
	gObjects.mPositions[green_wall].Position = { 0.0f,2.0f,0.0f };
	RenderItemDesc descg;
	descg.GeometryName = "OBJCornell";
	descg.MaterialName = "metal";
	descg.SubMeshName = "green_wall";
	descg.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descg.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descg, render, green_wall, &gObjects);
	PSystem.AddStaticToSystem(green_wall);

	EntityID red_wall = gObjects.addEntity("red_wall");
	gObjects.mFlags[red_wall] |= gObjects.FlagRenderData;
	gObjects.mPositions[red_wall].Position = { 0.0f,2.0f,0.0f };
	RenderItemDesc descr;
	descr.GeometryName = "OBJCornell";
	descr.MaterialName = "metal";
	descr.SubMeshName = "red_wall";
	descr.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descr.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descr, render, red_wall, &gObjects);
	PSystem.AddStaticToSystem(red_wall);

	EntityID short_block = gObjects.addEntity("short_block");
	gObjects.mFlags[short_block] |= gObjects.FlagRenderData;
	gObjects.mPositions[short_block].Position = { 0.0f,2.0f,0.0f };
	RenderItemDesc descSB;
	descSB.GeometryName = "OBJCornell";
	descSB.MaterialName = "metal";
	descSB.SubMeshName = "short_block";
	descSB.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descSB.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descSB, render, short_block, &gObjects);
	PSystem.AddStaticToSystem(short_block);

	EntityID tall_block = gObjects.addEntity("tall_block");
	gObjects.mFlags[tall_block] |= gObjects.FlagRenderData;
	gObjects.mPositions[tall_block].Position = { 0.0f,-1.0f,0.0f };
	RenderItemDesc descTB;
	descTB.GeometryName = "boxGeo";
	descTB.MaterialName = "metal";
	descTB.SubMeshName = "tall_block";
	descTB.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	descTB.Layer = RenderLayer::Opaque;
	CreateRenderItem(&descTB, render, tall_block, &gObjects);
	PSystem.AddStaticToSystem(tall_block);

	CreateShadowDebugView(render);
}

//ECS
int main()
{
	//LoadGltf("Models\\Triangle\\glTF\\Triangle.gltf", &loader, &model);

	DX12Renderer* render = new DX12Renderer(1920, 1080, "Winnidow", &gObjects);

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
	VisibilitySystem visSystem(&gObjects);
	LightSystem liSystem(render);
	PhysicsSystem PSystem(&gObjects, render);

	int boxCount = 1000;
	int maxWidth = (boxCount / 100);
	int height = 0;
	int width = 0;
	float maxSpeed = 8;
	float minSpeed = 3;

	for (int i = 0; i < boxCount; i++)
	{
		EntityID eId = gObjects.addEntity("box" + i);
		gObjects.mVelocitys[eId].Init(minSpeed, maxSpeed);
		gObjects.mFlags[eId] |= gObjects.FlagRenderData;
		if (width > maxWidth)
		{
			width = 0;
			height += 10;
		}
		gObjects.mPositions[eId].Position = { (float)width, 0.0f, (float)height };

		width += 10;
		RenderItemDesc desc;
		desc.GeometryName = "boxGeo";
		desc.MaterialName = "brick";
		desc.SubMeshName = "box";
		desc.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		desc.Layer = RenderLayer::Opaque;
		CreateRenderItem(&desc, render, eId, &gObjects);
		globalMovement.AddToSystem(eId);
		//PSystem.AddDynamicToSystem(eId);
		visSystem.AddToSystem(eId);
	}

	for (int i = 0; i < boxCount; i++)
	{
		EntityID eId = gObjects.addEntity("boxTrans" + i);
		gObjects.mVelocitys[eId].Init(minSpeed, maxSpeed);
		gObjects.mFlags[eId] |= gObjects.FlagRenderData;
		if (width > maxWidth)
		{
			width = 0;
			height += 10;
		}
		gObjects.mPositions[eId].Position = { (float)width, 0.0f, (float)height };

		width += 10;
		RenderItemDesc desc;
		desc.GeometryName = "boxGeo";
		desc.MaterialName = "wirefence";
		desc.SubMeshName = "box";
		desc.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		desc.Layer = RenderLayer::AlphaTest;
		CreateRenderItem(&desc, render, eId, &gObjects);
		globalMovement.AddToSystem(eId);
		//PSystem.AddDynamicToSystem(eId);
		visSystem.AddToSystem(eId);
	}

	EntityID cameraId = gObjects.addEntity("camera");
	render->mCameraSystem->AddObjectToSystem(cameraId);
	gObjects.mPositions[cameraId].Position = { 30.0f,5.0f,-10.0f };
	ConSystem.AddToSystem(cameraId);

	render->FinishSetup();

	while (render->IsWindowActive())
	{
		guiSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);

		globalMovement.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		visSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		ConSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		//PSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		liSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		renderSystem.UpdateSystem(ImGui::GetTime(), ImGui::GetIO().DeltaTime);

		render->Update(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
		render->Draw();
	}

	return 0;
}