#pragma once
#include "EngineHeader.h"
// 윈도우 라이브러리
#include <Windows.h>
#include <wrl.h>
#include <comdef.h>
// 다이렉트X 라이브러리
#include <dxgi1_4.h>
#include "d3dx12.h"
#include <D3Dcompiler.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
// std 라이브러리
#include <string>
#include <array>
#include <unordered_map>
#include <fstream>
#include <cassert>

namespace Engine
{
    inline std::wstring AnsiToWString(const std::string& str)
    {
		WCHAR buffer[512];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
		return std::wstring(buffer);
    }

    class D3D_API Util
    {
    public:
        inline static UINT CalcConstantBufferByteSize(UINT byteSize)
        {
            return (byteSize + 255) & ~255;
        }

        /// <summary>
        /// 바이너리 파일 읽기
        /// </summary>
        /// <param name="filename">파일 이름</param>
        /// <returns>파일 데이터</returns>
        static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

        /// <summary>
        /// 기본 힙 생성 함수
        /// </summary>
        /// <param name="initData">초기 데이터</param>
        /// <param name="byteSize">데이터 크기</param>
        /// <param name="uploadBuffer">데이터 복사를 위한 업로드 힙</param>
        static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
            ID3D12Device* device,
            ID3D12GraphicsCommandList* cmdList,
            const void* initData,
            UINT64 byteSize,
            Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

        /// <summary>
        /// 셰이더 컴파일 함수
        /// </summary>
        /// <param name="filename">컴파일 할 파일명</param>
        /// <param name="defines">컴파일 옵션</param>
        /// <param name="entrypoint">셰이더의 진입점 함수의 이름</param>
        /// <param name="target">사용할 셰이더의 종류와 버전</param>
        /// <returns>셰이더 바이트코드 포인터</returns>
        static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
            const std::wstring& filename,
            const D3D_SHADER_MACRO* defines,
            const std::string& entrypoint,
            const std::string& target);
    };

    class D3D_API DxException
    {
    public:
        DxException() = default;
        DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

        std::wstring ToString()const;

        HRESULT ErrorCode = S_OK;
        std::wstring FunctionName;
        std::wstring Filename;
        int LineNumber = -1;
    };

    // 하나의 정점/인덱스 버퍼에 여러개의 Geometry들이 저장되는 경우 사용됨
    struct SubmeshGeometry
    {
        UINT IndexCount = 0;
        UINT StartIndexLocation = 0;
        INT BaseVertexLocation = 0;

		DirectX::BoundingBox Bounds;
	};

    struct MeshGeometry
    {
        std::string Name;

        // 시스템 메모리의 복사본. 클라이언트가 적절한 형변환을 해야 함.
        Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
        Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

        // 버퍼 정보
        UINT VertexByteStride = 0;
        UINT VertexBufferByteSize = 0;
        DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
        UINT IndexBufferByteSize = 0;

        // 하나의 버텍스/인덱스 버퍼에 복수개의 Geometry가 저장되어 있을 수 있다.
        // SubmeshGeomtry를 사용해서 각 Submesh들을 개별적으로 그릴 수 있따.
        std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
        {
            D3D12_VERTEX_BUFFER_VIEW vbv;
            vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
            vbv.StrideInBytes = VertexByteStride;
            vbv.SizeInBytes = VertexBufferByteSize;
            return vbv;
        }

        D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
        {
            D3D12_INDEX_BUFFER_VIEW ibv;
            ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
            ibv.Format = IndexFormat;
            ibv.SizeInBytes = IndexBufferByteSize;
            return ibv;
        }
        // GPU에 업로드가 끝났다면, 메모리를 해제한다.
        void DisposeUploaders()
        {
            VertexBufferUploader = nullptr;
            IndexBufferUploader = nullptr;
        }
    };

}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif