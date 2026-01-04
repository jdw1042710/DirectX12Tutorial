#include "Application.h"
#include <WindowsX.h>
#include <vector>

namespace Engine
{
	using Microsoft::WRL::ComPtr;
	using namespace std;
	using namespace DirectX;

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return Application::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
	}

	Application* Application::mApp = nullptr;
	Application::Application(HINSTANCE hInstance) 
		: mhAppInst(hInstance)
	{
		assert(mApp == nullptr); // app이 두개 이상 생성될 시, 오류 발생
		mApp = this;
	}

	Application::~Application()
	{
		// 명령 목록 방출
		if (mD3DDevice != nullptr)
			FlushCommandQueue();
	}

	Application* Application::GetApp()
	{
		return mApp;
	}

	inline void Application::Set4xMsaaState(bool value)
	{
		if (m4xMsaaState != value)
		{
			m4xMsaaState = value;
			// 4x MSAA 상태가 변경되었으므로, 스왑 체인과 버퍼를 다시 생성
			CreateSwapChain();
			OnResize();
		}
	}

	int Application::Application::Run()
	{
		MSG msg = { 0 };

		mTimer.Reset();

		while (msg.message != WM_QUIT)
		{
			// If there are Window messages then process them.
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// Otherwise, do animation/game stuff.
			else
			{
				mTimer.Tick();

				if (!mAppPaused)
				{
					CalculateFrameStats();
					Update(mTimer);
					Draw(mTimer);
				}
				else
				{
					Sleep(100);
				}
			}
		}

		return (int)msg.wParam;
	}

	bool Application::Initialize()
	{
		// 윈도우 창 초기화
		if (!InitMainWindow()) return false;
		// Direct3D 초기화
		if (!InitDirect3D()) return false;
		// 창 크기 조정
		OnResize();
		return true;
	}
	LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{	
			// 윈도우 창이 활성화/비활성화 될 때 전송됨
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				// 윈도우창이 비활성화 시, 게임 일시정지
				mAppPaused = true;
				mTimer.Stop();
			}
			else
			{
				// 윈도우창이 활성화 시, 게임 재개
				mAppPaused = false;
				mTimer.Start();
			}
			return 0;
			// 윈도우 창 크기가 변경될 때 전송됨
		case WM_SIZE:
			// Save the new client area dimensions.
			mClientWidth = LOWORD(lParam);
			mClientHeight = HIWORD(lParam);
			if (mD3DDevice)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					mAppPaused = true;
					mMinimized = true;
					mMaximized = false;
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					mAppPaused = false;
					mMinimized = false;
					mMaximized = true;
					OnResize();
				}
				else if (wParam == SIZE_RESTORED)
				{

					// Restoring from minimized state?
					if (mMinimized)
					{
						mAppPaused = false;
						mMinimized = false;
						OnResize();
					}

					// Restoring from maximized state?
					else if (mMaximized)
					{
						mAppPaused = false;
						mMaximized = false;
						OnResize();
					}
					// 만약 사용자가 리사이즈 바를 드래그하고 있는 중이라면,
					else if (mResizing)
					{
						// WM_SIZE 메시지가 연속적으로 전송되는데,
						// 수신시마다 버퍼 크기를 변경하는 것은 비효율적이고, 속도가 느리므로,
						// 사용자가 사이즈 바에서 손을 때는 순간에 WM_EXITSIZEMOVE 메시지가 전송되므로,
						// 이때 버퍼를 재설정한다.
					}
					// API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					else 
					{
						OnResize();
					}
				}
			}
			return 0;
			// WM_ENTERSIZEMOVE는 사용자가 리사이즈 바를 잡을 때 전송됨
		case WM_ENTERSIZEMOVE:
			mAppPaused = true;
			mResizing = true;
			mTimer.Stop();
			return 0;
			// WM_EXITSIZEMOVE는 사용자가 리사이즈 바에서 손을 뗄 때 전송됨
			// 여기서 모든 것을 새로운 창 크기에 맞게 재설정한다.
		case WM_EXITSIZEMOVE:
			mAppPaused = false;
			mResizing = false;
			mTimer.Start();
			OnResize();
			return 0;

			// WM_DESTROY는 창이 파괴될 때 전송됨
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			// WM_MENUCHAR는 메뉴가 활성화되어 있고, 사용자가 단축키나 엑셀러레이터 키에 해당하지 않는 키를 눌렀을 때 전송됨
		case WM_MENUCHAR:
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);
			// 윈도우 크기가 너무 작아지는 것을 방지
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_MOUSEMOVE:
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_KEYUP:
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			else if ((int)wParam == VK_F2)
				Set4xMsaaState(!m4xMsaaState);

			return 0;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	void Application::OnResize()
	{
		assert(mD3DDevice);
		assert(mSwapChain);
		assert(mDirectCmdListAlloc);

		// 명령 목록이 실행 중일 수 있으므로, 대기
		FlushCommandQueue();
		// 명령 목록을 초기화
		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

		// 모든 스왑 체인 및 깊이 스텐실 버퍼를 해제
		for (int i = 0; i < SwapChainBufferCount; ++i)
			mSwapChainBuffer[i].Reset();
		mDepthStencilBuffer.Reset();

		// 스왑 체인 버퍼 크기 조정
		ThrowIfFailed(mSwapChain->ResizeBuffers(
			SwapChainBufferCount,
			mClientWidth, mClientHeight,
			mBackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		mCurrentBackBuffer = 0;

		// 스왑 체인 버퍼들에 대한 렌더 타겟 뷰를 다시 생성
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < SwapChainBufferCount; ++i)
		{
			ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
			mD3DDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
			rtvHeapHandle.Offset(1, mRtvDescriptorSize);
		}
		// 깊이-스텐실 버퍼 및 뷰를 다시 생성
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = mClientWidth;
		depthStencilDesc.Height = mClientHeight;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		// SSAO 챕터에서 깊이 버퍼로부터 데이터를 읽기 위해 SRV가 필요하다.
		// 따라서 동일한 자원에 대해 두가지 뷰를 생성해야한다.
		// 1. SRV 타입: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		// 2. DSV 타입: DXGI_FORMAT_D24_UNORM_S8_UINT
		// 따라서 버퍼 리소스를 typeless 포맷으로 생성해야 한다.
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear; // 초기화 값
		optClear.Format = mDepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		// 깊이-스텐실 버퍼 생성
		ThrowIfFailed(mD3DDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

		// 모든 리소스에 대한 밉맵레벨 0인 뷰를 만든다.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = mDepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		mD3DDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

		// 깊이-스텐실 버퍼를 일반 상태에서 깊이-스텐실 쓰기 상태로 전환
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			mDepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE));

		// 명령 목록을 닫고, 실행
		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// GPU가 명령을 완료할 때까지 대기
		FlushCommandQueue();

		// 뷰포트 및 가위 사각형을 클라이언트 영역 크기에 맞게 설정
		mScreenViewport.TopLeftX = 0;
		mScreenViewport.TopLeftY = 0;
		mScreenViewport.Width = static_cast<float>(mClientWidth);
		mScreenViewport.Height = static_cast<float>(mClientHeight);
		mScreenViewport.MinDepth = 0.0f;
		mScreenViewport.MaxDepth = 1.0f;

		mScissorRect = { 0, 0, mClientWidth, mClientHeight };
	}

	void Application::CreateRtvAndDsvDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(mD3DDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(mD3DDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDsvHeap)));
	}

	bool Application::InitMainWindow()
	{
		WNDCLASS wc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = MainWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = mhAppInst;
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = L"MainWnd";

		if (!RegisterClass(&wc))
		{
			MessageBox(0, L"RegisterClass Failed.", 0, 0);
			return false;
		}

		// 윈도우 박스 크기 계산
		RECT R = { 0, 0, mClientWidth, mClientHeight };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		int width = R.right - R.left;
		int height = R.bottom - R.top;
		mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
			WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			width, height, 0, 0, mhAppInst, 0);

		if (!mhMainWnd)
		{
			MessageBoxW(0, L"CreateWindow Failed.", 0, 0);
			return false;
		}
		ShowWindow(mhMainWnd, SW_SHOW);
		UpdateWindow(mhMainWnd);
		return true;
	}

	bool Application::InitDirect3D()
	{
		// D3D12 디버그 계층 활성화
		#if defined(DEBUG) || defined(_DEBUG)
		
		#endif	
		{
			ComPtr<ID3D12Debug> debugController;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
		}

		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mDxgiFactory)));

		// 하드웨어 디바이스 생성 시도
		HRESULT hardwareResult = D3D12CreateDevice(
			nullptr, // 기본 어댑터
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mD3DDevice));
		// 실패시 소프트웨어 디바이스(WARP) 생성
		if (FAILED(hardwareResult))
		{
			ComPtr<IDXGIAdapter> pWarpAdapter;
			ThrowIfFailed(mDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
			ThrowIfFailed(D3D12CreateDevice(
				pWarpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&mD3DDevice)));
		}
		// CPU/GPU 동기화를 위한 펜스 생성
		ThrowIfFailed(mD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

		mRtvDescriptorSize = mD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		mDsvDescriptorSize = mD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		mCbvSrvUavDescriptorSize = mD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// 4x MSAA 품질 수준을 확인
		// Direct3D 11이 돌아가는 모든 하드웨어는 4x MSAA를 지원하므로,
		// 품질 수준만을 확인하면 됨
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = mBackBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0; // 결괏값이 여기 저장됨
		ThrowIfFailed(mD3DDevice->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msQualityLevels,
			sizeof(msQualityLevels)));
		m4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level."); // 품질 수준이 0보다 커야 함
#ifdef _DEBUG
		LogAdapters();
#endif
		CreateCommandObjects(); // 명령대기열, 명령할당자, 명령목록 생성
		CreateSwapChain();    // 스왑체인 생성
		CreateRtvAndDsvDescriptorHeaps(); // RTV, DSV 서술자 힙 생성

		return true;
	}
	void Application::CreateCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		// 명령 대기열 생성
		ThrowIfFailed(mD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
		// 명령 할당자 생성
		ThrowIfFailed(mD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));
		// 명령 목록 생성
		ThrowIfFailed(mD3DDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mDirectCmdListAlloc.Get(), // 명령 할당자
			nullptr, // 초기 파이프라인 상태 객체
			IID_PPV_ARGS(mCommandList.GetAddressOf())));

		// 명령 목록은 생성시엔 기록 상태이므로, 이후 Reset을 호출하기 위해선 Close로 닫아줘야 함
		mCommandList->Close(); 
	}
	void Application::CreateSwapChain()
	{
		// 스왑 체인을 다시 생성하기 전에 이전 스왑 체인을 정리
		mSwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferDesc.Width = mClientWidth;
		sd.BufferDesc.Height = mClientHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = mBackBufferFormat;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = SwapChainBufferCount;
		sd.OutputWindow = mhMainWnd;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// 스왑 체인은 명령 대기열을 이용해서 Flush를 수행한다.
		ThrowIfFailed(mDxgiFactory->CreateSwapChain(
			mCommandQueue.Get(),
			&sd,
			mSwapChain.GetAddressOf()));
	}

	void Application::FlushCommandQueue()
	{
		// 펜스 값을 하나 증가시켜,
		// 현재 울타리 지점까지의 명령들을 표시한다.
		++mCurrentFence;

		// 현재 울타리 지점을 설정(하는 명령을 GPU에 추가)한다.
		ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

		// CPU가 현재 울타리 지점에 도달할 때까지 대기
		if (mFence->GetCompletedValue() < mCurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			// 펜스가 현재 울타리 지점에 도달하면 이벤트가 발생되도록 펜스에 알림을 등록
			ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));
			// 이벤트가 발생될 때까지 대기
			WaitForSingleObject(eventHandle, INFINITE);
			// 이벤트 핸들 닫기
			CloseHandle(eventHandle);
		}
	}

	void Application::CalculateFrameStats()
	{
		
		static int frameCount = 0;
		static float timeElapsed = 0.0f;

		frameCount++;

		// 1초마다의 프레임 수와 밀리초 단위의 프레임 시간을 계산
		if ((mTimer.TotalTime() - timeElapsed) >= 1.0f)
		{
			float fps = (float)frameCount; // 초당 프레임 수
			float mspf = 1000.0f / fps;    // 프레임 당 밀리초

			std::wstring fpsString = std::to_wstring(fps);
			std::wstring mspfString = std::to_wstring(mspf);

			std::wstring windowText = mMainWndCaption +
				L"    fps: " + fpsString +
				L"    mspf: " + mspfString;

			// 윈도우 캡션 바에 출력
			SetWindowText(mhMainWnd, windowText.c_str());

			// 다음 계산을 위해 초기화
			frameCount = 0;
			timeElapsed += 1.0f;
		}
	}

	void Application::LogAdapters()
	{
		UINT i = 0;
		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapterList;
		while (mDxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			std::wstring text = L"***Adapter: ";
			text += desc.Description;
			text += L"\n";

			OutputDebugString(text.c_str());

			adapterList.push_back(adapter);

			++i;
		}

		for (size_t i = 0; i < adapterList.size(); ++i)
		{
			LogAdapterOutputs(adapterList[i]);
			ReleaseCom(adapterList[i]);
		}
	}
	void Application::LogAdapterOutputs(IDXGIAdapter* adapter)
	{
		UINT i = 0;
		IDXGIOutput* output = nullptr;
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);

			std::wstring text = L"***Output: ";
			text += desc.DeviceName;
			text += L"\n";
			OutputDebugString(text.c_str());

			LogOutputDisplayModes(output, mBackBufferFormat);

			ReleaseCom(output);

			++i;
		}
	}
	void Application::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
	{
		UINT count = 0;
		UINT flags = 0;

		// 리스트 갯수를 얻기 위해 nullptr를 전달
		output->GetDisplayModeList(format, flags, &count, nullptr);

		std::vector<DXGI_MODE_DESC> modeList(count);
		output->GetDisplayModeList(format, flags, &count, &modeList[0]);

		for (auto& x : modeList)
		{
			UINT n = x.RefreshRate.Numerator;
			UINT d = x.RefreshRate.Denominator;
			std::wstring text =
				L"Width = " + std::to_wstring(x.Width) + L" " +
				L"Height = " + std::to_wstring(x.Height) + L" " +
				L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
				L"\n";

			::OutputDebugString(text.c_str());
		}
	}
}

