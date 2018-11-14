#include "GeometrySystem.h"

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