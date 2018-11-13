#include <assert.h>
#include "InitDX.h"

DX12Render::DX12Render(DX12Context* context, Waves* wave)
{
	mTimer = new GameTimer();
	mDXCon = context;
	gWaves = wave;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(mDXCon->Window->hwnd);
	ImGui_ImplDX12_Init(mDXCon->mD3dDevice.Get(), gNumFrameResources,
		mDXCon->mBackBufferFormat, mDXCon->mSRVHeap->GetCPUDescriptorHandleForHeapStart(),
		mDXCon->mSRVHeap->GetGPUDescriptorHandleForHeapStart());

	buildRootSignature();
	buildShaders();
	buildPSO();
}

bool DX12Render::isWindowActive(void)
{
	MSG msg;

	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return false;
	}

	calculateFrameStats();
	mTimer->Tick();
	processGlobalEvents();
	return true;
}

void DX12Render::Draw()
{
	ComPtr<ID3D12CommandAllocator> cmdListAlloc = mCurrentFrameResource->CmdListAlloc;

	HR(cmdListAlloc->Reset());

	HR(mDXCon->mCmdList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	mDXCon->mCmdList->RSSetViewports(1, &mDXCon->mViewPort);
	mDXCon->mCmdList->RSSetScissorRects(1, &mDXCon->mScissorRect);

	mDXCon->mCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackbuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mDXCon->mCmdList->ClearRenderTargetView(CurrentbackBufferView(), Colors::LightCyan, 0, nullptr);
	mDXCon->mCmdList->ClearDepthStencilView(DepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mDXCon->mCmdList->OMSetRenderTargets(1, &CurrentbackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mDXCon->mTextureHeap.Get() };
	mDXCon->mCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mDXCon->mCmdList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrentFrameResource->PassCB->Resource();
	mDXCon->mCmdList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	drawRenderItems(mDXCon->mCmdList.Get(), mRItems[RenderLayer::Opaque]);

	mDXCon->mCmdList->SetDescriptorHeaps(1, mDXCon->mSRVHeap.GetAddressOf());
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mDXCon->mCmdList.Get());

	mDXCon->mCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackbuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	HR(mDXCon->mCmdList->Close());

	ID3D12CommandList* cmdLists[] = { mDXCon->mCmdList.Get() };
	mDXCon->mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	HR(mDXCon->mSwapchain->Present(0, 0));
	mDXCon->mCurrentBackbuffer = (mDXCon->mCurrentBackbuffer + 1) % mSwapchainBufferCount;

	mCurrentFrameResource->Fence = ++mDXCon->mCurrentFence;

	mDXCon->mCmdQueue->Signal(mDXCon->mFence.Get(), mDXCon->mCurrentFence);
}

void DX12Render::Update()
{
	mCurrentFrameResourceIndex = (mCurrentFrameResourceIndex + 1) % gNumFrameResources;
	mCurrentFrameResource = &mFrameResources[mCurrentFrameResourceIndex];

	if (mCurrentFrameResource->Fence != 0 && mDXCon->mFence->GetCompletedValue() < mCurrentFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		HR(mDXCon->mFence->SetEventOnCompletion(mCurrentFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	for (u32 i = 0; i < RenderLayer::Count; i++)
	{
		UpdateObjectPassCB(mRItems[i], mCurrentFrameResource->ObjecCB);
	}

	UpdateMaterialCBs(mAllMaterials, mCurrentFrameResource->MaterialCB);
	UpdateMainPassCB(mMainPassCB, mCurrentFrameResource->PassCB, &mMainCam, mDXCon, mTimer, mProj);
}

void DX12Render::AddMaterial(std::string name, Material m)
{
	mAllMaterials.push_back(m);
	mMaterialsIndex[name] = mAllMaterials.size() - 1;
}

void DX12Render::AddGeometry(std::string name, MeshGeometry g)
{
	g.GeometryIndex = mAllGeometry.size();
	mAllGeometry.push_back(g);
	mGeometeriesIndex[name] = mAllGeometry.size() - 1;
}

void DX12Render::AddTexture(std::string name, Texture t)
{
	mAllTextures.push_back(t);
	mTextureIndex[name] = mAllTextures.size() - 1;
}
void DX12Render::AddRenderItem(std::string name, RenderItem r, RenderLayer rl)
{
	mRItems[rl].push_back(r);
	mRenderItemPair[name] = std::pair<RenderLayer, u32>(rl, mRItems[rl].size() - 1);
}
void DX12Render::
SetMainCamera(XMFLOAT3 position, XMFLOAT4X4 view)
{
	mMainCam.EyePos = position;
	mMainCam.View = view;
}
MeshGeometry* DX12Render::GetGeometry(std::string name)
{
	return &mAllGeometry[mGeometeriesIndex[name]];
}

Material* DX12Render::GetMaterial(std::string name)
{
	return &mAllMaterials[mMaterialsIndex[name]];
}

Texture * DX12Render::GetTexture(std::string name)
{
	return &mAllTextures[mTextureIndex[name]];
}

RenderItem * DX12Render::GetRenderItem(std::string name)
{
	std::pair<RenderLayer, u32> renderPair = mRenderItemPair[name];
	return &mRItems[renderPair.first][renderPair.second];
}

void DX12Render::FinishSetup()
{
	buildDescriptorHeaps();
	buildFrameResources();

	HR(mDXCon->mCmdList->Close());
	ID3D12CommandList* cmdLists[] = { mDXCon->mCmdList.Get() };
	mDXCon->mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	mDXCon->flushCommandQueue();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f*XM_PI, (float)mDXCon->Window->width / mDXCon->Window->height, 1.0f, 1000.f);
	XMStoreFloat4x4(&mProj, p);
	Update();
}

void DX12Render::processGlobalEvents()
{
	if (gGlobalEvents.isResized)
	{
		mDXCon->resize(gGlobalEvents.ResizeWidth, gGlobalEvents.ResizeHeight);

		gGlobalEvents.isResized = false;
		XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f*XM_PI, (float)mDXCon->Window->width / mDXCon->Window->height, 1.0f, 1000.f);
		XMStoreFloat4x4(&mProj, p);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Render::CurrentbackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mDXCon->mRTVHeap->GetCPUDescriptorHandleForHeapStart(),
		mDXCon->mCurrentBackbuffer,
		mDXCon->mRtvDescriptorSize
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Render::DepthStencilView()
{
	return mDXCon->mDSVHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* DX12Render::CurrentBackbuffer()
{
	return mDXCon->mSwapchainBuffer[mDXCon->mCurrentBackbuffer].Get();
}

void DX12Render::calculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((mTimer->GetGameTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		char windowText[50];

		sprintf_s(windowText, "fps: %4.1f, mspf %4.2f", fps, mspf);

		SetWindowText(mDXCon->Window->hwnd, windowText);
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void DX12Render::buildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSmaplers = mDXCon->GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		staticSmaplers.size(), staticSmaplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HR(D3D12SerializeRootSignature(&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(),
		errorBlob.GetAddressOf()));

	if (errorBlob != nullptr)
		fprintf(stdout, (char*)errorBlob->GetBufferPointer());

	HR(mDXCon->mD3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())
	));
}

void DX12Render::buildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.NumDescriptors = mAllTextures.size();
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HR(mDXCon->mD3dDevice->CreateDescriptorHeap(&texHeapDesc, IID_PPV_ARGS(&mDXCon->mTextureHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mDXCon->mTextureHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC texSRVDesc = {};
	texSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	texSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	texSRVDesc.Texture2D.MostDetailedMip = 0;
	texSRVDesc.Texture2D.MipLevels = -1;

	for (int i = 0; i < mAllTextures.size(); i++)
	{
		texSRVDesc.Format = mAllTextures[i].Resource->GetDesc().Format;
		mDXCon->mD3dDevice->CreateShaderResourceView(mAllTextures[i].Resource.Get(), &texSRVDesc, hDescriptor);
		hDescriptor.Offset(1, mDXCon->mCbvSrvUavDescriptorSize);
	}
}

void DX12Render::buildShaders()
{
	mShaders["standardVS"] = CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["opaquePS"] = CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	gInputLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	gInputLayout[1] = { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };
	gInputLayout[2] = { "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };
}

void DX12Render::buildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc = {};
	opaquePsoDesc.InputLayout = { gInputLayout, 3 };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		mShaders["standardVS"]->GetBufferPointer(),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		mShaders["opaquePS"]->GetBufferPointer(),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mDXCon->mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = 1;
	opaquePsoDesc.SampleDesc.Quality = 0;
	opaquePsoDesc.DSVFormat = mDXCon->mDepthStencilFormat;

	ComPtr<ID3D12PipelineState> opPSO;
	HR(mDXCon->mD3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&opPSO)));
	mPSOs["opaque"] = opPSO;

	ComPtr<ID3D12PipelineState> wirePSO;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireFramePSoDesc = opaquePsoDesc;
	opaqueWireFramePSoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	HR(mDXCon->mD3dDevice->CreateGraphicsPipelineState(&opaqueWireFramePSoDesc,
		IID_PPV_ARGS(&wirePSO)));

	mPSOs["opaque_wireframe"] = wirePSO;
}

void DX12Render::buildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; i++)
	{
		mFrameResources.push_back(FrameResource(mDXCon->mD3dDevice.Get(), 1, mRItems[RenderLayer::Opaque].size(),
			mAllMaterials.size(), gWaves->VertexCount()));//add transparent later
	}
}

void DX12Render::drawRenderItems(ID3D12GraphicsCommandList * cmdList, std::vector<RenderItem>& rItems)
{
	u32 objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));
	u32 matCBByteSize = CalcConstantBufferByteSize(sizeof(MaterialConstants));

	ID3D12Resource* objConst = mCurrentFrameResource->ObjecCB->Resource();
	ID3D12Resource* matConst = mCurrentFrameResource->MaterialCB->Resource();

	for (int i = 0; i < rItems.size(); i++)
	{
		RenderItem ri = rItems[i];

		cmdList->IASetVertexBuffers(0, 1, &mAllGeometry[ri.GeoIndex].VertexBufferView());
		cmdList->IASetIndexBuffer(&mAllGeometry[ri.GeoIndex].IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri.PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mDXCon->mTextureHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri.texHeapIndex, mDXCon->mCbvSrvUavDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objConst->GetGPUVirtualAddress() + ri.ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matConst->GetGPUVirtualAddress() + ri.MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.baseVertexLocation, 0);
	}
}