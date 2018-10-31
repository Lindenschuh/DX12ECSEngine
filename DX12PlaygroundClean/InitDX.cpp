#include <assert.h>
#include "InitDX.h"
#include "Ext/imgui_impl_win32.h"
#include "Ext/imgui_impl_dx12.h"
RenderItem* gWavesRItem = nullptr;

static void UpdateWaves(GameTimer* mTimer, UploadBuffer<Vertex1>* currWavesVB, DX12Render& render)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if ((mTimer->GetGameTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = Rand(4, gWaves->RowCount() - 5);
		int j = Rand(4, gWaves->ColumnCount() - 5);

		float r = RandF(0.2f, 0.5f);

		gWaves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	gWaves->Update(mTimer->mDeltaTime);

	// Update the wave vertex buffer with the new solution.
	for (int i = 0; i < gWaves->VertexCount(); ++i)
	{
		Vertex1 v;

		v.Pos = gWaves->mCurrSolution[i];
		v.Normal = gWaves->mNormals[i];

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave render item to the current frame VB.
	render.GetGeometry("waterGeo")->VertexBufferGPU = currWavesVB->Resource();
}

DX12Render::DX12Render(DX12Context* context)
{
	mTimer = new GameTimer();
	mDXCon = context;

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
	OnKeyBoardInput();
	UpdateCamera();

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
	UpdateMainPassCB(mMainPassCB, mCurrentFrameResource->PassCB);

	UpdateWaves(mTimer, mCurrentFrameResource->WavesVB, *this);
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
	buildFrameResources();

	HR(mDXCon->mCmdList->Close());
	ID3D12CommandList* cmdLists[] = { mDXCon->mCmdList.Get() };
	mDXCon->mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	mDXCon->flushCommandQueue();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f*XM_PI, (float)mDXCon->Window->width / mDXCon->Window->height, 1.0f, 1000.f);
	XMStoreFloat4x4(&mProj, p);
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
	else if (gGlobalEvents.IsMouseDown)
	{
		OnMouseDown(gGlobalEvents.MDWP, gGlobalEvents.MDX, gGlobalEvents.MDY);
		gGlobalEvents.IsMouseDown = false;
	}
	else if (gGlobalEvents.IsMouseUP)
	{
		OnMouseUp(gGlobalEvents.MUWP, gGlobalEvents.MUX, gGlobalEvents.MUY);
		gGlobalEvents.IsMouseUP = false;
	}
	else if (gGlobalEvents.isMouseMoving)
	{
		OnMouseMove(gGlobalEvents.MMWP, gGlobalEvents.MMX, gGlobalEvents.MMY);
		gGlobalEvents.isMouseMoving = false;
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
		staticSmaplers.size, staticSmaplers.data(),
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

void DX12Render::buildShaders()
{
	mShaders["standardVS"] = CompileShader("Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["opaquePS"] = CompileShader("Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	gInputLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	gInputLayout[1] = { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };
}

void DX12Render::buildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { gInputLayout, 2 };
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

void DX12Render::UpdateObjectPassCB(std::vector<RenderItem>& rItems,
	UploadBuffer<ObjectConstants>* currentObjectCB)
{
	for (int i = 0; i < rItems.size(); i++)
	{
		if (rItems[i].NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&rItems[i].WorldPos);

			ObjectConstants objConst;
			XMStoreFloat4x4(&objConst.World, XMMatrixTranspose(world));

			currentObjectCB->CopyData(rItems[i].ObjCBIndex, objConst);

			rItems[i].NumFramesDirty--;
		}
	}
}

void DX12Render::UpdateMaterialCBs(std::vector<Material>& allMaterials,
	UploadBuffer<MaterialConstants>* currMat)
{
	for (int i = 0; i < allMaterials.size(); i++)
	{
		Material* mat = &allMaterials[i];

		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConst;
			matConst.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConst.FresnelR0 = mat->FresnelR0;
			matConst.Roughness = mat->Roughness;

			currMat->CopyData(mat->MatCBIndex, matConst);

			mat->NumFramesDirty--;
		}
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

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objConst->GetGPUVirtualAddress() + ri.ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matConst->GetGPUVirtualAddress() + ri.MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		cmdList->DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.baseVertexLocation, 0);
	}
}

void DX12Render::UpdateCamera()
{
	mEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
	mEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
	mEyePos.y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
}

void DX12Render::UpdateMainPassCB(PassConstants& mainPassCB, UploadBuffer<PassConstants>* passCB)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mainPassCB.EyePosW = mEyePos;
	mainPassCB.RenderTargetSize = XMFLOAT2(mDXCon->Window->width, mDXCon->Window->height);
	mainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mDXCon->Window->width, 1.0f / mDXCon->Window->height);
	mainPassCB.NearZ = 1.0f;
	mainPassCB.FarZ = 1000.0f;
	mainPassCB.DeltaTime = mTimer->mDeltaTime;
	mainPassCB.TotalTime = mTimer->GetGameTime();
	mainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	XMVECTOR lightDir = -SphericalToCartesian(1.0f, mSunTheta, mSunPhi);

	XMStoreFloat3(&mainPassCB.Lights[0].Direction, lightDir);
	mainPassCB.Lights[0].Strength = { 1.0f, 1.0f, 0.9f };

	passCB->CopyData(0, mainPassCB);
}

void DX12Render::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePosition.x = x;
	mLastMousePosition.y = y;

	SetCapture(mDXCon->Window->hwnd);
}

void DX12Render::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DX12Render::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * (float)(x - mLastMousePosition.x));
		float dy = XMConvertToRadians(0.25f * (float)(y - mLastMousePosition.y));

		mTheta += dx;
		mPhi += dy;

		mPhi = Clamp(mPhi, 0.1f, XM_PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.05f * (float)(x - mLastMousePosition.x);
		float dy = 0.05f * (float)(y - mLastMousePosition.y);

		mRadius += dx - dy;
		mRadius = Clamp(mRadius, 5.0f, 150.0f);
	}

	mLastMousePosition.x = x;
	mLastMousePosition.y = y;
}

void DX12Render::OnKeyBoardInput()
{
	if (GetAsyncKeyState('1') & 0x8000)
		mIsWireFrame = true;
	else
		mIsWireFrame = false;
}