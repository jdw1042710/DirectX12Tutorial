#include "Application.h"

using namespace Engine;
using namespace DirectX;

class SimpleDirect3DApp : public Application
{
public :
    SimpleDirect3DApp(HINSTANCE hInstance);
	~SimpleDirect3DApp();

	virtual bool Initialize() override;
private:
    virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;
};


SimpleDirect3DApp::SimpleDirect3DApp(HINSTANCE hInstance)
	: Application(hInstance) {}

SimpleDirect3DApp::~SimpleDirect3DApp() {}

bool SimpleDirect3DApp::Initialize()
{
    if(!Application::Initialize()) return false;
	return true;
}

void SimpleDirect3DApp::OnResize()
{
    Application::OnResize();
}

void SimpleDirect3DApp::Update(const Engine::GameTimer& gt) { }

void SimpleDirect3DApp::Draw(const Engine::GameTimer& gt)
{
	// 명령 할당자 초기화
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// 명령 목록 초기화
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// 현재 백버퍼를 렌더 타겟 상태로 전환
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT,	D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 뷰포트와 가위 사각형을 설정. 이는 명령 목록이 초기화 될 때마다 설정해줘야 함
    mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

    // 현재 백버퍼와 깊이 버퍼를 초기화
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(),
		Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 현재 백버퍼와 깊이 버퍼를 렌더 타겟으로 설정
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    // 현재 백버퍼를 프레젠트 상태로 전환
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 명령 목록을 닫고, 실행
	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 스왑 체인 프레젠트
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % SwapChainBufferCount;

	// GPU가 명령을 완료할 때까지 대기
	FlushCommandQueue();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    using namespace Engine;
    try
    {
        SimpleDirect3DApp app(hInstance);
        if (!app.Initialize())
            return 0;
        


        return app.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}