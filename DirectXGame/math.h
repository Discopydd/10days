#pragma once

#include <cmath>

#include <math/Matrix4x4.h>
#include <math/Vector3.h>

namespace KamataEngine {

// ============================================================
// 4x4行列の乗算
// ============================================================
inline Matrix4x4 Multiply(const Matrix4x4& matrix1, const Matrix4x4& matrix2) {
	Matrix4x4 result{};

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			for (int k = 0; k < 4; ++k) {
				result.m[row][column] +=
					matrix1.m[row][k] * matrix2.m[k][column];
			}
		}
	}

	return result;
}

// ============================================================
// 拡大縮小行列を作る
// ============================================================
inline Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result{};

	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;

	return result;
}

// ============================================================
// X軸回転行列を作る
// ============================================================
inline Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result{};

	const float cosValue = std::cos(radian);
	const float sinValue = std::sin(radian);

	result.m[0][0] = 1.0f;
	result.m[1][1] = cosValue;
	result.m[1][2] = sinValue;
	result.m[2][1] = -sinValue;
	result.m[2][2] = cosValue;
	result.m[3][3] = 1.0f;

	return result;
}

// ============================================================
// Y軸回転行列を作る
// ============================================================
inline Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result{};

	const float cosValue = std::cos(radian);
	const float sinValue = std::sin(radian);

	result.m[0][0] = cosValue;
	result.m[0][2] = -sinValue;
	result.m[1][1] = 1.0f;
	result.m[2][0] = sinValue;
	result.m[2][2] = cosValue;
	result.m[3][3] = 1.0f;

	return result;
}

// ============================================================
// Z軸回転行列を作る
// ============================================================
inline Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result{};

	const float cosValue = std::cos(radian);
	const float sinValue = std::sin(radian);

	result.m[0][0] = cosValue;
	result.m[0][1] = sinValue;
	result.m[1][0] = -sinValue;
	result.m[1][1] = cosValue;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	return result;
}

// ============================================================
// 平行移動行列を作る
// ============================================================
inline Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}

// ============================================================
// スケール・回転・平行移動を合成してアフィン行列を作る
//
// このエンジンのシェーダーは mul(position, world) の形で
// 行ベクトルとして座標を変換するため、
// Scale -> RotateX -> RotateY -> RotateZ -> Translate の順で合成する。
// ============================================================
inline Matrix4x4 MakeAffineMatrix(
	const Vector3& scale,
	const Vector3& rotation,
	const Vector3& translation) {

	const Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	const Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotation.x);
	const Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotation.y);
	const Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotation.z);
	const Matrix4x4 translateMatrix = MakeTranslateMatrix(translation);

	Matrix4x4 result = Multiply(scaleMatrix, rotateXMatrix);
	result = Multiply(result, rotateYMatrix);
	result = Multiply(result, rotateZMatrix);
	result = Multiply(result, translateMatrix);

	return result;
}

} // namespace KamataEngine
