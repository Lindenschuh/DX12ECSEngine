#include "GeometryGenerator.h"

GeometryGenerator::GeometryGenerator()
{
}

MeshData GeometryGenerator::CreateBox(float width, float height, float depth, u32 numSubDivisons)
{
	MeshData meshData;

	VertexComplex v[24];

	float wHalf = 0.5f*width;
	float hHalf = 0.5f*height;
	float dHalf = 0.5f*depth;

	//fill front face
	v[0] = VertexComplex{ {-wHalf, -hHalf, -dHalf}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f} };
	v[1] = VertexComplex{ {-wHalf, +hHalf, -dHalf}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f} };
	v[2] = VertexComplex{ {+wHalf, +hHalf, -dHalf}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f} };
	v[3] = VertexComplex{ {+wHalf, -hHalf, -dHalf}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f} };

	//fill backface
	v[4] = VertexComplex{ {-wHalf, -hHalf, +dHalf}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f} };
	v[5] = VertexComplex{ {+wHalf, -hHalf, +dHalf}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f} };
	v[6] = VertexComplex{ {+wHalf, +hHalf, +dHalf}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f} };
	v[7] = VertexComplex{ {-wHalf, +hHalf, +dHalf}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f} };

	v[8] = VertexComplex{ {-wHalf, +hHalf, -dHalf}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f} };
	v[9] = VertexComplex{ {-wHalf, +hHalf, +dHalf}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f} };
	v[10] = VertexComplex{ {+wHalf, +hHalf, +dHalf}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f} };
	v[11] = VertexComplex{ {+wHalf, +hHalf, -dHalf}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f} };

	v[12] = VertexComplex{ {-wHalf, -hHalf, -dHalf}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f} };
	v[13] = VertexComplex{ {+wHalf, -hHalf, -dHalf}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f} };
	v[14] = VertexComplex{ {+wHalf, -hHalf, +dHalf}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f} };
	v[15] = VertexComplex{ {-wHalf, -hHalf, +dHalf}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f} };

	v[16] = VertexComplex{ {-wHalf, -hHalf, +dHalf}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, { 0.0f, 1.0f} };
	v[17] = VertexComplex{ {-wHalf, +hHalf, +dHalf}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, { 0.0f, 0.0f} };
	v[18] = VertexComplex{ {-wHalf, +hHalf, -dHalf}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, { 1.0f, 0.0f} };
	v[19] = VertexComplex{ {-wHalf, -hHalf, -dHalf}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, { 1.0f, 1.0f} };

	v[20] = VertexComplex{ {+wHalf, -hHalf, -dHalf}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f} };
	v[21] = VertexComplex{ {+wHalf, +hHalf, -dHalf}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f} };
	v[22] = VertexComplex{ {+wHalf, +hHalf, +dHalf}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f} };
	v[23] = VertexComplex{ {+wHalf, -hHalf, +dHalf}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f} };

	meshData.Vertices.assign(&v[0], &v[24]);

	u16 i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indicies.assign(&i[0], &i[36]);

	numSubDivisons = (numSubDivisons > 6 ? 6 : numSubDivisons);

	for (int i = 0; i < numSubDivisons; i++)
		Subdivide(meshData);

	return meshData;
}

MeshData GeometryGenerator::CreateSphere(float radius, u32 sliceCount, u32 stackCount)
{
	MeshData meshData;

	VertexComplex topVertex = { {0.0f, +radius, 0.0f},{0.0f, +1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f} };
	VertexComplex bottomVertex = { {0.0f, -radius, 0.0f},{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f} };

	meshData.Vertices.push_back(topVertex);

	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f * XM_PI / sliceCount;

	for (u32 i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;

		for (u32 j = 0; j <= sliceCount; j++)
		{
			float theta = j * thetaStep;

			VertexComplex v;

			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);

			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR P = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, XMVector3Normalize(P));

			v.TexCoord.x = theta / XM_2PI;
			v.TexCoord.y = phi / XM_PI;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomVertex);

	for (u32 i = 1; i <= sliceCount; i++)
	{
		meshData.Indicies.push_back(0);
		meshData.Indicies.push_back(i + 1);
		meshData.Indicies.push_back(i);
	}

	u32 baseIndex = 1;
	u32 ringVertexCount = sliceCount + 1;

	for (u32 i = 0; i < stackCount - 2; i++)
	{
		for (u32 j = 0; j < sliceCount; j++)
		{
			meshData.Indicies.push_back(baseIndex + i * ringVertexCount + j);
			meshData.Indicies.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indicies.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.Indicies.push_back(baseIndex + (i + 1)* ringVertexCount + j);
			meshData.Indicies.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indicies.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	u32 southPoleIndex = (u32)meshData.Vertices.size() - 1;

	baseIndex = southPoleIndex - ringVertexCount;

	for (u32 i = 0; i < sliceCount; i++)
	{
		meshData.Indicies.push_back(southPoleIndex);
		meshData.Indicies.push_back(baseIndex + i);
		meshData.Indicies.push_back(baseIndex + i + 1);
	}

	return meshData;
}

MeshData GeometryGenerator::CreateGeospehere(float radius, u32 numSubdivisons)
{
	MeshData meshData;

	numSubdivisons = numSubdivisons > 6 ? 6 : numSubdivisons;

	const float X = 0.525731f;
	const float Z = 0.850651f;

	XMFLOAT3 pos[12] =
	{
		XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
		XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
		XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
		XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
		XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
	};

	u16 k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	meshData.Vertices.resize(12);
	meshData.Indicies.assign(&k[0], &k[60]);

	for (u32 i = 0; i < 12; i++)
		meshData.Vertices[i].Position = pos[i];

	for (u32 i = 0; i < numSubdivisons; i++)
		Subdivide(meshData);

	for (u32 i = 0; i < meshData.Vertices.size(); i++)
	{
		//
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.Vertices[i].Position));

		XMVECTOR p = radius * n;

		XMStoreFloat3(&meshData.Vertices[i].Position, p);
		XMStoreFloat3(&meshData.Vertices[i].Normal, n);

		float theta = atan2f(meshData.Vertices[i].Position.z, meshData.Vertices[i].Position.x);

		if (theta < 0.0f)
			theta += XM_2PI;

		float phi = acosf(meshData.Vertices[i].Position.y / radius);

		meshData.Vertices[i].TexCoord.x = theta / XM_2PI;
		meshData.Vertices[i].TexCoord.y = phi / XM_PI;

		meshData.Vertices[i].TangentU.x = -radius * sinf(phi) * sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;
		meshData.Vertices[i].TangentU.z = +radius * sinf(phi) * cosf(theta);

		XMVECTOR T = XMLoadFloat3(&meshData.Vertices[i].TangentU);
		XMStoreFloat3(&meshData.Vertices[i].TangentU, XMVector3Normalize(T));
	}

	return meshData;
}

MeshData GeometryGenerator::CreateCylider(float bottomRadius, float topRadius, float height, u32 sliceCount, u32 stackCount)
{
	MeshData meshData;

	float stackHeight = height / stackCount;

	float radiusStep = (topRadius - bottomRadius) / stackCount;

	u32 ringCount = stackCount + 1;

	for (u32 i = 0; i < ringCount; i++)
	{
		float y = -0.5f*height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		float dTheta = 2.0f*XM_PI / sliceCount;
		for (u32 j = 0; j <= sliceCount; j++)
		{
			VertexComplex v;

			float c = cosf(j*dTheta);
			float s = sinf(j*dTheta);

			v.Position = { r*c, y, r*s };

			v.TexCoord.x = (float)j / sliceCount;
			v.TexCoord.y = 1.0f - (float)i / stackCount;

			v.TangentU = { -s, 0.0f, c };

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent = { dr*c, -height, dr*s };
			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&v.Normal, N);

			meshData.Vertices.push_back(v);
		}
	}

	u32 ringVertexCount = sliceCount + 1;

	for (u32 i = 0; i < stackCount; i++)
	{
		for (u32 j = 0; j < sliceCount; j++)
		{
			meshData.Indicies.push_back(i * ringVertexCount + j);
			meshData.Indicies.push_back((i + 1)* ringVertexCount + j);
			meshData.Indicies.push_back((i + 1)* ringVertexCount + j + 1);

			meshData.Indicies.push_back(i * ringVertexCount + j);
			meshData.Indicies.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.Indicies.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCyliderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCyliderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

	return meshData;
}

MeshData GeometryGenerator::CreateGrid(float width, float depth, u32 m, u32 n)
{
	MeshData meshData;

	u32 vertexCount = m * n;
	u32 faceCount = (m - 1) * (n - 1) * 2;

	float halfWidth = 0.5f*width;
	float halfDepth = 0.5f*depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.Vertices.resize(vertexCount);

	for (u32 i = 0; i < m; i++)
	{
		float z = halfDepth - i * dz;
		for (u32 j = 0; j < n; j++)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i*n + j].Position = { x, 0.0f, z };
			meshData.Vertices[i*n + j].Normal = { 0.0f, 1.0f, 0.0f };
			meshData.Vertices[i*n + j].TangentU = { 1.0f, 0.0f, 0.0f };

			meshData.Vertices[i*n + j].TexCoord.x = j * du;
			meshData.Vertices[i*n + j].TexCoord.y = i * dv;
		}
	}

	meshData.Indicies.resize(faceCount * 3);
	u32 k = 0;
	for (u32 i = 0; i < m - 1; i++)
	{
		for (u32 j = 0; j < n - 1; j++)
		{
			meshData.Indicies[k] = i * n + j;
			meshData.Indicies[k + 1] = i * n + j + 1;
			meshData.Indicies[k + 2] = (i + 1) * n + j;

			meshData.Indicies[k + 3] = (i + 1)*n + j;
			meshData.Indicies[k + 4] = i * n + j + 1;
			meshData.Indicies[k + 5] = (i + 1)* n + j + 1;

			k += 6;
		}
	}

	return meshData;
}

MeshData GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
	MeshData meshData;

	meshData.Vertices.resize(4);
	meshData.Indicies.resize(6);

	meshData.Vertices[0] =
	{
		{x, y - h, depth},
		{0.0f, 0.0f,-1.0f},
		{1.0f,0.0f, -1.0f},
		{0.0f, 1.0f}
	};

	meshData.Vertices[1] =
	{
		{x, y, depth},
		{0.0f, 0.0f,-1.0f},
		{1.0f,0.0f, 0.0f},
		{0.0f, 0.0f}
	};

	meshData.Vertices[2] =
	{
		{x + w, y, depth},
		{0.0f, 0.0f,-1.0f},
		{1.0f,0.0f, 0.0f},
		{1.0f, 0.0f}
	};

	meshData.Vertices[3] =
	{
		{x + w, y - h, depth},
		{0.0f, 0.0f,-1.0f},
		{1.0f,0.0f, 0.0f},
		{1.0f, 1.0f}
	};

	meshData.Indicies[0] = 0;
	meshData.Indicies[1] = 1;
	meshData.Indicies[2] = 2;

	meshData.Indicies[3] = 0;
	meshData.Indicies[4] = 2;
	meshData.Indicies[5] = 3;

	return meshData;
}

GeometryGenerator::~GeometryGenerator()
{
}

void GeometryGenerator::Subdivide(MeshData& meshData)
{
	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	MeshData inputCopy = meshData;

	meshData.Vertices.clear();
	meshData.Indicies.clear();

	u32 numTris = (u32)inputCopy.Indicies.size() / 3;
	for (u32 i = 0; i < numTris; i++)
	{
		VertexComplex v0 = inputCopy.Vertices[(u32)inputCopy.Indicies[i * 3 + 0]];
		VertexComplex v1 = inputCopy.Vertices[(u32)inputCopy.Indicies[i * 3 + 1]];
		VertexComplex v2 = inputCopy.Vertices[(u32)inputCopy.Indicies[i * 3 + 2]];

		VertexComplex m0 = MidPoint(v0, v1);
		VertexComplex m1 = MidPoint(v1, v2);
		VertexComplex m2 = MidPoint(v0, v2);

		meshData.Vertices.push_back(v0);
		meshData.Vertices.push_back(v1);
		meshData.Vertices.push_back(v2);

		meshData.Vertices.push_back(m0);
		meshData.Vertices.push_back(m1);
		meshData.Vertices.push_back(m2);

		meshData.Indicies.push_back(i * 6 + 0);
		meshData.Indicies.push_back(i * 6 + 3);
		meshData.Indicies.push_back(i * 6 + 5);

		meshData.Indicies.push_back(i * 6 + 3);
		meshData.Indicies.push_back(i * 6 + 4);
		meshData.Indicies.push_back(i * 6 + 5);

		meshData.Indicies.push_back(i * 6 + 5);
		meshData.Indicies.push_back(i * 6 + 4);
		meshData.Indicies.push_back(i * 6 + 2);

		meshData.Indicies.push_back(i * 6 + 3);
		meshData.Indicies.push_back(i * 6 + 1);
		meshData.Indicies.push_back(i * 6 + 4);
	}
}

VertexComplex GeometryGenerator::MidPoint(const VertexComplex & v0, const VertexComplex & v1)
{
	XMVECTOR p0 = XMLoadFloat3(&v0.Position);
	XMVECTOR p1 = XMLoadFloat3(&v1.Position);;

	XMVECTOR n0 = XMLoadFloat3(&v0.Normal);
	XMVECTOR n1 = XMLoadFloat3(&v1.Normal);

	XMVECTOR tan0 = XMLoadFloat3(&v0.TangentU);
	XMVECTOR tan1 = XMLoadFloat3(&v1.TangentU);

	XMVECTOR tex0 = XMLoadFloat2(&v0.TexCoord);
	XMVECTOR tex1 = XMLoadFloat2(&v1.TexCoord);

	XMVECTOR pos = 0.5f*(p0 + p1);
	XMVECTOR normal = XMVector3Normalize(0.5f*(n0 + n1));
	XMVECTOR tagent = XMVector3Normalize(0.5f*(tan0 + tan1));
	XMVECTOR tex = 0.5f*(tex0 + tex1);

	VertexComplex v;
	XMStoreFloat3(&v.Position, pos);
	XMStoreFloat3(&v.Normal, normal);
	XMStoreFloat3(&v.TangentU, tagent);
	XMStoreFloat2(&v.TexCoord, tex);

	return v;
}

void GeometryGenerator::BuildCyliderTopCap(float bottomRadius, float topRadius,
	float height, u32 sliceCount, u32 stackCount, MeshData & meshData)
{
	u32 baseIndex = (u32)meshData.Vertices.size();

	float y = 0.5f*height;
	float dTheta = 2.0f* XM_PI / sliceCount;

	for (u32 i = 0; i <= sliceCount; i++)
	{
		float x = topRadius * cosf(i*dTheta);
		float z = topRadius * sinf(i*dTheta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back({ {x, y, z}, {0.0f, 1.0f,0.0f},{1.0f,0.0f,0.0f},{u,v} });
	}

	meshData.Vertices.push_back({ { 0.0f, y, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.5f, 0.5f } });

	u32 centerIndex = (u32)meshData.Vertices.size() - 1;

	for (u32 i = 0; i < sliceCount; i++)
	{
		meshData.Indicies.push_back(centerIndex);
		meshData.Indicies.push_back(baseIndex + i + 1);
		meshData.Indicies.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCyliderBottomCap(float bottomRadius, float topRadius,
	float height, u32 sliceCount, u32 stackCount, MeshData & meshData)
{
	u32 baseIndex = (u32)meshData.Vertices.size();

	float y = -0.5f*height;

	float dTheta = 2.0f* XM_PI / sliceCount;
	for (u32 i = 0; i <= sliceCount; i++)
	{
		float x = bottomRadius * cosf(i*dTheta);
		float z = bottomRadius * sinf(i*dTheta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back({ {x, y, z}, {0.0f, -1.0f, 0.0f},{1.0f,0.0f,0.0f},{u,v} });
	}

	meshData.Vertices.push_back({ { 0.0f, y, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.5f, 0.5f } });

	u32 centerIndex = (u32)meshData.Vertices.size() - 1;

	for (u32 i = 0; i < sliceCount; i++)
	{
		meshData.Indicies.push_back(centerIndex);
		meshData.Indicies.push_back(baseIndex + i);
		meshData.Indicies.push_back(baseIndex + i + 1);
	}
}