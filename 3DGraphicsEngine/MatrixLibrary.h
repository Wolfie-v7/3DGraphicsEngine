#include <math.h>
#include <corecrt_math_defines.h>
#include "Geometry.h"

struct mat4x4
{
	float matrix[4][4];

	mat4x4() : matrix{ 0 } {}

};

struct IdentityMatrix : mat4x4
{
	IdentityMatrix() : mat4x4()
	{
		matrix[0][0] = 1.0f;
		matrix[1][1] = 1.0f;
		matrix[2][2] = 1.0f;
		matrix[3][3] = 1.0f;

	}
};

struct TranslationMatrix : mat4x4
{

	TranslationMatrix(float tx, float ty, float tz) : mat4x4()
	{
		matrix[0][0] = 1.0f;
		matrix[1][1] = 1.0f;
		matrix[2][2] = 1.0f;
		matrix[3][3] = 1.0f;
		matrix[3][0] = tx;
		matrix[3][1] = ty;
		matrix[3][2] = tz;

	}

};

struct ScalingMatrix : mat4x4
{

	ScalingMatrix(float sx, float sy, float sz) : mat4x4() 
	{
		matrix[0][0] = sx;
		matrix[1][1] = sy;
		matrix[2][2] = sz;
		matrix[3][3] = 1.0f;
	}

};

struct ProjectionMatrix : mat4x4
{
	ProjectionMatrix(float aspectRatio = 1.5f, float zNear = 0.1f, float zFar = 10000.0f, float fov = 90.0f) : mat4x4()
	{
		float fFoV_Rad = 1.0f / tanf(fov * 0.5f / 180.0f * (float)M_PI);
		matrix[0][0] = aspectRatio * fFoV_Rad;
		matrix[1][1] = fFoV_Rad;
		matrix[2][2] = zFar / (zFar - zNear);
		matrix[2][3] = 1.0f;
		matrix[3][2] = (-zFar * zNear) / (zFar - zNear);
		matrix[3][3] = 0.0f;
	}
};


struct xRotationMatrix : mat4x4
{
	
	xRotationMatrix(float rx) : mat4x4()
	{
		matrix[0][0] = 1.0f;
		matrix[1][1] = cosf(rx);
		matrix[1][2] = sinf(rx);
		matrix[2][1] = -sinf(rx);
		matrix[2][2] = cosf(rx);
		matrix[3][3] = 1.0f;
	}
};

struct yRotationMatrix : mat4x4
{
	yRotationMatrix(float ry) : mat4x4() 
	{
		matrix[0][0] = cosf(ry);
		matrix[0][2] = sinf(ry);
		matrix[1][1] = 1.0f;
		matrix[2][0] = -sinf(ry);
		matrix[2][2] = cosf(ry);
		matrix[3][3] = 1.0f;
	}
};

struct zRotationMatrix : mat4x4
{

	zRotationMatrix(float rz) : mat4x4() 
	{
		matrix[0][0] = cosf(rz);
		matrix[0][1] = sinf(rz);
		matrix[1][0] = -sinf(rz);
		matrix[1][1] = cosf(rz);
		matrix[2][2] = 1.0f;
		matrix[3][3] = 1.0f;
	}

};

struct PointAtMat : mat4x4
{
	PointAtMat(vector3D& position, vector3D& target, vector3D& _up) : mat4x4()
	{
		// Calculate new Forward vector
		vector3D forward = NormalizeVector(target - position);

		// Calculate new Up vector
		vector3D _temp = forward * DotProduct(_up, forward);
		vector3D up = NormalizeVector(_up - _temp);

		//Calculate new Right vector
		vector3D right = CrossProduct(forward, up);

		matrix[0][0] = right.x;    matrix[0][1] = right.y;    matrix[0][2] = right.z;
		matrix[1][0] = up.x;    matrix[1][1] = up.y;    matrix[1][2] = up.z;
		matrix[2][0] = forward.x;    matrix[2][1] = forward.y;    matrix[2][2] = forward.z;
		matrix[3][0] = position.x;    matrix[3][1] = position.y;    matrix[3][2] = position.z;    matrix[3][3] = 1.0f;

	}
};


static vector3D multiplyMatrixByVector(const vector3D &input, mat4x4 m)
{
	vector3D output;
	/*output.x = input.x * m.matrix[0][0] + input.y * m.matrix[0][1] + input.z * m.matrix[0][2] + input.w * m.matrix[0][3];
	output.y = input.x * m.matrix[1][0] + input.y * m.matrix[1][1] + input.z * m.matrix[1][2] + input.w * m.matrix[1][3];
	output.z = input.x * m.matrix[2][0] + input.y * m.matrix[2][1] + input.z * m.matrix[2][2] + input.w * m.matrix[2][3];
	output.w = input.x * m.matrix[3][0] + input.y * m.matrix[3][1] + input.z * m.matrix[3][2] + input.w * m.matrix[3][3];*/

	output.x = input.x * m.matrix[0][0] + input.y * m.matrix[1][0] + input.z * m.matrix[2][0] + input.w * m.matrix[3][0];
	output.y = input.x * m.matrix[0][1] + input.y * m.matrix[1][1] + input.z * m.matrix[2][1] + input.w * m.matrix[3][1];
	output.z = input.x * m.matrix[0][2] + input.y * m.matrix[1][2] + input.z * m.matrix[2][2] + input.w * m.matrix[3][2];
	output.w = input.x * m.matrix[0][3] + input.y * m.matrix[1][3] + input.z * m.matrix[2][3] + input.w * m.matrix[3][3];

	return output;
}

static mat4x4 multiplyMatrixByMatrix(const mat4x4& m1, const mat4x4& m2)
{
	mat4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			result.matrix[row][col] = m1.matrix[row][0] * m2.matrix[0][col]
				+ m1.matrix[row][1] * m2.matrix[1][col]
				+ m1.matrix[row][2] * m2.matrix[2][col]
				+ m1.matrix[row][3] * m2.matrix[3][col];
		}
	}

	return result;
}
static mat4x4 invertMatrix_special(mat4x4& m) // Only for rotation/translation matrices
{
	mat4x4 output;
	output.matrix[0][0] = m.matrix[0][0]; output.matrix[0][1] = m.matrix[1][0]; output.matrix[0][2] = m.matrix[2][0]; output.matrix[0][3] = m.matrix[3][0];
	output.matrix[1][0] = m.matrix[0][1]; output.matrix[1][1] = m.matrix[1][1]; output.matrix[1][2] = m.matrix[2][1]; output.matrix[1][3] = m.matrix[3][1];
	output.matrix[2][0] = m.matrix[0][2]; output.matrix[2][1] = m.matrix[1][2]; output.matrix[2][2] = m.matrix[2][2]; output.matrix[2][3] = m.matrix[3][2];
	output.matrix[3][0] = -(m.matrix[3][0] * output.matrix[0][0] + m.matrix[3][1] * output.matrix[1][0] + m.matrix[3][2] * output.matrix[2][0]);
	output.matrix[3][1] = -(m.matrix[3][0] * output.matrix[0][1] + m.matrix[3][1] * output.matrix[1][1] + m.matrix[3][2] * output.matrix[2][1]);
	output.matrix[3][2] = -(m.matrix[3][0] * output.matrix[0][2] + m.matrix[3][1] * output.matrix[1][2] + m.matrix[3][2] * output.matrix[2][2]);
	output.matrix[3][3] = 1.0f;

	return output;
}

static mat4x4 invertMatrix(mat4x4& m)
{
	double  det;


	mat4x4 matInv;

	matInv.matrix[0][0] = m.matrix[1][1] * m.matrix[2][2] * m.matrix[3][3] - m.matrix[1][1] * m.matrix[2][3] * m.matrix[3][2] - m.matrix[2][1] * m.matrix[1][2] * m.matrix[3][3] + m.matrix[2][1] * m.matrix[1][3] * m.matrix[3][2] + m.matrix[3][1] * m.matrix[1][2] * m.matrix[2][3] - m.matrix[3][1] * m.matrix[1][3] * m.matrix[2][2];
	matInv.matrix[1][0] = -m.matrix[1][0] * m.matrix[2][2] * m.matrix[3][3] + m.matrix[1][0] * m.matrix[2][3] * m.matrix[3][2] + m.matrix[2][0] * m.matrix[1][2] * m.matrix[3][3] - m.matrix[2][0] * m.matrix[1][3] * m.matrix[3][2] - m.matrix[3][0] * m.matrix[1][2] * m.matrix[2][3] + m.matrix[3][0] * m.matrix[1][3] * m.matrix[2][2];
	matInv.matrix[2][0] = m.matrix[1][0] * m.matrix[2][1] * m.matrix[3][3] - m.matrix[1][0] * m.matrix[2][3] * m.matrix[3][1] - m.matrix[2][0] * m.matrix[1][1] * m.matrix[3][3] + m.matrix[2][0] * m.matrix[1][3] * m.matrix[3][1] + m.matrix[3][0] * m.matrix[1][1] * m.matrix[2][3] - m.matrix[3][0] * m.matrix[1][3] * m.matrix[2][1];
	matInv.matrix[3][0] = -m.matrix[1][0] * m.matrix[2][1] * m.matrix[3][2] + m.matrix[1][0] * m.matrix[2][2] * m.matrix[3][1] + m.matrix[2][0] * m.matrix[1][1] * m.matrix[3][2] - m.matrix[2][0] * m.matrix[1][2] * m.matrix[3][1] - m.matrix[3][0] * m.matrix[1][1] * m.matrix[2][2] + m.matrix[3][0] * m.matrix[1][2] * m.matrix[2][1];
	matInv.matrix[0][1] = -m.matrix[0][1] * m.matrix[2][2] * m.matrix[3][3] + m.matrix[0][1] * m.matrix[2][3] * m.matrix[3][2] + m.matrix[2][1] * m.matrix[0][2] * m.matrix[3][3] - m.matrix[2][1] * m.matrix[0][3] * m.matrix[3][2] - m.matrix[3][1] * m.matrix[0][2] * m.matrix[2][3] + m.matrix[3][1] * m.matrix[0][3] * m.matrix[2][2];
	matInv.matrix[1][1] = m.matrix[0][0] * m.matrix[2][2] * m.matrix[3][3] - m.matrix[0][0] * m.matrix[2][3] * m.matrix[3][2] - m.matrix[2][0] * m.matrix[0][2] * m.matrix[3][3] + m.matrix[2][0] * m.matrix[0][3] * m.matrix[3][2] + m.matrix[3][0] * m.matrix[0][2] * m.matrix[2][3] - m.matrix[3][0] * m.matrix[0][3] * m.matrix[2][2];
	matInv.matrix[2][1] = -m.matrix[0][0] * m.matrix[2][1] * m.matrix[3][3] + m.matrix[0][0] * m.matrix[2][3] * m.matrix[3][1] + m.matrix[2][0] * m.matrix[0][1] * m.matrix[3][3] - m.matrix[2][0] * m.matrix[0][3] * m.matrix[3][1] - m.matrix[3][0] * m.matrix[0][1] * m.matrix[2][3] + m.matrix[3][0] * m.matrix[0][3] * m.matrix[2][1];
	matInv.matrix[3][1] = m.matrix[0][0] * m.matrix[2][1] * m.matrix[3][2] - m.matrix[0][0] * m.matrix[2][2] * m.matrix[3][1] - m.matrix[2][0] * m.matrix[0][1] * m.matrix[3][2] + m.matrix[2][0] * m.matrix[0][2] * m.matrix[3][1] + m.matrix[3][0] * m.matrix[0][1] * m.matrix[2][2] - m.matrix[3][0] * m.matrix[0][2] * m.matrix[2][1];
	matInv.matrix[0][2] = m.matrix[0][1] * m.matrix[1][2] * m.matrix[3][3] - m.matrix[0][1] * m.matrix[1][3] * m.matrix[3][2] - m.matrix[1][1] * m.matrix[0][2] * m.matrix[3][3] + m.matrix[1][1] * m.matrix[0][3] * m.matrix[3][2] + m.matrix[3][1] * m.matrix[0][2] * m.matrix[1][3] - m.matrix[3][1] * m.matrix[0][3] * m.matrix[1][2];
	matInv.matrix[1][2] = -m.matrix[0][0] * m.matrix[1][2] * m.matrix[3][3] + m.matrix[0][0] * m.matrix[1][3] * m.matrix[3][2] + m.matrix[1][0] * m.matrix[0][2] * m.matrix[3][3] - m.matrix[1][0] * m.matrix[0][3] * m.matrix[3][2] - m.matrix[3][0] * m.matrix[0][2] * m.matrix[1][3] + m.matrix[3][0] * m.matrix[0][3] * m.matrix[1][2];
	matInv.matrix[2][2] = m.matrix[0][0] * m.matrix[1][1] * m.matrix[3][3] - m.matrix[0][0] * m.matrix[1][3] * m.matrix[3][1] - m.matrix[1][0] * m.matrix[0][1] * m.matrix[3][3] + m.matrix[1][0] * m.matrix[0][3] * m.matrix[3][1] + m.matrix[3][0] * m.matrix[0][1] * m.matrix[1][3] - m.matrix[3][0] * m.matrix[0][3] * m.matrix[1][1];
	matInv.matrix[3][2] = -m.matrix[0][0] * m.matrix[1][1] * m.matrix[3][2] + m.matrix[0][0] * m.matrix[1][2] * m.matrix[3][1] + m.matrix[1][0] * m.matrix[0][1] * m.matrix[3][2] - m.matrix[1][0] * m.matrix[0][2] * m.matrix[3][1] - m.matrix[3][0] * m.matrix[0][1] * m.matrix[1][2] + m.matrix[3][0] * m.matrix[0][2] * m.matrix[1][1];
	matInv.matrix[0][3] = -m.matrix[0][1] * m.matrix[1][2] * m.matrix[2][3] + m.matrix[0][1] * m.matrix[1][3] * m.matrix[2][2] + m.matrix[1][1] * m.matrix[0][2] * m.matrix[2][3] - m.matrix[1][1] * m.matrix[0][3] * m.matrix[2][2] - m.matrix[2][1] * m.matrix[0][2] * m.matrix[1][3] + m.matrix[2][1] * m.matrix[0][3] * m.matrix[1][2];
	matInv.matrix[1][3] = m.matrix[0][0] * m.matrix[1][2] * m.matrix[2][3] - m.matrix[0][0] * m.matrix[1][3] * m.matrix[2][2] - m.matrix[1][0] * m.matrix[0][2] * m.matrix[2][3] + m.matrix[1][0] * m.matrix[0][3] * m.matrix[2][2] + m.matrix[2][0] * m.matrix[0][2] * m.matrix[1][3] - m.matrix[2][0] * m.matrix[0][3] * m.matrix[1][2];
	matInv.matrix[2][3] = -m.matrix[0][0] * m.matrix[1][1] * m.matrix[2][3] + m.matrix[0][0] * m.matrix[1][3] * m.matrix[2][1] + m.matrix[1][0] * m.matrix[0][1] * m.matrix[2][3] - m.matrix[1][0] * m.matrix[0][3] * m.matrix[2][1] - m.matrix[2][0] * m.matrix[0][1] * m.matrix[1][3] + m.matrix[2][0] * m.matrix[0][3] * m.matrix[1][1];
	matInv.matrix[3][3] = m.matrix[0][0] * m.matrix[1][1] * m.matrix[2][2] - m.matrix[0][0] * m.matrix[1][2] * m.matrix[2][1] - m.matrix[1][0] * m.matrix[0][1] * m.matrix[2][2] + m.matrix[1][0] * m.matrix[0][2] * m.matrix[2][1] + m.matrix[2][0] * m.matrix[0][1] * m.matrix[1][2] - m.matrix[2][0] * m.matrix[0][2] * m.matrix[1][1];

	det = m.matrix[0][0] * matInv.matrix[0][0] + m.matrix[0][1] * matInv.matrix[1][0] + m.matrix[0][2] * matInv.matrix[2][0] + m.matrix[0][3] * matInv.matrix[3][0];
	//	if (det == 0) return false;

	det = 1.0 / det;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			matInv.matrix[i][j] *= (float)det;

	return matInv;
}