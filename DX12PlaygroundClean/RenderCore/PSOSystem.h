#pragma once
#include "../Core/Default.h"
#include "DX12Context.h"
#include "ShaderSystem.h"

struct PSOBlendOptions
{
	D3D12_BLEND SrcBlend;
	D3D12_BLEND DestBlend;
	D3D12_BLEND_OP BlendOp;
	D3D12_BLEND SrcBlendAlpha;
	D3D12_BLEND DestBlendAlpha;
	D3D12_BLEND_OP BlendOpAlpha;
};
static
PSOBlendOptions
DefaultPSOBlendOptions()
{
	return
	{
		D3D12_BLEND_SRC_ALPHA,
		D3D12_BLEND_INV_SRC_ALPHA,
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD
	};
}
struct PSOOptions
{
	D3D12_INPUT_LAYOUT_DESC Layout;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
	D3D12_CULL_MODE CullMode;
	u32 SampleMask;
};

static D3D12_INPUT_ELEMENT_DESC gInputLayout[3]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 }
};
static PSOOptions
DefaultPSOOptions()
{
	return
	{
		{gInputLayout,3 },
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		D3D12_CULL_MODE_BACK,
		UINT_MAX
	};
}

struct PSO
{
	std::string VSName;
	std::string PSName;
	PSOOptions options;
	ComPtr<ID3D12PipelineState> PSOData;
};

class PSOSystem
{
private:
	DX12Context* mDXContext;
	ShaderSystem* mShaderSystem;
	ID3D12RootSignature* mRootSignature;
	std::unordered_map<std::string, PSO> mPSOs;
public:
	PSOSystem(DX12Context* Context, ShaderSystem* shaderSystem, ID3D12RootSignature* RootSignature);
	void BuildPSO(std::string name, std::string VSName, std::string PSName, PSOOptions& options);
	void BuildBlendablePSO(std::string name, std::string VSName, std::string PSName, PSOOptions& options, PSOBlendOptions& blendOptions);
	void ReloadPSO(std::string name);
	PSO& GetPSO(std::string name);
	~PSOSystem();
};
