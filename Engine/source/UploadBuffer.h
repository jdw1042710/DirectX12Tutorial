#pragma once

#include "Util.h"

namespace Engine
{
	template<typename T>
	class UploadBuffer
	{
	public:
		/// <param name="device"> 현재 디바이스 </param>
		/// <param name="elementCount"> 버퍼의 요소 개수 </param>
		/// <param name="isConstantBuffer"> 상수 버퍼 여부 </param>
		UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : mIsConstantBuffer(isConstantBuffer)
		{
			mElementByteSize = sizeof(T);

			// 만약 업로드 버퍼가 상수 버퍼라면 256바이트로 정렬해야 함
			if(isConstantBuffer)
				mElementByteSize = Util::CalcConstantBufferByteSize(mElementByteSize); // 256바이트 정렬 함수

			// 업로드 버퍼 생성
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mUploadBuffer)));

			// 버퍼를 CPU가 접근할 수 있도록 매핑
			ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));
		}
		UploadBuffer(const UploadBuffer& rhs) = delete; // 복사 생성 금지
		UploadBuffer& operator=(const UploadBuffer& rhs) = delete; // 복사 대입 금지
		~UploadBuffer()
		{
			if(mUploadBuffer != nullptr)
				mUploadBuffer->Unmap(0, nullptr);
			mMappedData = nullptr;
		}

		/// <summary>
		/// 업로드 버퍼 자원 반환
		/// </summary>
		/// <returns></returns>
		inline ID3D12Resource* Resource() const
		{
			return mUploadBuffer.Get();
		}

		/// <summary>
		/// elementIndex 위치에 data 복사
		/// </summary>
		/// <param name="elementIndex">복사해넣을 요소의 인덱스</param>
		/// <param name="data">복사해넣을 데이터</param>
		inline void CopyData(UINT elementIndex, const T& data)
		{
			memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer; // 업로드 버퍼 자원
		BYTE* mMappedData = nullptr; // 버퍼의 CPU 접근 포인터

		UINT mElementByteSize = 0; // 버퍼 요소의 하나 크기
		bool mIsConstantBuffer = false; // 상수 버퍼 여부
	};
}