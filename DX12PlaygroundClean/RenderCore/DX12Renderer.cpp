#include <assert.h>
#include "DX12Renderer.h"

DX12Renderer::DX12Renderer(u32 width, u32 height, const char* windowName)
{
	mTimer = new GameTimer();
	mDXCon = new DX12Context(width, height, windowName);

	BuildRootSignature();
	mTextureSystem = new TextureSystem(mDXCon);
	mMaterialSystem = new MaterialSystem(mDXCon, mTextureSystem);
	mGeometrySystem = new GeometrySystem(mDXCon);
	mShaderSystem = new ShaderSystem(mDXCon);
	mPSOSystem = new PSOSystem(mDXCon, mShaderSystem, mRootSignature.Get());

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(mDXCon->Window->hwnd);
	ImGui_ImplDX12_Init(mDXCon->mD3dDevice.Get(), gNumFrameResources,
		mDXCon->mBackBufferFormat, mDXCon->mSRVHeap->GetCPUDescriptorHandleForHeapStart(),
		mDXCon->mSRVHeap->GetGPUDescriptorHandleForHeapStart());
}

void DX12Renderer::FinishSetup()
{
	mTextureSystem->UploadTextures();
	mFrameResourceSystem = new FrameResourceSystem(gNumFrameResources, mDXCon, 1,
		mRItems[Opaque].size(), mMaterialSystem->GetMaterialCount());

	HR(mDXCon->mCmdList->Close());
	ID3D12CommandList* cmdLists[] = { mDXCon->mCmdList.Get() };
	mDXCon->mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	mDXCon->flushCommandQueue();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f*XM_PI, (float)mDXCon->Window->width / mDXCon->Window->height, 1.0f, 1000.f);
	XMStoreFloat4x4(&mProj, p);
	Update();
}

void DX12Renderer::
SetMainCamera(XMFLOAT3 position, XMFLOAT4X4 view)
{
	mMainCam.EyePos = position;
	mMainCam.View = view;
}

RenderItem * DX12Renderer::GetRenderItem(std::string name)
{
	std::pair<RenderLayer, u32> renderPair = mRenderItemPair[name];
	return &mRItems[renderPair.first][renderPair.second];
}

void DX12Renderer::AddRenderItem(std::string name, RenderItem r, RenderLayer rl)
{
	mRItems[rl].push_back(r);
	mRenderItemPair[name] = std::pair<RenderLayer, u32>(rl, mRItems[rl].size() - 1);
}
bool DX12Renderer::IsWindowActive(void)
{
	MSG msg;

	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return false;
	}

	CalculateFrameStats();
	mTimer->Tick();
	ProcessGlobalEvents();
	return true;
}

void DX12Renderer::Draw()
{
	FrameResource& currentFrameResource = mFrameResourceSystem->GetCurrentFrameResource();
	ComPtr<ID3D12CommandAllocator> cmdListAlloc = currentFrameResource.CmdListAlloc;

	HR(cmdListAlloc->Reset());

	HR(mDXCon->mCmdList->Reset(cmdListAlloc.Get(), mPSOSystem->GetPSO("opaque").PSOData.Get()));

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

	auto passCB = currentFrameResource.PassCB->Resource();
	mDXCon->mCmdList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(mDXCon->mCmdList.Get(), mRItems[RenderLayer::Opaque]);

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

	currentFrameResource.Fence = ++mDXCon->mCurrentFence;

	mDXCon->mCmdQueue->Signal(mDXCon->mFence.Get(), mDXCon->mCurrentFence);
}

void DX12Renderer::Update()
{
	mFrameResourceSystem->SwitchFrameResource();
	FrameResource& currentFrameResource = mFrameResourceSystem->GetCurrentFrameResource();
	for (u32 i = 0; i < RenderLayer::Count; i++)
	{
		UpdateObjectPassCB(mRItems[i], currentFrameResource.ObjecCB);
	}

	mMaterialSystem->UpdateMaterials(currentFrameResource.MaterialCB);
	UpdateMainPassCB(mMainPassCB, currentFrameResource.PassCB, &mMainCam, mDXCon, mTimer, mProj);
}

void DX12Renderer::ProcessGlobalEvents()
{
	if (gGlobalEvents.isResized)
	{
		mDXCon->resize(gGlobalEvents.ResizeWidth, gGlobalEvents.ResizeHeight);

		gGlobalEvents.isResized = false;
		XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f*XM_PI, (float)mDXCon->Window->width / mDXCon->Window->height, 1.0f, 1000.f);
		XMStoreFloat4x4(&mProj, p);
	}
}

void DX12Renderer::BuildRootSignature()
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

void DX12Renderer::DrawRenderItems(ID3D12GraphicsCommandList * cmdList, std::vector<RenderItem>& rItems)
{
	FrameResource mCurrentFrameResource = mFrameResourceSystem->GetCurrentFrameResource();
	u32 objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));
	u32 matCBByteSize = CalcConstantBufferByteSize(sizeof(MaterialConstants));

	ID3D12Resource* objConst = mCurrentFrameResource.ObjecCB->Resource();
	ID3D12Resource* matConst = mCurrentFrameResource.MaterialCB->Resource();

	for (int i = 0; i < rItems.size(); i++)
	{
		RenderItem& ri = rItems[i];
		MeshGeometry& geo = mGeometrySystem->GetMeshGeomerty(ri.GeoIndex);
		cmdList->IASetVertexBuffers(0, 1, &geo.VertexBufferView());
		cmdList->IASetIndexBuffer(&geo.IndexBufferView());
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
D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::CurrentbackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mDXCon->mRTVHeap->GetCPUDescriptorHandleForHeapStart(),
		mDXCon->mCurrentBackbuffer,
		mDXCon->mRtvDescriptorSize
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::DepthStencilView()
{
	return mDXCon->mDSVHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* DX12Renderer::CurrentBackbuffer()
{
	return mDXCon->mSwapchainBuffer[mDXCon->mCurrentBackbuffer].Get();
}

void DX12Renderer::CalculateFrameStats()
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