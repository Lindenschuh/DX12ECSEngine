#pragma once
#include "Default.h"

struct VertexComplex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT3 TagentU;
	XMFLOAT2 TexCoord;
};

struct MeshData
{
	std::vector<u16> Indicies;
	std::vector<VertexComplex> Vertices;
};

class GeometryGenerator
{
public:
	GeometryGenerator();

	MeshData CreateBox(float width, float height, float depth, u32 numSubDivisons);

	MeshData CreateSphere(float radius, u32 sliceCount, u32 stackCount);

	MeshData CreateGeospehere(float radius, u32 numSubdivisons);

	MeshData CreateCylider(float bottomRadius, float topRadius, float height, u32 sliceCount, u32 stackCount);

	MeshData CreateGrid(float width, float depth, u32 m, u32 n);

	MeshData CreateQuad(float x, float y, float w, float h, float depth);

	~GeometryGenerator();

private:
	void Subdivide(MeshData& meshData);
	VertexComplex MidPoint(const VertexComplex& v0, const VertexComplex& v1);
	void BuildCyliderTopCap(float bottomRadius, float topRadius, float height, u32 sliceCount, u32 stackCount, MeshData& meshData);
	void BuildCyliderBottomCap(float bottomRadius, float topRadius, float height, u32 sliceCount, u32 stackCount, MeshData& meshData);
};
