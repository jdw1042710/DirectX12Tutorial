#include "Application.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

using namespace Engine;
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class BoxApp : public Application
{
public:
	BoxApp(HINSTANCE hInstance);
	BoxApp(const BoxApp& rhs) = delete; // 복사 생성자 삭제
	BoxApp& operator=(const BoxApp& rhs) = delete; // 대입 연산자 삭제
	~BoxApp();

	virtual bool Initialize() override;
private:
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr; // 루트 서명
	ComPtr<ID3D12PipelineState> mPSO = nullptr; // 파이프라인 상태 객체

	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr; // 상수 버퍼 서술자 힙
	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr; // 상수 버퍼

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr; // 박스 메쉬 기하 정보;

	ComPtr<ID3DBlob> mvsByteCode = nullptr; // 정점 쉐이더 데이터
	ComPtr<ID3DBlob> mpsByteCode = nullptr; // 픽셀 쉐이더 데이터

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout; // 입력 배치


	// 월드, 뷰, 투영 행렬
	XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * XM_PI; // 카메라의 수평 회전 각도
	float mPhi = XM_PIDIV4; // 카메라의 수직 회전 각도
	float mRadius = 5.0f; // 카메라와 원점 사이의 거리

	POINT mLastMousePos = {0, 0}; // 이전 마우스 위치 정보
private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

	/// <summary>
	/// 서술자 힙 (추가) 생성 : 상수 버퍼 서술자 힙
	/// </summary>
	void BuildDescriptorHeaps();
	/// <summary>
	/// 상수 버퍼 생성
	/// </summary>
	void BuildConstantBuffers();
	/// <summary>
	/// 루트 서명 생성 (서술자 테이블을 사용하여 생성)
	/// </summary>
	void BuildRootSignature();
	/// <summary>
	/// 셰이더 불러오기 & 입력 서술 초기화
	/// </summary>
	void BuildShadersAndInputLayout();
	/// <summary>
	/// 박스 메시 생성
	/// </summary>
	void BuildBoxGeometry();
	/// <summary>
	/// 파이프라인 상태 객체 초기화
	/// </summary>
	void BuildPSO();
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstanec, PSTR cmdLine, int showCmd)
{
	// 디버그 활성화 시, 실시간 메모리 체크
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		BoxApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;
		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

BoxApp::BoxApp(HINSTANCE hInstance) : Application(hInstance)
{}

BoxApp::~BoxApp()
{}

bool BoxApp::Initialize()
{
	// 기본 Application 초기화 호출
	if(!Application::Initialize())
		return false;
	// 초기화 명령들을 준비하기 위해, 명령 리스트 재설정
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// 초기화 명령들을 실행
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// 초기화가 완료될 때까지 대기.
	FlushCommandQueue();

	return true;
}

void BoxApp::OnResize()
{
	Application::OnResize();
	// 윈도우 크기가 변경될 때마다, 투영 행렬을 업데이트
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::Update(const GameTimer& gt)
{
	// view 행렬 업데이트
	XMVECTOR pos = MathHelper::SphericalToCartesian(mRadius, mTheta, mPhi); // 구좌표계를 직교 좌표계로 변환
	XMVECTOR target = XMVectorZero(); // 원점 방향을 바라봄
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 월드 업 벡터

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	// 월드-뷰-투영 행렬 계산
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	// 상수 버퍼에 데이터 복사
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	mObjectCB->CopyData(0, objConstants);
}

void BoxApp::Draw(const GameTimer& gt)
{
	// 메모리 재사용
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// 명령목록 재사용
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 백버퍼를 쓰기 상태로 변환
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 백배퍼와 깊이 버퍼를 Clear
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 출력 병합기에 렌더타겟 설정
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// 현재 렌더링 파이프라인에서 사용할 상수 버퍼 힙 서술자 전달
	ID3D12DescriptorHeap* descriptorHeap[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);

	// 현재 렌더링 파이프라인에서 사용할 루트 서명 설정
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	// 입력조립기 단계에 정점/인덱스 데이터 및 토폴로지 전달
	mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	// 루트 서명에 서술자 힙 연결
	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	// 그리기 명령
	mCommandList->DrawIndexedInstanced(mBoxGeo->DrawArgs["box"].IndexCount, 
		1, mBoxGeo->DrawArgs["box"].StartIndexLocation, mBoxGeo->DrawArgs["box"].BaseVertexLocation, 0);

	// 백 버퍼를 출력 상태로 전환
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 명령 기록 완료
	ThrowIfFailed(mCommandList->Close());

	// 명령 힙에 명령 목록 전달
	ID3D12CommandList* cmdLists[] = { mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// 버퍼 스왑
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % SwapChainBufferCount;

	// 명령이 완료될 때까지 대기
	FlushCommandQueue();
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	// 이 창이 마우스 이벤트를 지속적으로 받도록 설정
	SetCapture(mhMainWnd); 
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	// SetCapture 해제
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// 움직인 픽셀을 각도로 변환 (1/4배)
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// 입력 값을 바탕으로 각도를 갱신
		mTheta += dx;
		mPhi += dy;

		// mPhi의 각도를 제한
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// 움직인 픽셀을 계산(0.005 unit 크기)
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// 입력 값을 바탕으로 반지름을 계산
		mRadius += dx - dy; // 증가 : 위/오른쪽, 감소 : 아래/왼쪽

		// 반지름의 길이를 제한
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}


void BoxApp::BuildDescriptorHeaps()
{
	// 상수 버퍼 서술자 힙 서술
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(mD3DDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(mD3DDevice.Get(), 1, true);
	
	UINT objCBByteSize = Util::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
	int boxCBufIndex = 0; // 업로드 버퍼 안의 i번째 (박스) 상수 버퍼 객체의 offset 값 (왜 있는건지 모르겠다...)
	cbAddress += boxCBufIndex * objCBByteSize; // (어짜피 0 더해지는데...)

	/// 상수 버퍼 서술자 서술
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objCBByteSize;

	mD3DDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
	// 쉐이더 프로그램은 주로 입력으로 리소스(= 상수 버퍼, 텍스쳐, 샘플러)를 필요로 하는데,
	// 루트 서명은 쉐이더 프로그램이 기대하고 있는 리소스를 정의한다.
	// 쉐이더 프로그램 : 함수, 리소스 : 매개변수 -> 루트 서명 : 함수 시그니처
	
	// 루트 파라미터는 서술자 테이블, 루트 서술자, 루트 상수를 통해 생성될 수 있다. (이번 챕터에선 서술자 테이블로 생성함)
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// 단일 서술자 테이블을 생성
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // 0번째부터 1개의 리소스 생성 (ex. b0 ...)
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// 루트 서명은 루트 파라미터의 배열이다.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT); // 파라미터 배열 크기, 파라미터 값...
	
	// 단일 상수 버퍼로 구성된 DESCRIPTOR RANGE를 가리키는 단일 slot을 통해 루트 서명을 생성
	ComPtr<ID3DBlob> serializedRootSig = nullptr; // 루트 서명 데이터
	ComPtr<ID3DBlob> errorBlob = nullptr; // 에러 데이터
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	
	// 에러 체크 1
	if (errorBlob != nullptr)
	{
		// 에러 발생
		OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
	}
	// 에러 체크 2
	ThrowIfFailed(hr);

	// 루트 서명 생성
	ThrowIfFailed(mD3DDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void BoxApp::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;
	// 셰이더 프로그램 컴파일
	mvsByteCode = Util::CompileShader(L"source\\shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = Util::CompileShader(L"source\\shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	// 입력 서술 초기화
	mInputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}, // 12 바이트
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}, // 16 바이트
	};
}

void BoxApp::BuildBoxGeometry()
{
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,
		// back face
		4, 6, 5,
		4, 7, 6,
		// left face
		4, 5, 1,
		4, 1, 0,
		// right face
		3, 2, 6,
		3, 6, 7,
		// top face
		1, 5, 6,
		1, 6, 2,
		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
	const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(std::uint16_t));

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	// 버텍스 버퍼 데이터 (CPU) 입력
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	// 인덱스 버퍼 데이터 (CPU) 입력
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	
	// 버텍스 버퍼 데이터 (GPU) 입력
	mBoxGeo->VertexBufferGPU = Util::CreateDefaultBuffer(mD3DDevice.Get(), mCommandList.Get(),
		vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	// 인덱스 버퍼 데이터 (GPU) 입력
	mBoxGeo->IndexBufferGPU = Util::CreateDefaultBuffer(mD3DDevice.Get(), mCommandList.Get(),
		indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry subMesh;
	subMesh.IndexCount = static_cast<UINT>(indices.size());
	subMesh.StartIndexLocation = 0;
	subMesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = subMesh;
}

void BoxApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), static_cast<UINT>(mInputLayout.size()) }; // 입력 서술 설정 (셰이더 함수 입력)
	psoDesc.pRootSignature = mRootSignature.Get(); // 루트 서명 설정 (셰이더 함수 리소스 서명)
	psoDesc.VS = { reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), mvsByteCode->GetBufferSize() }; // 정점 셰이더 설정
	psoDesc.PS = { reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), mpsByteCode->GetBufferSize() }; // 픽셀 셰이더 설정
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // 레스터화기 상태 설정
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // 블랜드 설정
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // 깊이-스텐실 버퍼 상태 설정
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // 토폴로지 설정
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(mD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}
