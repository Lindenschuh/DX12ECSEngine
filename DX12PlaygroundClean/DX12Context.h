#pragma once
#include "Default.h"
#include <WindowsX.h>
const char gClassName[] = "WindowClassName";

// Variables
static const u32 mSwapchainBufferCount = 2;

struct GlobalEvent
{
	bool isResized = false;
	u32 ResizeWidth = 0;
	u32 ResizeHeight = 0;
	bool IsMouseDown = false;
	WPARAM MDWP = 0;
	u32 MDX = 0;
	u32 MDY = 0;
	bool IsMouseUP = false;
	WPARAM MUWP = 0;
	u32 MUX = 0;
	u32 MUY = 0;
	bool isMouseMoving = false;
	WPARAM MMWP = 0;
	u32 MMX = 0;
	u32 MMY = 0;
};

extern GlobalEvent gGlobalEvents;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
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

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		gGlobalEvents.IsMouseDown = true;
		gGlobalEvents.MDWP = wParam;
		gGlobalEvents.MDX = GET_X_LPARAM(lParam);
		gGlobalEvents.MDY = GET_Y_LPARAM(lParam);
		return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		gGlobalEvents.IsMouseUP = true;
		gGlobalEvents.MUWP = wParam;
		gGlobalEvents.MUX = GET_X_LPARAM(lParam);
		gGlobalEvents.MUY = GET_Y_LPARAM(lParam);
		return 0;

	case WM_MOUSEMOVE:
		gGlobalEvents.isMouseMoving = true;
		gGlobalEvents.MMWP = wParam;
		gGlobalEvents.MMX = GET_X_LPARAM(lParam);
		gGlobalEvents.MMY = GET_Y_LPARAM(lParam);
		return 0;
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
