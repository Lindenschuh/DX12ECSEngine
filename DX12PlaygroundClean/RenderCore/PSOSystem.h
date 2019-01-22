#pragma once
#include "../Core/Default.h"
#include "DX12Context.h"
#include "ShaderSystem.h"

struct PSOOptions
{
	D3D12_INPUT_LAYOUT_DESC Layout;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
	u32 SampleMask;
};

static D3D12_INPUT_ELEMENT_DESC gInputLayout[4]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,32,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
};
static PSOOptions
DefaultPSOOptions()
{
	return
	{
		{gInputLayout,4 },
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
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
	void BuildPSO(std::string name, std::string VSName, std::string PSName, PSOOptions& options,
		D3D12_BLEND_DESC* blendOptions = nullptr,
		D3D12_DEPTH_STENCIL_DESC* stencilOption = nullptr,
		D3D12_RASTERIZER_DESC* rasterizerOptions = nullptr);

	void BuildTransparentPSO(std::string name, std::string VSName,
		std::string PSName, PSOOptions& options);

	void BuildStencilMirrowPSO(std::string name, std::string VSName,
		std::string PSName, PSOOptions& options);

	void BuildStencilReflectionsPSO(std::string name, std::string VSName,
		std::string PSName, PSOOptions& options);

	void BuildShadowPSO(std::string name, std::string VSName,
		std::string PSName, PSOOptions& options);

	void BuildSkyboxPSO(std::string name, std::string VSName,
		std::string PSName, PSOOptions& options);

	void ReloadPSO(std::string name);
	PSO& GetPSO(std::string name);
	~PSOSystem();
};
