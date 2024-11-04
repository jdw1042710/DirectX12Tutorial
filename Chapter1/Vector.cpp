#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

// �Լ� ����� �Ʒ��� �Ծ࿡ ���� XMVECTOR���� ���� �޾ƾ� �Ѵ�.
// XMVECTOR �Ű� ������ ������ 1~3��°�� FXMVECTOR, 4��°�� GXMVECTOR
// 5~6��°�� HXMVECTOR��, �� �̻��� CXMVECTOR�� ����ؾ��Ѵ�.
// �Լ� �̸� �տ��� XM_CALLCONV��� ȣ�� �Ծ� �����ڸ� �ٿ��� �Ѵ�.

void XM_CALLCONV ExampleFunc(FXMVECTOR para1, FXMVECTOR para2, FXMVECTOR para3, GXMVECTOR para4, HXMVECTOR para5, HXMVECTOR para6, CXMVECTOR para7)
{
	//���� SIMD�� ������ Ȱ���� �� �ִ� �ڷ���
	XMVECTOR vector = { 0 };
	//Alignment�� ���� Ŭ���� �ɹ� ������ ���Ǵ� �ڷ���
	XMFLOAT2 vector2 = XMFLOAT2({ 0, 0 });
	XMFLOAT3 vector3 = XMFLOAT3({ 0, 0, 0 });
	XMFLOAT4 vector4 = XMFLOAT4({ 0, 0, 0, 0 });

	//�� �ڷ������� load �� store�Լ���
	XMLoadFloat2(&vector2);
	XMLoadFloat3(&vector3);
	XMLoadFloat4(&vector4);

	//XMVECTOR�� Getter, Setter �Լ���
	float x = 0, y = 0, z = 0;
	XMVectorGetX(vector);
	XMVectorGetY(vector);
	XMVectorGetZ(vector);
	XMVectorSetX(vector, x);
	XMVectorSetY(vector, y);
	XMVectorSetZ(vector, z);

	//const XMVECTOR �ν��Ͻ�
	const XMVECTORF32 halfVector = { 0.5f };
	//const XMVECTOR �ν��Ͻ�(���� ����)
	const XMVECTORU32 zeroVector = { 0 };
}

// �������� ��� XMVECTOR �Ű� ������ ������ 1~3��°�� FXMVECTOR��,
// �� �̻��� CXMVECTOR�� ����ؾ��Ѵ�.
// ����, �Լ��ʹ� �޸� XM_CALLCONV��� ȣ�� �Ծ� �����ڸ� �ٿ����� �ȵȴ�.
// (�Ʒ��� ���Ƿ� �Լ� ���������� ������ ���� ���ǰ� �����ϰ� § ���ô�.)

void ExampleConstructor(FXMVECTOR para1, FXMVECTOR para2, FXMVECTOR para3,  CXMVECTOR para4)
{
}

