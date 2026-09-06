#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"

// ============================================================
// 移動可能ブロックギミック
//
// 「紅」など特定の漢字名には依存させず、
// 「接続中だけ押す・引くことができるブロック」として実装する。
// そのため、後で担当する漢字が変更されてもこのクラスは流用できる。
// ============================================================
class MovableBlockGimmick {
public:
	// モデル・初期位置・大きさを設定する
	void Initialize(
	    KamataEngine::Model* model,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale = {1.0f, 1.0f, 1.0f});

	// 毎フレーム更新
	// playerPosition : プレイヤーの現在位置
	// playerMoveDelta: プレイヤーがこのフレームで移動した量
	void Update(
	    const KamataEngine::Vector3& playerPosition,
	    const KamataEngine::Vector3& playerMoveDelta);

	// 描画
	void Draw(const KamataEngine::Camera& camera);

	// 接続開始 / 接続解除
	// 本番では「接続システム」側から呼び出してもらう想定
	void SetConnected(bool connected);

	// 指定位置へスナップし、その場に固定する
	void SnapAndLock(const KamataEngine::Vector3& position);

	// デモを初期状態に戻す
	void Reset(const KamataEngine::Vector3& position);

	// 壁など、ブロックが通り抜けてはいけないAABBを登録する
	void AddObstacle(const Collision::AABB& obstacle);

	// 登録済みの障害物をすべて消す
	void ClearObstacles();

	// 現在のブロックAABBを取得する
	Collision::AABB GetAABB() const;

	// 現在位置を取得する
	const KamataEngine::Vector3& GetPosition() const;

	// 接続中か
	bool IsConnected() const;

	// スイッチなどに固定済みか
	bool IsLocked() const;

private:
	// 通常状態の色に戻す
	void SetNormalColor();

	// 指定位置へ移動しても障害物に当たらないか調べる
	bool CanMoveTo(const KamataEngine::Vector3& position) const;

	// X・Zを別々に移動判定して壁沿いに滑りやすくする
	void TryMoveXZ(const KamataEngine::Vector3& move);

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;

	// cube.objは中心から±1程度なので、scaleを半径として扱う
	KamataEngine::Vector3 halfSize_ = {1.0f, 1.0f, 1.0f};

	// 接続中だけブロックを操作できる
	bool isConnected_ = false;

	// スイッチへ置いた後など、固定中は移動できない
	bool isLocked_ = false;

	// 10日制作のデモなので固定長配列で十分
	static constexpr int kMaxObstacles = 8;
	Collision::AABB obstacles_[kMaxObstacles]{};
	int obstacleCount_ = 0;
};
