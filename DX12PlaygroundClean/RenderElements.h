#pragma once
#include "Default.h"
#include "DX12Context.h"
enum RenderLayer
{
	Opaque = 0,
	Count
};

struct RenderItem
{
	XMFLOAT4X4 WorldPos = Identity4x4();
	XMFLOAT4X4 TextureTransform = Identity4x4();
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	s32 NumFramesDirty = gNumFrameResources;
	u32 ObjCBIndex = -1;
	u32 MatCBIndex = -1;
	u32 texHeapIndex = -1;
	u32 GeoIndex = -1;

	//IndexParameters
	u32 IndexCount = 0;
	u32 StartIndexLocation = 0;
	s32 baseVertexLocation = 0;
};

static Waves * gWaves = new Waves(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);
extern RenderItem* gWavesRItem;
