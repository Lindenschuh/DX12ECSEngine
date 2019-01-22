#include <assert.h>
#include "DX12Renderer.h"

DX12Renderer::DX12Renderer(u32 width, u32 height, const char* windowName, EntityManger* eManager)
{
	mTimer = new GameTimer();
	mDXCon = new DX12Context(width, height, windowName);

	BuildRootSignature();
	mTextureSystem = new TextureSystem(mDXCon);
	mMaterialSystem = new MaterialSystem(mDXCon, mTextureSystem);
	mGeometrySystem = new GeometrySystem(mDXCon);
	mShaderSystem = new ShaderSystem(mDXCon);
	mPSOSystem = new PSOSystem(mDXCon, mShaderSystem, mRootSignature.Get());
	mCameraSystem = new CameraSystem(eManager);
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
	BuildSkyBox();
	mTextureSystem->UploadTextures();
	u32 instanceCount = 0;
	for (int i = 0; i < RenderLayer::Count; i++)
	{
		for (int j = 0; j < mRItems[i].size(); j++)
		{
			instanceCount += mRItems[i][j].Instances.size();
		}
	}

	mFrameResourceSystem = new FrameResourceSystem(gNumFrameResources, mDXCon, 1,
		instanceCount, mMaterialSystem->GetMaterialCount());

	HR(mDXCon->mCmdList->Close());
	ID3D12CommandList* cmdLists[] = { mDXCon->mCmdList.Get() };
	mDXCon->mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	mDXCon->flushCommandQueue();

	mCameraSystem->SetFrustum(mCameraSystem->GetMainCamera(), 0.25f*XM_PI,
		(float)mDXCon->Window->width / mDXCon->Window->height, 1.0f, 1000.0f);
	Update(ImGui::GetTime(), ImGui::GetIO().DeltaTime);
}

void DX12Renderer::SetFogData(XMFLOAT4 fogColor, float fogStart, float fogRange)
{
	mFogData.FogColor = fogColor;
	mFogData.FogStart = fogStart;
	mFogData.FogRange = fogRange;
}

void DX12Renderer::SetLayerPSO(std::string psoName, RenderLayer layer)
{
	LayerPSO[layer] = psoName;
}

std::string DX12Renderer::GetLayerPSO(RenderLayer layer)
{
	return LayerPSO[layer];
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

	HR(mDXCon->mCmdList->Reset(cmdListAlloc.Get(), mPSOSystem->GetPSO(LayerPSO[RenderLayer::Opaque]).PSOData.Get()));

	mDXCon->mCmdList->RSSetViewports(1, &mDXCon->mViewPort);
	mDXCon->mCmdList->RSSetScissorRects(1, &mDXCon->mScissorRect);

	mDXCon->mCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackbuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mDXCon->mCmdList->ClearRenderTargetView(CurrentbackBufferView(), (float*)&mFogData.FogColor, 0, nullptr);
	mDXCon->mCmdList->ClearDepthStencilView(DepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mDXCon->mCmdList->OMSetRenderTargets(1, &CurrentbackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mDXCon->mTextureHeap.Get() };
	mDXCon->mCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mDXCon->mCmdList->SetGraphicsRootSignature(mRootSignature.Get());

	auto materialBuffer = currentFrameResource.MaterialBuffer->Resource();
	mDXCon->mCmdList->SetGraphicsRootShaderResourceView(1, materialBuffer->GetGPUVirtualAddress());

	auto passCB = currentFrameResource.PassCB->Resource();
	mDXCon->mCmdList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mDXCon->mTextureHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(mTextureSystem->GetSkyBoxID(), mDXCon->mCbvSrvUavDescriptorSize);
	mDXCon->mCmdList->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

	mDXCon->mCmdList->SetGraphicsRootDescriptorTable(4, mDXCon->mTextureHeap->GetGPUDescriptorHandleForHeapStart());

	u32 drawedObjects = 0;
	DrawRenderItems(mDXCon->mCmdList.Get(), mRItems[RenderLayer::Opaque], drawedObjects);

	mDXCon->mCmdList->SetPipelineState(mPSOSystem->GetPSO(LayerPSO[RenderLayer::Skybox]).PSOData.Get());
	DrawRenderItems(mDXCon->mCmdList.Get(), mRItems[RenderLayer::Skybox], drawedObjects);

	mDXCon->mCmdList->SetPipelineState(mPSOSystem->GetPSO(LayerPSO[RenderLayer::AlphaTest]).PSOData.Get());
	DrawRenderItems(mDXCon->mCmdList.Get(), mRItems[RenderLayer::AlphaTest], drawedObjects);

	mDXCon->mCmdList->SetPipelineState(mPSOSystem->GetPSO(LayerPSO[RenderLayer::Transparent]).PSOData.Get());
	DrawRenderItems(mDXCon->mCmdList.Get(), mRItems[RenderLayer::Transparent], drawedObjects);

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

void DX12Renderer::Update(float time, float deltaTime)
{
	mFrameResourceSystem->SwitchFrameResource();
	mCameraSystem->UpdateSystem(time, deltaTime);
	FrameResource& currentFrameResource = mFrameResourceSystem->GetCurrentFrameResource();
	CameraComponent comp = mCameraSystem->GetMainCameraComp();

	UpdateObjectPassCB(mRItems, currentFrameResource.InstanceBuffer, comp.ViewMat, comp.FrustrumBounds);

	mMaterialSystem->UpdateMaterials(currentFrameResource.MaterialBuffer);
	UpdateMainPassCB(mMainPassCB, currentFrameResource.PassCB, comp.ViewMat, mCameraSystem->GetMainCameraPos()
		, comp.ProjMat, mDXCon, mTimer, mFogData);
}

void DX12Renderer::ProcessGlobalEvents()
{
	if (gGlobalEvents.isResized)
	{
		mDXCon->resize(gGlobalEvents.ResizeWidth, gGlobalEvents.ResizeHeight);

		gGlobalEvents.isResized = false;
		mCameraSystem->SetFrustum(mCameraSystem->GetMainCamera(),
			0.25f*XM_PI, (float)mDXCon->Window->width / mDXCon->Window->height, 1.0f, 1000.f);
	}
}

void DX12Renderer::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTableSkyBox;
	texTableSkyBox.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, gNumOfTexures, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[5];
	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTableSkyBox, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[4].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSmaplers = mDXCon->GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
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

void DX12Renderer::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem>&rItems, u32& INOUToffset)
{
	FrameResource mCurrentFrameResource = mFrameResourceSystem->GetCurrentFrameResource();

	u32 instanceSize = sizeof(InstanceData);

	for (int i = 0; i < rItems.size(); i++)
	{
		RenderItem& ri = rItems[i];
		MeshGeometry& geo = mGeometrySystem->GetMeshGeomerty(ri.GeoIndex);

		cmdList->IASetVertexBuffers(0, 1, &geo.VertexBufferView());
		cmdList->IASetIndexBuffer(&geo.IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri.PrimitiveType);

		auto instanceBuffer = mCurrentFrameResource.InstanceBuffer->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS instanceBufferAddress = instanceBuffer->GetGPUVirtualAddress() + instanceSize * (INOUToffset);

		cmdList->SetGraphicsRootShaderResourceView(0, instanceBufferAddress);
		cmdList->DrawIndexedInstanced(ri.IndexCount, ri.InstancesVisible, ri.StartIndexLocation, ri.baseVertexLocation, 0);
		INOUToffset += ri.InstancesVisible;
		ri.InstanceUpdated = 0;
	}
}

void DX12Renderer::BuildSkyBox()
{
	mTextureSystem->LoadSkybox("SkyBox", L"Textures/grasscube1024.dds");
	mShaderSystem->LoadShader("SkyboxVS", L"Shaders\\Sky.hlsl", VertexShader);
	mShaderSystem->LoadShader("SkyboxPS", L"Shaders\\Sky.hlsl", PixelShader);
	mPSOSystem->BuildSkyboxPSO("SkyboxPSO", "SkyboxVS", "SkyboxPS", DefaultPSOOptions());

	SetLayerPSO("SkyboxPSO", RenderLayer::Skybox);

	MaterialConstants mc = { XMFLOAT4(1.0f,1.0f,1.0f,1.0f),XMFLOAT3(0.1f, 0.1f, 0.1f), 1.0f };

	MaterialID mId = mMaterialSystem->BuildMaterial("SkyBox", 0, 0, mc);
	GeometryGenerator geoGen;
	MeshData md = geoGen.CreateSphere(0.5f, 20, 20);

	std::vector<Vertex> vertices(md.Vertices.size());
	for (size_t i = 0; i < md.Vertices.size(); ++i)
	{
		auto& p = md.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = md.Vertices[i].Normal;
		vertices[i].TexC = md.Vertices[i].TexCoord;
		vertices[i].Tangent = md.Vertices[i].TangentU;
	}

	Submesh sm = { md.Indicies.size(),0,0 };
	std::string subMeshName = "Skybox";

	GeoInfo gInfo = {};
	gInfo.Name = "SkyboxCube";
	gInfo.indiceCount = md.Indicies.size();
	gInfo.indicies = md.Indicies.data();
	gInfo.vertCount = vertices.size();
	gInfo.verts = vertices.data();
	gInfo.SubmeshNames = &subMeshName;
	gInfo.submeshs = &sm;
	gInfo.submeshCount;

	GeometryID gID = mGeometrySystem->LoadGeometry(gInfo);
	RenderItem skybox;
	skybox.GeoIndex = gID;
	skybox.Bounds = sm.Bounds;
	skybox.baseVertexLocation = sm.BaseVertexLocation;
	skybox.IndexCount = sm.IndexCount;
	skybox.StartIndexLocation = sm.StartIndexLocation;

	InstanceData SkyBoxData;
	SkyBoxData.MaterialIndex = mId;
	XMStoreFloat4x4(&SkyBoxData.World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	skybox.Instances.push_back(SkyBoxData);

	mRItems[Skybox].push_back(skybox);
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