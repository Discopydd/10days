#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"
#include "../Gimmick/MovableBlockGimmick.h"

// ============================================================
class GameScene {
public:
	// リソース・オブジェクトの初期化
	void Initialize();

	// 毎フレーム更新
	bool Update();

	// 3Dオブジェクト描画
	void Draw();

	// 終了処理
	void Finalize();

private:
	// --------------------------------------------------------
	// 接続状態
	// --------------------------------------------------------
	enum class ConnectionState {
		kIdle,      // 未接続
		kShooting,  // 糸を発射中
		kPulling,   // 糸が命中し、互いに引き寄せ中
		kConnected, // 接続完了
	};

	// WorldTransform初期化補助
	void InitializeTransform(
	    KamataEngine::WorldTransform& worldTransform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

	// 入力からプレイヤーの移動量を作る
	KamataEngine::Vector3 GetPlayerInputMove() const;

	// プレイヤーを当たり判定付きで移動する
	KamataEngine::Vector3 MovePlayerWithCollision(
	    const KamataEngine::Vector3& move,
	    bool ignoreGimmickCollision = false);

	// 指定位置へプレイヤーが移動できるか判定する
	bool CanPlayerMoveTo(
	    const KamataEngine::Vector3& position,
	    bool ignoreGimmickCollision) const;

	// 現在のプレイヤーAABBを取得する
	Collision::AABB GetPlayerAABBAt(
	    const KamataEngine::Vector3& position) const;

	// 接続中の押す／引く移動を更新する
	void UpdateConnectedMovement();

	// プレイヤーとギミックが重なった場合に押し戻して解消する
	void ResolvePlayerGimmickOverlap();

	// Eキーによる接続開始／解除
	void UpdateConnectionInput();

	// 糸の射出アニメーションを更新する
	void UpdateRopeShot();

	// 命中後、プレイヤーとギミックを互いに引き寄せる
	void UpdatePullTogether();

	// プレイヤーとブロックが接触する中心間距離を求める
	float CalculateContactDistanceXZ(
	    const KamataEngine::Vector3& direction) const;

	// 接続状態を解除して通常状態へ戻す
	void CancelConnection();

	// スイッチへ置かれたか判定する
	void UpdateSwitch();

	// 糸の見た目を更新する
	void UpdateRope();

	// 2点間へ糸モデルを配置する
	void SetRopeTransform(
	    const KamataEngine::Vector3& start,
	    const KamataEngine::Vector3& end);

	// RキーでGameScene全体を初期状態へ戻す
	void ResetGame();

private:
	KamataEngine::Input* input_ = nullptr;

	// 描画確認を確実にするため、オブジェクトごとにモデルを分ける
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* switchModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* wallModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;

	KamataEngine::Camera camera_;

	// --------------------------------------------------------
	// プレイヤー
	// --------------------------------------------------------
	KamataEngine::WorldTransform playerWorldTransform_;
	KamataEngine::ObjectColor playerColor_;
	const KamataEngine::Vector3 kPlayerHalfSize_ = {0.6f, 0.6f, 0.6f};

	// --------------------------------------------------------
	// 移動ブロックギミック
	// --------------------------------------------------------
	MovableBlockGimmick movableBlock_;

	// --------------------------------------------------------
	// スイッチ
	// --------------------------------------------------------
	KamataEngine::WorldTransform switchWorldTransform_;
	KamataEngine::ObjectColor switchColor_;
	Collision::AABB switchAABB_{};

	// --------------------------------------------------------
	// 床
	// --------------------------------------------------------
	KamataEngine::WorldTransform floorWorldTransform_;
	KamataEngine::ObjectColor floorColor_;

	// --------------------------------------------------------
	// 壁4枚
	// --------------------------------------------------------
	static constexpr int kWallCount = 4;
	KamataEngine::WorldTransform wallWorldTransforms_[kWallCount];
	Collision::AABB wallAABBs_[kWallCount]{};
	KamataEngine::ObjectColor wallColor_;

	// --------------------------------------------------------
	// 糸
	// --------------------------------------------------------
	KamataEngine::WorldTransform ropeWorldTransform_;
	KamataEngine::ObjectColor ropeColor_;

	ConnectionState connectionState_ = ConnectionState::kIdle;

	// 0.0～1.0 : 糸の射出進行度
	float ropeShootProgress_ = 0.0f;

	// --------------------------------------------------------
	// GameScene用定数
	// --------------------------------------------------------
	static constexpr float kPlayerSpeed = 0.10f;
	static constexpr float kConnectDistance = 5.0f;
	static constexpr float kDisconnectDistance = 7.0f;

	// 約12～13フレームでターゲットへ到達する
	static constexpr float kRopeShootProgressPerFrame = 0.08f;

	// 命中後、1フレームに各オブジェクトが近づく最大量
	static constexpr float kPullSpeedPerFrame = 0.08f;

	// プレイヤーとブロックを完全密着させず、
	// 浮動小数点誤差によるめり込みを防ぐための僅かな隙間
	static constexpr float kPlayerBlockSkin = 0.03f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {-4.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kBlockStartPosition_ = {0.0f, 1.0f, 0.0f};
	const KamataEngine::Vector3 kSwitchPosition_ = {4.0f, 0.10f, 0.0f};
	const KamataEngine::Vector3 kSwitchScale_ = {1.6f, 0.10f, 1.6f};
};
