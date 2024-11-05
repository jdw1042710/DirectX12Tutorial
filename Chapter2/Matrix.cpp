#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// �Լ� ����� XMVECTOR�� ���������� �Ծ࿡ ���� XMMATRIX���� ���� �޾ƾ� �Ѵ�.
// XMVECTOR �Ű� ������ ������ ù��°�� FXMatrix��, �������� CXMMATRIX�� �ٿ����Ѵ�.
// �Լ� �̸� �տ��� XM_CALLCONV��� ȣ�� �Ծ� �����ڸ� �ٿ��� �Ѵ�.

void XM_CALLCONV ExampleFunc(FXMMATRIX para1, CXMMATRIX para2)
{
	//���� SIMD�� ������ Ȱ���� �� �ִ� �ڷ���
	XMMATRIX matrix;
	//Alignment�� ���� Ŭ���� �ɹ� ������ ���Ǵ� �ڷ���
	XMFLOAT4X4 matrix4;
	
	//�� �ڷ������� load �� store�Լ���
	XMLoadFloat4x4(&matrix4);
	XMStoreFloat4x4(&matrix4, matrix);

	//�پ��� ���� �Լ���
	XMMatrixIdentity();
	XMMatrixIsIdentity(matrix);
	XMMatrixMultiply(matrix, matrix);
	XMMatrixTranspose(matrix);
	XMVECTOR dets = XMMatrixDeterminant(matrix);
	XMMatrixInverse(&dets, matrix);

}

// �������� ��� XMVECTOR �Ű� ������ ������ CXMVECTOR�θ� ����ؾ��Ѵ�
// XM_CALLCONV��� ȣ�� �Ծ� �����ڸ� �ٿ����� �ȵȴ�.
// (�Ʒ��� ���Ƿ� �Լ� ���������� ������ ���� ���ǰ� �����ϰ� § ���ô�.)

void ExampleConstructor(CXMMATRIX para1)
{
}