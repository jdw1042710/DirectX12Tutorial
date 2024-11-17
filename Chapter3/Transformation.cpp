#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

void main()
{
	//비례행렬
	float ScaleX, ScaleY, ScaleZ;
	XMMatrixScaling(ScaleX, ScaleY, ScaleZ);
	FXMVECTOR Scale = { 1, 1, 1 };
	XMMatrixScalingFromVector(Scale);

	//회전행렬
	float AngleX, AngleY, AngleZ;
	XMMatrixRotationX(AngleX);
	XMMatrixRotationY(AngleY);
	XMMatrixRotationZ(AngleZ);
	FXMVECTOR Axis = { 1, 0, 0 };
	float Angle;
	XMMatrixRotationAxis(Axis, Angle);

	//이동행렬
	float OffsetX, OffsetY, OffsetZ;
	XMMatrixTranslation(OffsetX, OffsetY, OffsetZ);
	FXMVECTOR Offset = { 1, 1, 1 };
	XMMatrixTranslationFromVector(Offset);

	//벡터 대 행렬 곱 : vM
	FXMVECTOR Vector = { 1, 1, 1 };
	FXMMATRIX Matrix = XMMatrixSet
		(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	//v의 w에 1을 넣은 경우의 곱(점 Transform): w = 1
	XMVector3TransformCoord(Vector, Matrix);

	//v의 w에 0을 넣은 경우의 곱(벡터 Transform): w = 0
	XMVector3TransformNormal(Vector, Matrix);

}