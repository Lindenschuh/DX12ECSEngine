#pragma once
#include <stdint.h>
#include <stdio.h>
#include <d3d12.h>
#include "../Ext/d3dx12.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <wrl.h>
#include <directxcollision.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <vector>
#include <unordered_map>
#include <string.h>
#include <imgui.h>

#define MALLOC(t,n) ((t *) malloc((n)*sizeof(t)))
#define ALLOCA(t,n) ((t *) alloca((n)*sizeof(t)))
#define REALLOC(t,n,d) ((t *) realloc(d,(n)*sizeof(t)))
#define ARRAYCOUNT(Array) (sizeof(Array)/sizeof((Array)[0]))

#if defined(_DEBUG) || defined(DEBUG)
#define HR(x){	HRESULT hr = (x); if (FAILED(hr)) { fprintf(stderr, "Failed at %ld in file %s", __LINE__, __FILE__); DebugBreak();}}
#else
#define HR(x) (x);
#endif

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;
typedef int64_t		s64;

typedef float		f32;
typedef double		f64;

typedef u32 MaterialID;
typedef u32 TextureID;
typedef u32 GeometryID;
typedef u32 FrameResourceID;

using namespace DirectX;
using namespace Microsoft::WRL;

template<typename T>
T Clamp(const T& x, const T& low, const T& high)
{
	return x < low ? low : (x > high ? high : x);
}

static XMFLOAT4X4 Identity4x4()
{
	return XMFLOAT4X4
	{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };
};

static XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
{
	return DirectX::XMVectorSet(
		radius*sinf(phi)*cosf(theta),
		radius*cosf(phi),
		radius*sinf(phi)*sinf(theta),
		1.0f);
}

static float RandF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

static float RandF(float a, float b)
{
	return a + RandF()*(b - a);
}

static int Rand(int a, int b)
{
	return a + rand() % ((b - a) + 1);
}

static float RandomFloat01() { return (float)rand() / (float)RAND_MAX; }
static float RandomFloat(float from, float to) { return RandomFloat01() * (to - from) + from; }
const s32 gNumFrameResources = 3;