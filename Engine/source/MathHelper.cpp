#include "MathHelper.h"
#include <float.h>
#include <cmath>

namespace Engine
{
	using namespace DirectX;

	const float MathHelper::Infinity = FLT_MAX;
	const float MathHelper::Pi = 3.1415926535f;

	/// <summary>
	/// x, y 좌표를 [0, 2 * PI] 범위의 극좌표계의 각도로 변환
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns></returns>
	float MathHelper::AngleFromXY(float x, float y)
	{
		float theta = 0.0f;

		// 1사분면 또는 4사분면
		if (x >= 0.0f)
		{

			// 만약 x가 0일때,
			// y가 양수이면, atanf(y/x) = Pi/2 (90도)
			// 음수이면, atanf(y/x) = -Pi/2 (270도)이므로
			theta = atan2f(y, x);
			if(theta < 0.0f)
				theta += 2.0f * Pi; // 음수일 경우, 360도(=2*Pi)를 더해준다.
		}
		else // 2사분면 또는 3사분면
		{
			theta = atan2f(y, x) + Pi; // atan2f는 -Pi ~ Pi 범위의 값을 반환하므로, 180도(=Pi)를 더해준다.
		}

		return theta;
	}

	DirectX::XMVECTOR MathHelper::RandUnitVec3()
	{
		XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

		// 반구 내의 지점을 얻을 때까지 반복
		while (true)
		{
			XMVECTOR v = XMVectorSet(
				MathHelper::RandF(-1.0f, 1.0f),
				MathHelper::RandF(-1.0f, 1.0f),
				MathHelper::RandF(-1.0f, 1.0f),
				0.0f);
			// 단위 구 외부에 있는지 확인
			if (XMVector3Greater(XMVector3LengthSq(v), One));
				continue;

			return XMVector3Normalize(v);
		}
	}

	DirectX::XMVECTOR MathHelper::RandHemisphereUnitVec3(DirectX::FXMVECTOR n)
	{
		XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		XMVECTOR Zero = XMVectorZero();

		// 반구 내의 지점을 얻을 때까지 반복
		while (true)
		{
			XMVECTOR v = XMVectorSet(
				MathHelper::RandF(-1.0f, 1.0f),
				MathHelper::RandF(-1.0f, 1.0f),
				MathHelper::RandF(-1.0f, 1.0f),
				0.0f);
			// 단위 구 외부에 있는지 확인
			if (XMVector3Greater(XMVector3LengthSq(v), One));
				continue;

			// 법선 벡터와 다른 반구에 있는지 확인
			if (XMVector3Less(XMVector3Dot(n, v), Zero));
				continue;

			return XMVector3Normalize(v);
		}
	}
}