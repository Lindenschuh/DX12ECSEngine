#pragma once
#include "..\Core\Default.h"
#include <WindowsX.h>
#include "../Ext/imgui_impl_win32.h"
#include "../Ext/imgui_impl_dx12.h"
#include <array>
const char gClassName[] = "WindowClassName";

// Variables
static const u32 mSwapchainBufferCount = 2;

struct GlobalEvent
{
	bool isResized = false;
	u32 ResizeWidth = 0;
	u32 ResizeHeight = 0;
};

extern GlobalEvent gGlobalEvents;

extern LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:

		gGlobalEvents.ResizeWidth = LOWORD(lParam);
		gGlobalEvents.ResizeHeight = HIWORD(lParam);
		gGlobalEvents.isResized = true;
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

typedef struct Win32Window
{
	HWND hwnd;
	u32 width;
	u32 height;
	Win32Window()
	{
	};
	Win32Window(int pwidth, int pheight, const char * windowTitle)
	{
		HINSTANCE hInstance = GetModuleHandleW(NULL);
		int nCommandShow = SW_SHOW;

		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = gClassName;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
		{
			fprintf(stdout, "Failed to create Windowclass\n");
		}

		width = pwidth;
		height = pheight;

		hwnd = CreateWindowEx(0,
			gClassName,
			windowTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			NULL, NULL, hInstance, NULL);

		if (hwnd == NULL)
		{
			fprintf(stdout, "Failed to created window\n");
		}

		ShowWindow(hwnd, nCommandShow);

		fprintf(stdout, "Window Init done\n");
	}
} Win32Window;

class DX12Context
{
public:

	ComPtr<IDXGIFactory4> mFactory = nullptr;
	ComPtr<ID3D12Device> mD3dDevice = nullptr;
	ComPtr<ID3D12Fence> mFence = nullptr;
	ComPtr<ID3D12CommandQueue> mCmdQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc = nullptr;
	ComPtr<ID3D12GraphicsCommandList> mCmdList = nullptr;
	ComPtr<IDXGISwapChain> mSwapchain = nullptr;

	ComPtr<ID3D12Resource> mSwapchainBuffer[mSwapchainBufferCount];
	ComPtr<ID3D12Resource> mDepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	ComPtr<ID3D12DescriptorHeap>  mTextureHeap;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	Win32Window* Window = nullptr;
	u64 mCurrentFence = 0;
	u32 m4xMsaaQuality = 0;

	u32 mRtvDescriptorSize = 0;
	u32 mDsvDescriptorSize = 0;
	u32 mCbvSrvUavDescriptorSize = 0;

	s32 mCurrentBackbuffer = 0;

	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_VIEWPORT mViewPort;
	D3D12_RECT mScissorRect;

public:
	DX12Context(u32 width, u32 height, const char* windowTitle);
	void flushCommandQueue();
	void ExecuteCmdList();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
	void resize(u32 width, u32 height);
private:
	void initDX12(void);
	void setMSAALevel();
	void createD3D12MainComponents();
	void createCommandQAL();
	void createSwapchain();
	void activateDebugLayer();
	void buildDescriptorHeaps();
};
