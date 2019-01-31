#include "PSOSystem.h"

PSOSystem::PSOSystem(DX12Context* context, ShaderSystem* shaderSystem, ID3D12RootSignature* RootSignature)
{
	mDXContext = context;
	mShaderSystem = shaderSystem;
	mRootSignature = RootSignature;
}

void PSOSystem::BuildPSO(std::string name, std::string VSName, std::string PSName, PSOOptions & options,
	D3D12_BLEND_DESC * blendOptions,
	D3D12_DEPTH_STENCIL_DESC * stencilOption,
	D3D12_RASTERIZER_DESC * rasterizerOptions)
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

	if (blendOptions)
		psoDesc.BlendState = *blendOptions;

	if (stencilOption)
		psoDesc.DepthStencilState = *stencilOption;

	if (rasterizerOptions)
		psoDesc.RasterizerState = *rasterizerOptions;

	PSO pso = {};
	pso.options = options;
	pso.PSName = PSName;
	pso.VSName = VSName;

	HR(mDXContext->mD3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso.PSOData)));
	mPSOs[name] = pso;
}

void PSOSystem::BuildTransparentPSO(std::string name, std::string VSName, std::string PSName, PSOOptions & options)
{
	D3D12_BLEND_DESC desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	desc.RenderTarget[0] = transparencyBlendDesc;

	BuildPSO(name, VSName, PSName, options, &desc);
}

void PSOSystem::BuildStencilMirrowPSO(std::string name, std::string VSName, std::string PSName, PSOOptions & options)
{
	CD3DX12_BLEND_DESC mirrorBlendState(D3D12_DEFAULT);
	mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC mirrStencilDesc;
	mirrStencilDesc.DepthEnable = true;
	mirrStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrStencilDesc.StencilEnable = true;
	mirrStencilDesc.StencilReadMask = 0xff;
	mirrStencilDesc.StencilWriteMask = 0xff;

	mirrStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
	mirrStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	BuildPSO(name, VSName, PSName, options, &mirrorBlendState, &mirrStencilDesc);
}

void PSOSystem::BuildStencilReflectionsPSO(std::string name, std::string VSName, std::string PSName, PSOOptions & options)
{
	D3D12_DEPTH_STENCIL_DESC reflectionsStencilDesc;
	reflectionsStencilDesc.DepthEnable = true;
	reflectionsStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionsStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionsStencilDesc.StencilEnable = true;
	reflectionsStencilDesc.StencilReadMask = 0xff;
	reflectionsStencilDesc.StencilWriteMask = 0xff;

	reflectionsStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	reflectionsStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_RASTERIZER_DESC rastDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rastDesc.CullMode = D3D12_CULL_MODE_BACK;
	rastDesc.FrontCounterClockwise = true;

	BuildPSO(name, VSName, PSName, options, nullptr, &reflectionsStencilDesc, &rastDesc);
}

void PSOSystem::BuildShadowPSO(std::string name, std::string VSName, std::string PSName, PSOOptions & options)
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
	psoDesc.RasterizerState.DepthBias = 100000;
	psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = options.SampleMask;
	psoDesc.PrimitiveTopologyType = options.PrimitiveTopologyType;
	psoDesc.NumRenderTargets = 0;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = mDXContext->mDepthStencilFormat;

	PSO pso = {};
	pso.options = options;
	pso.PSName = PSName;
	pso.VSName = VSName;

	HR(mDXContext->mD3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso.PSOData)));
	mPSOs[name] = pso;
}

void PSOSystem::BuildSkyboxPSO(std::string name, std::string VSName, std::string PSName, PSOOptions & options)
{
	D3D12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	D3D12_DEPTH_STENCIL_DESC DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	BuildPSO(name, VSName, PSName, options, nullptr, &DepthStencilState, &RasterizerState);
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