#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

// 함수 선언시 아래의 규약에 따라 XMVECTOR값을 전달 받아야 한다.
// XMVECTOR 매개 변수의 형식을 1~3번째는 FXMVECTOR, 4번째는 GXMVECTOR
// 5~6번째는 HXMVECTOR를, 그 이상은 CXMVECTOR를 사용해야한다.
// 함수 이름 앞에넌 XM_CALLCONV라는 호출 규약 지시자를 붙여야 한다.

void XM_CALLCONV ExampleFunc(FXMVECTOR para1, FXMVECTOR para2, FXMVECTOR para3, GXMVECTOR para4, HXMVECTOR para5, HXMVECTOR para6, CXMVECTOR para7)
{
	//계산시 SIMD의 장점을 활용할 수 있는 자료형
	XMVECTOR vector = { 0 };
	//Alignment를 위해 클래스 맴버 변수에 사용되는 자료형
	XMFLOAT2 vector2 = XMFLOAT2({ 0, 0 });
	XMFLOAT3 vector3 = XMFLOAT3({ 0, 0, 0 });
	XMFLOAT4 vector4 = XMFLOAT4({ 0, 0, 0, 0 });

	//위 자료형들의 load 및 store함수들
	XMLoadFloat2(&vector2);
	XMLoadFloat3(&vector3);
	XMLoadFloat4(&vector4);

	//XMVECTOR의 Getter, Setter 함수들
	float x = 0, y = 0, z = 0;
	XMVectorGetX(vector);
	XMVectorGetY(vector);
	XMVectorGetZ(vector);
	XMVectorSetX(vector, x);
	XMVectorSetY(vector, y);
	XMVectorSetZ(vector, z);

	//const XMVECTOR 인스턴스
	const XMVECTORF32 halfVector = { 0.5f };
	//const XMVECTOR 인스턴스(정수 버전)
	const XMVECTORU32 zeroVector = { 0 };
}

// 생성자의 경우 XMVECTOR 매개 변수의 형식을 1~3번째는 FXMVECTOR를,
// 그 이상은 CXMVECTOR를 사용해야한다.
// 또한, 함수와는 달리 XM_CALLCONV라는 호출 규약 지시자를 붙여서는 안된다.
// (아래는 임의로 함수 선언이지만 생성자 선언 조건과 동일하게 짠 예시다.)

void ExampleConstructor(FXMVECTOR para1, FXMVECTOR para2, FXMVECTOR para3,  CXMVECTOR para4)
{
}

