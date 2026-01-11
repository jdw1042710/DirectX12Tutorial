#pragma once
#include "EngineHeader.h"
#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

namespace Engine
{
	class D3D_API MathHelper
	{
	public:
		/// <summary>
		/// 0.0f ~ 1.0f 사이의 랜덤 실수 반환
		/// </summary>
		/// <returns></returns>
		inline static float RandF()
		{
			return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		}

		/// <summary>
		/// [a, b) 사이의 랜덤 실수 반환
		/// </summary>
		inline static float RandF(float a, float b)
		{
			return a + RandF() * (b - a);
		}

		/// <summary>
		/// [a, b] 사이의 랜덤 정수 반환
		/// </summary>
		inline int Rand(int a, int b)
		{
			return a + rand() % ((b - a) + 1);
		}

		template<typename T>
		inline static T Min(const T& a, const T& b)
		{
			return a < b ? a : b;
		}

		template<typename T>
		inline static T Max(const T& a, const T& b)
		{
			return a > b ? a : b;
		}

		/// <summary>
		/// a와 b 사이를 t 비율만큼 선형 보간
		/// </summary>
		template<typename T>
		inline static T Lerp(const T& a, const T& b, float t)
		{
			return a + (b - a) * t;
		}

		/// <summary>
		/// x를 low와 high 사이로 클램핑
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="x"></param>
		/// <param name="low"></param>
		/// <param name="high"></param>
		/// <returns></returns>
		template<typename T>
		inline static T Clamp(const T& x, const T& low, const T& high)
		{
			return x < low ? low : (x > high ? high : x);
		}

		/// <summary>
		/// (x, y) 좌표를 [0, 2 * PI] 범위의 극좌표계의 각도로 변환
		/// </summary>
		static float AngleFromXY(float x, float y);

		/// <summary>
		/// 구면	좌표계를 직교 좌표계로 변환
		/// </summary>
		inline static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
		{
			return DirectX::XMVectorSet(
				radius * sinf(phi) * cosf(theta),
				radius * cosf(phi),
				radius * sinf(phi) * sinf(theta),
				1.0f);
		}

		/// <summary>
		/// 행렬	M의 역전치 행렬을 반환
		/// </summary>
		static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
		{
			// InverseTranspose는 주로 법선 벡터를 변환하는 데 사용된다.
			// 행렬의 평행 이동 성분은 벡터에 영향을 미치지 않으므로,
			// 따라서 역전치 행렬을 계산할 때 변환 행렬의 평행 이동 성분은 무시해야 한다.
			DirectX::XMMATRIX A = M;
			A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
			return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
		}

		static DirectX::XMFLOAT4X4 Identity4x4()
		{
			static DirectX::XMFLOAT4X4 I(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			return I;
		}
		/// <summary>
		/// 크기가 1인 랜덤 벡터 반환 (구면 내)
		/// </summary>
		/// <returns></returns>
		static DirectX::XMVECTOR RandUnitVec3();

		/// <summary>
		/// 랜덤 단위 벡터 반환 (반구 내)
		/// </summary>
		/// <param name="n"></param>
		/// <returns></returns>
		static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::FXMVECTOR n);

		static const float Infinity;
		static const float Pi;
	};
}


