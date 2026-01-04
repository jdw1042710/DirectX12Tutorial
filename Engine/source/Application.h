#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "EngineHeader.h"
#include "Util.h"
#include "GameTimer.h"

// 필수적인 D3D12 라이브러리들을 링크
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Engine
{
	class D3D_API Application
	{
	public:
		Application(HINSTANCE hInstance);
		Application(const Application& rhs) = delete;
		Application& operator=(const Application& rhs) = delete;
		~Application();

	public:
		static Application* GetApp();
		
		inline HINSTANCE AppInst() const { return mhAppInst; }
		inline HWND MainWnd() const { return mhMainWnd; }
		inline float AspectRatio() const { return static_cast<float>(mClientWidth) / mClientHeight; }

		inline bool Get4xMsaaState() const { return m4xMsaaState; }
		inline void Set4xMsaaState(bool value);
		/// <summary>
		/// 어플리케이션 실행
		/// </summary>
		int Run();
		/// <summary>
		/// 어플리케이션 초기화
		/// </summary>
		virtual bool Initialize();
		/// <summary>
		/// 재정의시, 재정의된 함수에서 처리되지 못한 메세지는 반드시 기본 함수로 전달되어야 함 
		/// </summary>
		virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	protected:
		static Application* mApp;

		HINSTANCE mhAppInst = nullptr; // 프로그램 인스턴스 핸들
		HWND mhMainWnd = nullptr;      // 메인 윈도우 핸들
		bool mAppPaused = false;       // 애플리케이션 일시정지 여부
		bool mMinimized = false;     // 애플리케이션 최소화 여부
		bool mMaximized = false;     // 애플리케이션 전체화면 여부
		bool mResizing = false;       // 윈도우 크기 조절 중(= 리사이즈 바를 드래그 중) 여부
		
		// 4x MSAA를 사용한다면 true (기본은 false)
		bool m4xMsaaState = false; // 4x MSAA 활성화 여부
		UINT m4xMsaaQuality = 0;   // 4x MSAA 품질 수준
		
		// DeltaTime과 전체 게임 시간을 관리하는 타이머
		GameTimer mTimer;          // 게임 타이머
		
		Microsoft::WRL::ComPtr<ID3D12Device> mD3DDevice; // 디바이스
		Microsoft::WRL::ComPtr<IDXGIFactory4> mDxgiFactory; // DXGI 팩토리

		// CPU/GPU 동기화를 위한 펜스 변수
		UINT64 mCurrentFence = 0;
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

		// 명령 대기열, 명령 할당자, 명령 목록 변수
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue; // 명령 대기열
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc; // 명령 할당자
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList; // 명령 목록

		// 스왑 체인 변수
		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain; // 스왑 체인
		static const int SwapChainBufferCount = 2; // 스왑 체인 버퍼 개수
		int mCurrentBackBuffer = 0; // 현재 백버퍼 인덱스
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount]; // 스왑 체인 버퍼
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer; // 깊이-스텐실 버퍼

		// 서술자 힙 변수
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap; // RTV 서술자 힙
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap; // DSV 서술자 힙
		
		UINT mRtvDescriptorSize = 0; // RTV 서술자 크기
		UINT mDsvDescriptorSize = 0; // DSV 서술자 크기
		UINT mCbvSrvUavDescriptorSize = 0; // CBV/SRV/UAV 서술자 크기

		// 뷰포트 및 가위 사각형
		D3D12_VIEWPORT mScreenViewport;
		D3D12_RECT mScissorRect;

		// 상속되는 클래스는 반드시 이 아래 변수들을 생성자에서 초기화 해야 함
		std::wstring mMainWndCaption = L"Direct3D Application"; // 윈도우 창 제목
		D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE; // 디바이스 드라이버 타입
		DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // 백버퍼 포맷
		DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // 깊이-스텐실 버퍼 포맷
		int mClientWidth = 800;  // 클라이언트 영역 너비
		int mClientHeight = 600; // 클라이언트 영역 높이

	// 재정의 함수들
	protected:
		virtual void OnResize(); // 윈도우 크기 조절 시 호출
		virtual void CreateRtvAndDsvDescriptorHeaps();

		// 틱당 호출되는 업데이트 및 렌더링 함수
		virtual void Update(const GameTimer& gt) = 0;
		virtual void Draw(const GameTimer& gt) = 0;
		
		// 마우스 입력 이벤트 처리 함수들
		virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
		virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
		virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

	protected:
		/// <summary>
		/// 윈도우 창 초기화(Initialize 함수에서 호출됨)
		/// </summary>
		bool InitMainWindow();
		/// <summary>
		/// Direct3D 초기화(Initialize 함수에서 호출됨)
		/// </summary>
		/// <returns></returns>
		bool InitDirect3D();
		/// <summary>
		/// 명령 큐, 명령 할당자, 명령 목록 생성 함수
		/// </summary>
		void CreateCommandObjects();
		/// <summary>
		/// 스왑	체인 생성 함수
		/// </summary>
		void CreateSwapChain();

		/// <summary>
		/// CPU/GPU 동기화를 위한 Flush 함수
		/// </summary>
		void FlushCommandQueue();

		inline ID3D12Resource* CurrentBackBuffer() const { return mSwapChainBuffer[mCurrentBackBuffer].Get(); }
		inline D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mCurrentBackBuffer, mRtvDescriptorSize); }
		inline D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const { return mDsvHeap->GetCPUDescriptorHandleForHeapStart(); }

		/// <summary>
		/// 프레임 통계 계산 및 출력 함수
		/// </summary>
		void CalculateFrameStats();

		void LogAdapters();
		void LogAdapterOutputs(IDXGIAdapter* adapter);
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	};

}


