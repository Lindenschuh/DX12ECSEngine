#include "PSOSystem.h"

PSOSystem::PSOSystem(DX12Context* context, ShaderSystem* shaderSystem, ID3D12RootSignature* RootSignature)
{
	mDXContext = context;
	mShaderSystem = shaderSystem;
	mRootSignature = RootSignature;
}

void PSOSystem::BuildPSO(std::string name, std::string VSName,
	std::string PSName, PSOOptions& options)
{
	Shader VS = mShaderSystem->GetShader(VSName);
	Shader PS = mShaderSystem->GetShader(PSName);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = options.Layout;
	psoDesc.pRootSignature = mRootSignature;
	psoDesc.VS =
	{
		VS.ShaderBlob->GetBufferPointer(),
		VS.ShaderBlob->GetBufferSize()
	};
	psoDesc.PS =
	{
		PS.ShaderBlob->GetBufferPointer(),
		PS.ShaderBlob->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = options.SampleMask;
	psoDesc.PrimitiveTopologyType = options.PrimitiveTopologyType;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mDXContext->mBackBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = mDXContext->mDepthStencilFormat;

	psoDesc.RasterizerState.CullMode = options.CullMode;

	PSO pso = {};
	pso.options = options;
	pso.PSName = PSName;
	pso.VSName = VSName;

	HR(mDXContext->mD3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso.PSOData)));
	mPSOs[name] = pso;
}

void PSOSystem::BuildBlendablePSO(std::string name, std::string VSName, std::string PSName, PSOOptions & options, PSOBlendOptions & blendOptions)
{
	Shader VS = mShaderSystem->GetShader(VSName);
	Shader PS = mShaderSystem->GetShader(PSName);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = options.Layout;
	psoDesc.pRootSignature = mRootSignature;
	psoDesc.VS =
	{
		VS.ShaderBlob->GetBufferPointer(),
		VS.ShaderBlob->GetBufferSize()
	};
	psoDesc.PS =
	{
		PS.ShaderBlob->GetBufferPointer(),
		PS.ShaderBlob->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	D3D12_RENDER_TARGET_BLEND_DESC blendDesc;
	blendDesc.BlendEnable = true;
	blendDesc.LogicOpEnable = false;
	blendDesc.SrcBlend = blendOptions.SrcBlend;
	blendDesc.DestBlend = blendOptions.DestBlend;
	blendDesc.BlendOp = blendOptions.BlendOp;
	blendDesc.SrcBlendAlpha = blendOptions.SrcBlendAlpha;
	blendDesc.DestBlendAlpha = blendOptions.DestBlendAlpha;
	blendDesc.BlendOpAlpha = blendOptions.BlendOpAlpha;
	blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState.RenderTarget[0] = blendDesc;

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = options.SampleMask;
	psoDesc.PrimitiveTopologyType = options.PrimitiveTopologyType;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mDXContext->mBackBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = mDXContext->mDepthStencilFormat;
	psoDesc.RasterizerState.CullMode = options.CullMode;

	PSO pso = {};
	pso.options = options;
	pso.PSName = PSName;
	pso.VSName = VSName;

	HR(mDXContext->mD3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso.PSOData)));
	mPSOs[name] = pso;
}

void PSOSystem::ReloadPSO(std::string name)
{
	PSO& pso = mPSOs[name];
	pso.PSOData->Release();
	mShaderSystem->ReloadShader(pso.VSName);
	mShaderSystem->ReloadShader(pso.PSName);
	BuildPSO(name, pso.VSName, pso.PSName, pso.options);
}

PSO& PSOSystem::GetPSO(std::string name)
{
	return mPSOs[name];
}

PSOSystem::~PSOSystem()
{
}