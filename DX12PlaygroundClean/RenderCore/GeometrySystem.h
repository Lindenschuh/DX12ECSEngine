#pragma once
#include "../Core/Default.h"
#include "DX12Context.h"
#include "DXData.h"

struct GeoInfo
{
	std::string Name;
	std::string* SubmeshNames;

	Submesh* submeshs;
	Vertex* verts;
	u16* indicies;

	u32 vertCount;
	u32 indiceCount;
	u32 submeshCount;
};

class GeometrySystem
{
private:
	std::vector<MeshGeometry> mAllGeometry;
	std::unordered_map<std::string, u32> mGeometryIDs;
	DX12Context* mDXContext;
public:
	GeometrySystem(DX12Context* DXContext);
	GeometryID LoadGeometry(GeoInfo& info);
	GeometryID LoadFromGLTF(std::string&& path, std::string&& name);
	MeshGeometry& GetMeshGeomerty(std::string name);
	MeshGeometry& GetMeshGeomerty(GeometryID id);
	GeometryID GetMeshGeomertyID(std::string name);
};