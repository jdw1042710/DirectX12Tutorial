#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// 함수 선언시 XMVECTOR와 마찬가지로 규약에 따라 XMMATRIX값을 전달 받아야 한다.
// XMVECTOR 매개 변수의 형식을 첫번째는 FXMatrix를, 나머지는 CXMMATRIX를 붙여야한다.
// 함수 이름 앞에넌 XM_CALLCONV라는 호출 규약 지시자를 붙여야 한다.

void XM_CALLCONV ExampleFunc(FXMMATRIX para1, CXMMATRIX para2)
{
	//계산시 SIMD의 장점을 활용할 수 있는 자료형
	XMMATRIX matrix;
	//Alignment를 위해 클래스 맴버 변수에 사용되는 자료형
	XMFLOAT4X4 matrix4;
	
	//위 자료형들의 load 및 store함수들
	XMLoadFloat4x4(&matrix4);
	XMStoreFloat4x4(&matrix4, matrix);

	//다양한 관련 함수들
	XMMatrixIdentity();
	XMMatrixIsIdentity(matrix);
	XMMatrixMultiply(matrix, matrix);
	XMMatrixTranspose(matrix);
	XMVECTOR dets = XMMatrixDeterminant(matrix);
	XMMatrixInverse(&dets, matrix);

}

// 생성자의 경우 XMVECTOR 매개 변수의 형식을 CXMVECTOR로만 사용해야한다
// XM_CALLCONV라는 호출 규약 지시자를 붙여서는 안된다.
// (아래는 임의로 함수 선언이지만 생성자 선언 조건과 동일하게 짠 예시다.)

void ExampleConstructor(CXMMATRIX para1)
{
}