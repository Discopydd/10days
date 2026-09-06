#pragma once

#include <KamataEngine.h>
#include <cmath>

// ============================================================
// 簡易コリジョン用の共通処理
// デモ用なのでAABBだけを扱う
// ============================================================
namespace Collision {

struct AABB {
	KamataEngine::Vector3 min;
	KamataEngine::Vector3 max;
};

// AABB同士が3次元で重なっているか判定する
inline bool IsOverlap(const AABB& a, const AABB& b) {
	return a.min.x < b.max.x && a.max.x > b.min.x &&
	       a.min.y < b.max.y && a.max.y > b.min.y &&
	       a.min.z < b.max.z && a.max.z > b.min.z;
}

// XZ平面だけで重なっているか判定する
// スイッチ判定では高さ(Y)を無視したいので使用する
inline bool IsOverlapXZ(const AABB& a, const AABB& b) {
	return a.min.x < b.max.x && a.max.x > b.min.x &&
	       a.min.z < b.max.z && a.max.z > b.min.z;
}

// 中心座標と半径からAABBを作成する
inline AABB MakeAABB(
    const KamataEngine::Vector3& center,
    const KamataEngine::Vector3& halfSize) {

	AABB result{};
	result.min = {
		center.x - halfSize.x,
		center.y - halfSize.y,
		center.z - halfSize.z,
	};
	result.max = {
		center.x + halfSize.x,
		center.y + halfSize.y,
		center.z + halfSize.z,
	};
	return result;
}

// 2点間の3次元距離を求める
inline float Distance(
    const KamataEngine::Vector3& a,
    const KamataEngine::Vector3& b) {

	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	const float dz = a.z - b.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

} // namespace Collision
