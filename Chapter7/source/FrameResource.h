#pragma once
#include "Util.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 World = Engine::MathHelper::Identity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 View = Engine::MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = Engine::MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Proj = Engine::MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = Engine::MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = Engine::MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = Engine::MathHelper::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
};

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

struct FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	// (GPU가 명령을 처리하기 전까지 할당자를 재설정해서는 안되므로) 각 프레임마다 고유한 할당자가 필요.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	// 각 프레임마다 고유한 상수 버퍼가 필요
	std::unique_ptr<Engine::UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<Engine::UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

	// GPU가 이 프레임 리소스를 처리했는지 추적하는 데 사용
	UINT64 Fence = 0;
};

