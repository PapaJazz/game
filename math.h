#ifndef math_h
#define math_h

#include <math.h>

// CONSTANT //
float math_constant_pifloat = 3.14159265359f;
float math_constant_degreesPerRadian = .01745329f;


// 2D VECTOR //
struct math_2D_vector
{
	float x;
	float y;
};
math_2D_vector math_2D_addVectors(math_2D_vector _A, math_2D_vector _B)
{
	math_2D_vector _result;
	_result.x = _A.x + _B.x;
	_result.y = _A.y + _B.y;
	return _result;
}
math_2D_vector math_2D_subtractVectors(math_2D_vector _A, math_2D_vector _B)
{
	math_2D_vector _result;
	_result.x = _A.x - _B.x;
	_result.y = _A.y - _B.y;
	return _result;
}
math_2D_vector math_2D_multiplyVectorByScalar(math_2D_vector _A, float _Scalar)
{
	math_2D_vector _result;
	_result.x = _A.x * _Scalar;
	_result.y = _A.y * _Scalar;
	return _result;
}
float math_2D_dotProductVectors(math_2D_vector _A, math_2D_vector _B)
{
	float _result = _A.x * _B.x + _A.y * _B.y;
	return _result;
}



// 3D VECTOR //
struct math_3D_vector
{
	float x;
	float y;
	float z;
};
math_3D_vector math_3D_addVectors(math_3D_vector _A, math_3D_vector _B)
{
	math_3D_vector _result;
	_result.x = _A.x + _B.x;
	_result.y = _A.y + _B.y;
	_result.z = _A.z + _B.z;
	return _result;
}
math_3D_vector math_3D_subtractVectors(math_3D_vector _A, math_3D_vector _B)
{
	math_3D_vector _result;
	_result.x = _A.x - _B.x;
	_result.y = _A.y - _B.y;
	_result.z = _A.z - _B.z;
	return _result;
}
math_3D_vector math_3D_multiplyVectorByScalar(math_3D_vector _A, float _Scalar)
{
	math_3D_vector _result;
	_result.x = _A.x * _Scalar;
	_result.y = _A.y * _Scalar;
	_result.y = _A.z * _Scalar;
	return _result;
}
float math_3D_dotProductVectors(math_3D_vector _A, math_3D_vector _B)
{
	float _result = _A.x * _B.x + _A.y * _B.y + _A.z * _B.z;
	return _result;
}


// QUATERNION //
struct math_quaternion_struct
{
	float x;
	float y;
	float z;
	float w;
};
math_quaternion_struct math_quaternion_add(math_quaternion_struct _A, math_quaternion_struct _B)
{
	math_quaternion_struct _result = {};
	return _result;
}
math_quaternion_struct math_quaternion_normalize(math_quaternion_struct _Quaternion)
{
	math_quaternion_struct _result;
	float _length = (float)sqrt(_Quaternion.x * _Quaternion.x + _Quaternion.y * _Quaternion.y + _Quaternion.z * _Quaternion.z + _Quaternion.w * _Quaternion.w);
	_result.x = _Quaternion.x / _length;
	_result.y = _Quaternion.y / _length;
	_result.z = _Quaternion.z / _length;
	_result.w = _Quaternion.w / _length;
	return _result;
}
math_quaternion_struct math_quaternion_multiplyTwoQuaternions(math_quaternion_struct _A, math_quaternion_struct _B)
{
	math_quaternion_struct _result = {};
	_result.w = _A.w * _B.w - _A.x * _B.x - _A.y * _B.y - _A.z * _B.z;
	_result.x = _A.x * _B.w + _A.w * _B.x + _A.y * _B.z - _A.z * _B.y;
	_result.y = _A.y * _B.w + _A.w * _B.y + _A.z * _B.x - _A.x * _B.z;
	_result.z = _A.z * _B.w + _A.w * _B.z + _A.x * _B.y - _A.y * _B.x;
	return _result;
}



// 4D VECTOR //
struct math_4D_vector 
{
	float x;
	float y;
	float z;
	float w;
};
const math_4D_vector math_4D_leftVector = {-1,0,0,1};
const math_4D_vector math_4D_rightVector = {1,0,0,1};
const math_4D_vector math_4D_upVector = {0,1,0,1};
const math_4D_vector math_4D_downVector = {0,-1,0,1};
const math_4D_vector math_4D_forwardVector = {0,0,1,1};
const math_4D_vector math_4D_backwardVector = {0,0,-1,1};
math_4D_vector math_4D_addVectors(math_4D_vector _A, math_4D_vector _B)
{
	math_4D_vector _result;

	_result.x = _A.x + _B.x;
	_result.y = _A.y + _B.y;
	_result.z = _A.z + _B.z;
	_result.w = _A.w + _B.w;

	return _result;
}
math_quaternion_struct math_quaternion_multiplyQuaternionByPoint(math_quaternion_struct _Quaternion, math_4D_vector _Point)
{
	math_quaternion_struct _result = {};
	_result.w = -_Quaternion.x * _Point.x - _Quaternion.y * _Point.y - _Quaternion.z * _Point.z;
	_result.x =  _Quaternion.w * _Point.x + _Quaternion.y * _Point.z - _Quaternion.z * _Point.y;
	_result.y =  _Quaternion.w * _Point.y + _Quaternion.z * _Point.x - _Quaternion.x * _Point.z;
	_result.z =  _Quaternion.w * _Point.z + _Quaternion.x * _Point.y - _Quaternion.y * _Point.x;
	return _result;
}
math_4D_vector math_4D_rotate(math_quaternion_struct _Rotation, math_4D_vector _Point)
{
	math_4D_vector _result; 
	math_quaternion_struct _inverseQuaternion = {-_Rotation.x, -_Rotation.y, -_Rotation.z, _Rotation.w};
	math_quaternion_struct _rotatedPoint; 

	_rotatedPoint = math_quaternion_multiplyQuaternionByPoint(_Rotation, _Point);
	_rotatedPoint = math_quaternion_multiplyTwoQuaternions(_rotatedPoint, _inverseQuaternion);

	_result.x = _rotatedPoint.x;
	_result.y = _rotatedPoint.y;
	_result.z = _rotatedPoint.z;
	_result.w = 1.0f;

	return _result;
}
math_4D_vector math_4D_scale(math_4D_vector _Vector, float _Amount)
{
	math_4D_vector _result;

	_result.x = _Vector.x * _Amount;
	_result.y = _Vector.y * _Amount;
	_result.z = _Vector.z * _Amount;
	_result.w = _Vector.w * _Amount;

	return _result;
}
math_4D_vector math_4D_add(math_4D_vector _A, math_4D_vector _B)
{
	math_4D_vector _result;

	_result.x = _A.x + _B.x;
	_result.y = _A.y + _B.y;
	_result.z = _A.z + _B.z;
	_result.w = _A.w + _B.w;

	return _result;
}
math_4D_vector math_4D_divideByPerspective(math_4D_vector _Vector)
{
	math_4D_vector _result;

	_result.x = _Vector.x / _Vector.w;
	_result.y = _Vector.y / _Vector.w;
	_result.z = _Vector.z / _Vector.w;
	_result.w = _Vector.w;

	return _result;
}
int math_4D_vectorIsInsideViewFrustum(math_4D_vector _Vector)
{
	return (fabsf(_Vector.x) <= fabsf(_Vector.w) && (fabsf(_Vector.y) <= fabsf(_Vector.w)) && (fabsf(_Vector.z) <= fabsf(_Vector.w)));
}


// 4D MATRIX //
struct math_4D_matrix
{
	float e[4][4]; // element
};
math_4D_matrix math_4D_multiplyTwoMatricies(math_4D_matrix _A, math_4D_matrix _B)
{
	math_4D_matrix _result = {};
	int _i = 0;
	while(_i < 4)
	{
		int _j = 0;
		while(_j < 4)
		{
			_result.e[_i][_j] = _A.e[_i][0] * _B.e[0][_j] 
									+ _A.e[_i][1] * _B.e[1][_j] 
									+ _A.e[_i][2] * _B.e[2][_j] 
									+ _A.e[_i][3] * _B.e[3][_j];
			_j++;
		}
		_i++;
	}
	return _result;
}
math_4D_matrix math_4D_createTranslationMatrix(float _X, float _Y, float _Z)
{
	math_4D_matrix _result;

	_result.e[0][0] = 1; _result.e[0][1] = 0; _result.e[0][2] = 0; _result.e[0][3] = _X;
	_result.e[1][0] = 0; _result.e[1][1] = 1; _result.e[1][2] = 0; _result.e[1][3] = _Y;
	_result.e[2][0] = 0; _result.e[2][1] = 0; _result.e[2][2] = 1; _result.e[2][3] = _Z;
	_result.e[3][0] = 0; _result.e[3][1] = 0; _result.e[3][2] = 0; _result.e[3][3] = 1;

	return _result;
}
math_4D_matrix math_4D_createRotationMatrix(float _X, float _Y, float _Z)
{
	math_4D_matrix _rx;
	math_4D_matrix _ry;
	math_4D_matrix _rz;

	_rz.e[0][0] = (float)cos(_Z); _rz.e[0][1] = -(float)sin(_Z); 	_rz.e[0][2] = 0; _rz.e[0][3] = 0;
	_rz.e[1][0] = (float)sin(_Z); _rz.e[1][1] = (float)cos(_Z); 	_rz.e[1][2] = 0; _rz.e[1][3] = 0;
	_rz.e[2][0] = 0; 					_rz.e[2][1] = 0; 						_rz.e[2][2] = 1; _rz.e[2][3] = 0;
	_rz.e[3][0] = 0;					_rz.e[3][1] = 0; 						_rz.e[3][2] = 0; _rz.e[3][3] = 1;

	_rx.e[0][0] = 1; _rx.e[0][1] = 0; 					_rx.e[0][2] = 0; 						_rx.e[0][3] = 0;
	_rx.e[1][0] = 0; _rx.e[1][1] = (float)cos(_X); 	_rx.e[1][2] = -(float)sin(_X); 	_rx.e[1][3] = 0;
	_rx.e[2][0] = 0; _rx.e[2][1] = (float)sin(_X); 	_rx.e[2][2] = (float)cos(_X); 	_rx.e[2][3] = 0;
	_rx.e[3][0] = 0; _rx.e[3][1] = 0; 					_rx.e[3][2] = 0; 						_rx.e[3][3] = 1;

	_ry.e[0][0] = (float)cos(_Y); _ry.e[0][1] = 0; _ry.e[0][2] = -(float)sin(_Y); _ry.e[0][3] = 0;
	_ry.e[1][0] = 0; 					_ry.e[1][1] = 1; _ry.e[1][2] = 0; 					_ry.e[1][3] = 0;
	_ry.e[2][0] = (float)sin(_Y); _ry.e[2][1] = 0; _ry.e[2][2] = (float)cos(_Y); 	_ry.e[2][3] = 0;
	_ry.e[3][0] = 0; 					_ry.e[3][1] = 0; _ry.e[3][2] = 0; 					_ry.e[3][3] = 1;

	math_4D_matrix _result;
	_result = math_4D_multiplyTwoMatricies(_ry, _rx);
	_result = math_4D_multiplyTwoMatricies(_rz, _result);
	return _result;
}
math_4D_matrix math_4D_createPerspectiveMatrix(float _FieldOfView, float _AspectRatio, float _ZNear, float _ZFar)
{
	math_4D_matrix _result;
	float _tanHalfFOV = (float)tan(_FieldOfView / 2);
	float _zRange = _ZNear - _ZFar;


_result.e[0][0] = 1.0f / (_tanHalfFOV * _AspectRatio);
_result.e[0][1] = 0;
_result.e[0][2] = 0;
_result.e[0][3] = 0;
_result.e[1][0] = 0;
_result.e[1][1] = 1.0f / _tanHalfFOV;
_result.e[1][2] = 0;
_result.e[1][3] = 0;
_result.e[2][0] = 0;
_result.e[2][1] = 0;
_result.e[2][2] = (-_ZNear -_ZFar)/_zRange;
_result.e[2][3] = 2 * _ZFar * _ZNear / _zRange;
_result.e[3][0] = 0;
_result.e[3][1] = 0;
_result.e[3][2] = 1;
_result.e[3][3] = 0;



#if 0
	_result.e[0][0] = 1.0f / (_tanHalfFOV * _AspectRatio);
	_result.e[0][1] = 0;
	_result.e[0][2] = 0;
	_result.e[0][3] = 0;
	_result.e[1][1] = 1.0f / _tanHalfFOV;
	_result.e[1][2] = 0;
	_result.e[1][3] = 0;
	_result.e[2][0] = 0;
	_result.e[2][1] = 0;
	_result.e[2][2] = (-_ZNear -_ZFar) / _zRange;
	_result.e[2][3] = 2 * _ZFar * _ZNear / _zRange;
	_result.e[3][0] = 0;
	_result.e[3][1] = 0;
	_result.e[3][2] = 1;
	_result.e[3][3] = 0;
#endif 

	return _result;
}
math_4D_matrix math_4D_createScaleMatrix(float _X, float _Y, float _Z)
{
	math_4D_matrix _result;

	_result.e[0][0] = _X; _result.e[0][1] = 0; _result.e[0][2] = 0; _result.e[0][3] = 0;
	_result.e[1][0] = 0; _result.e[1][1] = _Y; _result.e[1][2] = 0; _result.e[1][3] = 0;
	_result.e[2][0] = 0; _result.e[2][1] = 0; _result.e[2][2] = _Z; _result.e[2][3] = 0;
	_result.e[3][0] = 0; _result.e[3][1] = 0; _result.e[3][2] = 0; _result.e[3][3] = 1;

	return _result;
}
math_4D_matrix math_4D_createScreenSpaceMatrix(float _HalfWidth, float _HalfHeight)
{
	math_4D_matrix _result;

	_result.e[0][0] = _HalfWidth; _result.e[0][1] = 0; 				_result.e[0][2] = 0; _result.e[0][3] = _HalfWidth - 0.5f;
	_result.e[1][0] = 0; 			_result.e[1][1] = _HalfHeight; 	_result.e[1][2] = 0; _result.e[1][3] = _HalfHeight - 0.5f;
	_result.e[2][0] = 0; 			_result.e[2][1] = 0; 				_result.e[2][2] = 1; _result.e[2][3] = 0;
	_result.e[3][0] = 0; 			_result.e[3][1] = 0; 				_result.e[3][2] = 0; _result.e[3][3] = 1;

	return _result;
}
math_4D_matrix math_4D_createIdentityMatrix(void)
{
	math_4D_matrix _result;

	_result.e[0][0] = 1; _result.e[0][1] = 0; _result.e[0][2] = 0; _result.e[0][3] = 0;
	_result.e[1][0] = 0; _result.e[1][1] = 1; _result.e[1][2] = 0; _result.e[1][3] = 0;
	_result.e[2][0] = 0; _result.e[2][1] = 0; _result.e[2][2] = 1; _result.e[2][3] = 0;
	_result.e[3][0] = 0; _result.e[3][1] = 0; _result.e[3][2] = 0; _result.e[3][3] = 1;

	return _result;
}
math_4D_vector math_4D_transformVectorByMatrix(math_4D_vector _Vector, math_4D_matrix _Matrix)
{
	math_4D_vector _result;

	_result.x = _Matrix.e[0][0] * _Vector.x + _Matrix.e[0][1] * _Vector.y + _Matrix.e[0][2] * _Vector.z + _Matrix.e[0][3] * _Vector.w;
	_result.y = _Matrix.e[1][0] * _Vector.x + _Matrix.e[1][1] * _Vector.y + _Matrix.e[1][2] * _Vector.z + _Matrix.e[1][3] * _Vector.w;
	_result.z = _Matrix.e[2][0] * _Vector.x + _Matrix.e[2][1] * _Vector.y + _Matrix.e[2][2] * _Vector.z + _Matrix.e[2][3] * _Vector.w;
	_result.w = _Matrix.e[3][0] * _Vector.x + _Matrix.e[3][1] * _Vector.y + _Matrix.e[3][2] * _Vector.z + _Matrix.e[3][3] * _Vector.w;

	return _result;
}
math_4D_matrix math_4D_createRotationMatrix(math_quaternion_struct _Quaternion)
{
	math_4D_vector _forwardVector = {2.0f * (_Quaternion.x * _Quaternion.z - _Quaternion.w * _Quaternion.y), 2.0f * (_Quaternion.y * _Quaternion.z + _Quaternion.w * _Quaternion.x), 1.0f - 2.0f * (_Quaternion.x * _Quaternion.x + _Quaternion.y * _Quaternion.y), 1.0f};
	math_4D_vector _upVector = {2.0f * (_Quaternion.x * _Quaternion.y + _Quaternion.w * _Quaternion.z), 1.0f - 2.0f * (_Quaternion.x * _Quaternion.x + _Quaternion.z * _Quaternion.z), 2.0f * (_Quaternion.y * _Quaternion.z - _Quaternion.w * _Quaternion.x), 1.0f};
	math_4D_vector _rightVector = {1.0f - 2.0f * (_Quaternion.y * _Quaternion.y + _Quaternion.z * _Quaternion.z), 2.0f * (_Quaternion.x * _Quaternion.y - _Quaternion.w * _Quaternion.z), 2.0f * (_Quaternion.x * _Quaternion.z + _Quaternion.w * _Quaternion.y), 1.0f};

	math_4D_matrix _result;

	_result.e[0][0] = _rightVector.x;	_result.e[0][1] = _rightVector.y;	_result.e[0][2] = _rightVector.z;	_result.e[0][3] = 0;
	_result.e[1][0] = _upVector.x;		_result.e[1][1] = _upVector.z;		_result.e[1][2] = _upVector.z;		_result.e[1][3] = 0;
	_result.e[2][0] = _forwardVector.x;	_result.e[2][1] = _forwardVector.y;	_result.e[2][2] = _forwardVector.z; _result.e[2][3] = 0;
	_result.e[3][0] = 0;						_result.e[3][1] = 0;						_result.e[3][2] = 0;						_result.e[3][3] = 1;

	return _result;
}
#if 0
math_4D_matrix math_4D_createRotationMatrix(Vector4f forward, Vector4f up, Vector4f right)
{
	Vector4f f = forward;
	Vector4f r = right;
	Vector4f u = up;

	m[0][0] = r.GetX();	m[0][1] = r.GetY();	m[0][2] = r.GetZ();	m[0][3] = 0;
	m[1][0] = u.GetX();	m[1][1] = u.GetY();	m[1][2] = u.GetZ();	m[1][3] = 0;
	m[2][0] = f.GetX();	m[2][1] = f.GetY();	m[2][2] = f.GetZ();	m[2][3] = 0;
	m[3][0] = 0;		m[3][1] = 0;		m[3][2] = 0;		m[3][3] = 1;

	return this;
}
#endif
#endif
