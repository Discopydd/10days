#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"

// ============================================================
// プレイヤークラス
// ============================================================
class Player {
public:
	// 接続先に合わせて表示色を切り替える。
	enum class ConnectionColor {
		kNormal,
		kRed,
		kGreen,
		kPurple,
	};

	// プレイヤーを初期化する
	void Initialize(const KamataEngine::Vector3& position);

	// プレイヤーのモデルを解放する
	void Finalize();

	// プレイヤーを描画する
	void Draw(const KamataEngine::Camera& camera);

	// WASD入力から、このフレームの移動量を取得する
	KamataEngine::Vector3 GetInputMove() const;

	// --------------------------------------------------------
	// 当たり判定付きで移動する
	//
	// staticObstacles : 壁などの固定障害物
	// obstacleCount   : 固定障害物の数
	// dynamicObstacle : ブロックなどの動的障害物。不要ならnullptr
	//
	// 戻り値は「実際に移動できた量」。
	// 引く処理などで、壁に止められた分を考慮するために使用する。
	// --------------------------------------------------------
	KamataEngine::Vector3 MoveWithCollision(
	    const KamataEngine::Vector3& move,
	    const Collision::AABB* staticObstacles,
	    int obstacleCount,
	    const Collision::AABB* dynamicObstacle = nullptr);

	// 指定位置へ移動できるか判定する
	bool CanMoveTo(
	    const KamataEngine::Vector3& position,
	    const Collision::AABB* staticObstacles,
	    int obstacleCount,
	    const Collision::AABB* dynamicObstacle = nullptr) const;

	// プレイヤーを指定位置へ直接配置する
	void SetPosition(const KamataEngine::Vector3& position);

	// 初期位置へ戻す
	void Reset(const KamataEngine::Vector3& position);

	// 現在位置を取得する
	const KamataEngine::Vector3& GetPosition() const;

	// 当たり判定用の半サイズを取得する
	const KamataEngine::Vector3& GetHalfSize() const;

	// 現在のプレイヤーAABBを取得する
	Collision::AABB GetAABB() const;

	// 指定位置にプレイヤーがいる場合のAABBを取得する
	Collision::AABB GetAABBAt(const KamataEngine::Vector3& position) const;

	// 移動速度を変更する
	void SetMoveSpeed(float speed);

	// 接続先の色をプレイヤーへ反映する。
	void SetConnectionColor(ConnectionColor color);

private:
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 normalColor_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// プレイヤーの当たり判定サイズ
	KamataEngine::Vector3 halfSize_ = {0.6f, 0.6f, 0.6f};

	// 1フレームの移動速度
	float moveSpeed_ = 0.10f;
};
