#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

void main()
{
	//������
	float ScaleX, ScaleY, ScaleZ;
	XMMatrixScaling(ScaleX, ScaleY, ScaleZ);
	FXMVECTOR Scale = { 1, 1, 1 };
	XMMatrixScalingFromVector(Scale);

	//ȸ�����
	float AngleX, AngleY, AngleZ;
	XMMatrixRotationX(AngleX);
	XMMatrixRotationY(AngleY);
	XMMatrixRotationZ(AngleZ);
	FXMVECTOR Axis = { 1, 0, 0 };
	float Angle;
	XMMatrixRotationAxis(Axis, Angle);

	//�̵����
	float OffsetX, OffsetY, OffsetZ;
	XMMatrixTranslation(OffsetX, OffsetY, OffsetZ);
	FXMVECTOR Offset = { 1, 1, 1 };
	XMMatrixTranslationFromVector(Offset);

	//���� �� ��� �� : vM
	FXMVECTOR Vector = { 1, 1, 1 };
	FXMMATRIX Matrix = XMMatrixSet
		(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	//v�� w�� 1�� ���� ����� ��(�� Transform): w = 1
	XMVector3TransformCoord(Vector, Matrix);

	//v�� w�� 0�� ���� ����� ��(���� Transform): w = 0
	XMVector3TransformNormal(Vector, Matrix);

}