#include "GeometrySystem.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "../Ext/tiny_gltf.h"

struct GeometrySystemResources
{
	ComPtr<ID3DBlob> CpuData;
	ComPtr<ID3D12Resource> GpuData;
	ComPtr<ID3D12Resource> UploadBuffer;
};

GeometrySystem::GeometrySystem(DX12Context* DXContext)
{
	mDXContext = DXContext;
}

GeometryID GeometrySystem::LoadGeometry(GeoInfo& info)
{
	MeshGeometry geo;
	GeometryID id = mAllGeometry.size();
	geo.GeometryIndex = id;
	const u32 vbByteSize = info.vertCount * sizeof(Vertex);
	const u32 ibByteSize = info.indiceCount * sizeof(u16);

	HR(D3DCreateBlob(vbByteSize, &geo.VertexBufferCPU));
	CopyMemory(geo.VertexBufferCPU->GetBufferPointer(), info.verts, vbByteSize);

	HR(D3DCreateBlob(ibByteSize, &geo.IndexBufferCPU));
	CopyMemory(geo.IndexBufferCPU->GetBufferPointer(), info.indicies, ibByteSize);

	geo.VertexBufferGPU = CreateDefaultBuffer(mDXContext->mD3dDevice.Get(), mDXContext->mCmdList.Get(),
		info.verts, vbByteSize, geo.VertexBufferUploader);

	geo.IndexBufferGPU = CreateDefaultBuffer(mDXContext->mD3dDevice.Get(), mDXContext->mCmdList.Get(),
		info.indicies, ibByteSize, geo.IndexBufferUploader);

	geo.VertexByteStride = sizeof(Vertex);
	geo.VertexBufferByteSize = vbByteSize;
	geo.IndexFormat = DXGI_FORMAT_R16_UINT;
	geo.IndexBufferByteSize = ibByteSize;

	for (int i = 0; i < info.submeshCount; i++)
	{
		geo.Submeshes[info.SubmeshNames[i]] = info.submeshs[i];
	}
	mAllGeometry.push_back(geo);
	mGeometryIDs[info.Name] = id;
	return id;
}

GeometryID GeometrySystem::LoadFromGLTF(std::string&& path, std::string&& name)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;

	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

	if (!warn.empty())
	{
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty())
	{
		printf("Err: %s\n", err.c_str());
	}

	if (!ret)
	{
		printf("Failed to parse glTF\n");
		return ret;
	}

	for (int i = 0; i < model.meshes.size(); i++)
	{
		u16* indicies;
		u32 numOfIndicies = 0;

		tinygltf::Mesh mesh = model.meshes[i];
		tinygltf::Primitive prim = mesh.primitives[0];

		tinygltf::Accessor indiAccesor = model.accessors[prim.indices];
		tinygltf::BufferView indiBView = model.bufferViews[indiAccesor.bufferView];
		tinygltf::Buffer indiBuffer = model.buffers[indiBView.buffer];
		indicies = (u16*)(&indiBuffer.data.data()[indiBView.byteOffset]);
		numOfIndicies = indiAccesor.count;

		tinygltf::Accessor posAccesor = model.accessors[prim.attributes["POSITION"]];
		tinygltf::BufferView posBView = model.bufferViews[posAccesor.bufferView];
		tinygltf::Buffer posBuffer = model.buffers[posBView.buffer];

		std::vector<Vertex> verticies(posAccesor.count);

		for (int j = 0; j < posAccesor.count; j++)
		{
			verticies[j].Pos = *(XMFLOAT3*)((posBuffer.data.data() + posBView.byteOffset +
				posAccesor.byteOffset + (j * sizeof(XMFLOAT3))));
		}

		Submesh sm;
		sm.BaseVertexLocation = 0;
		sm.Bounds;
		sm.IndexCount = numOfIndicies;
		sm.StartIndexLocation = 0;

		GeoInfo geo;
		geo.indicies = indicies;
		geo.indiceCount = numOfIndicies;
		geo.Name = name;
		geo.submeshCount = 1;
		geo.SubmeshNames = &name;
		geo.vertCount = verticies.size();
		geo.verts = verticies.data();
		geo.submeshs = &sm;

		LoadGeometry(geo);
		//TODO: für verschiedene Accesors tun ind Verticies bauen
	}
}

MeshGeometry & GeometrySystem::GetMeshGeomerty(std::string name)
{
	return mAllGeometry[mGeometryIDs[name]];
}

MeshGeometry & GeometrySystem::GetMeshGeomerty(GeometryID id)
{
	return mAllGeometry[id];
}

GeometryID GeometrySystem::GetMeshGeomertyID(std::string name)
{
	return mGeometryIDs[name];
}