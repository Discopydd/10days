#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"
#include "../Gimmick/MovableBlockGimmick.h"
#include "../Gimmick/SwitchDoorGimmick.h"
#include "../Player/Player.h"

// ============================================================
// 1ステージ分のゲームシーン
//
// ステージ構成：
// ・中央の壁の向こう側にGOAL
// ・中央の通路には閉じたドア
// ・ドアの手前を3個の移動ブロックが塞いでいる
// ・右上のスイッチへ1個のブロックを置くとドアが開く
// ・残り2個のブロックをどかして通路を通り、GOALへ到達する
// ============================================================
class GameScene {
public:
	void Initialize();
	bool Update();
	void Draw();
	void Finalize();

	bool IsClear() const { return isClear_; }
	void ResetStage() { ResetGame(); }

private:
	// --------------------------------------------------------
	// 接続状態
	// --------------------------------------------------------
	enum class ConnectionState {
		kIdle,
		kShooting,
		kPulling,
		kConnected,
	};

	void InitializeTransform(
	    KamataEngine::WorldTransform& worldTransform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

	// --------------------------------------------------------
	// Player
	// --------------------------------------------------------
	KamataEngine::Vector3 GetPlayerInputMove() const;

	KamataEngine::Vector3 MovePlayerWithCollision(
	    const KamataEngine::Vector3& move,
	    int ignoreBlockIndex = -1);

	bool CanPlayerMoveTo(
	    const KamataEngine::Vector3& position,
	    int ignoreBlockIndex = -1) const;

	Collision::AABB GetPlayerAABBAt(
	    const KamataEngine::Vector3& position) const;

	int BuildPlayerObstacleList(
	    Collision::AABB* outObstacles,
	    int maxCount,
	    int ignoreBlockIndex = -1) const;

	// --------------------------------------------------------
	// Block
	// --------------------------------------------------------
	MovableBlockGimmick* GetActiveBlock();
	const MovableBlockGimmick* GetActiveBlock() const;

	int FindNearestConnectableBlock() const;

	KamataEngine::Vector3 MoveBlockWithCollision(
	    int blockIndex,
	    const KamataEngine::Vector3& move);

	void UpdateConnectedMovement();
	void ResolvePlayerBlockOverlap(int blockIndex);

	// --------------------------------------------------------
	// 接続・糸
	// --------------------------------------------------------
	void UpdateConnectionInput();
	void UpdateRopeShot();
	void UpdatePullTogether();

	float CalculateContactDistanceXZ(
	    const KamataEngine::Vector3& direction) const;

	void CancelConnection();
	void UpdateRope();

	void SetRopeTransform(
	    const KamataEngine::Vector3& start,
	    const KamataEngine::Vector3& end);

	// --------------------------------------------------------
	// ステージギミック
	// --------------------------------------------------------
	void UpdateSwitch();
	void UpdateGoal();
	void ResetGame();

private:
	KamataEngine::Input* input_ = nullptr;

	Player* player_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	// --------------------------------------------------------
	// モデル
	// --------------------------------------------------------
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* switchModel_ = nullptr;
	KamataEngine::Model* doorModel_ = nullptr;
	KamataEngine::Model* goalModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* wallModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;

	// --------------------------------------------------------
	// 移動ブロック3個
	// --------------------------------------------------------
	static constexpr int kBlockCount = 3;
	MovableBlockGimmick* movableBlocks_[kBlockCount]{};
	int activeBlockIndex_ = -1;

	// --------------------------------------------------------
	// スイッチ
	// --------------------------------------------------------
	KamataEngine::WorldTransform switchWorldTransform_;
	KamataEngine::ObjectColor switchColor_;
	Collision::AABB switchAABB_{};
	bool switchActivated_ = false;

	// --------------------------------------------------------
	// ドア
	// --------------------------------------------------------
	SwitchDoorGimmick* door_ = nullptr;

	// --------------------------------------------------------
	// GOAL
	// --------------------------------------------------------
	KamataEngine::WorldTransform goalWorldTransform_;
	KamataEngine::ObjectColor goalColor_;
	Collision::AABB goalAABB_{};
	bool isClear_ = false;

	// --------------------------------------------------------
	// 床
	// --------------------------------------------------------
	KamataEngine::WorldTransform floorWorldTransform_;
	KamataEngine::ObjectColor floorColor_;

	// --------------------------------------------------------
	// 外壁4枚 + 中央仕切り壁2枚
	// --------------------------------------------------------
	static constexpr int kWallCount = 6;
	KamataEngine::WorldTransform wallWorldTransforms_[kWallCount];
	Collision::AABB wallAABBs_[kWallCount]{};
	KamataEngine::ObjectColor wallColor_;

	// --------------------------------------------------------
	// 糸
	// --------------------------------------------------------
	KamataEngine::WorldTransform ropeWorldTransform_;
	KamataEngine::ObjectColor ropeColor_;
	ConnectionState connectionState_ = ConnectionState::kIdle;
	float ropeShootProgress_ = 0.0f;

	// --------------------------------------------------------
	// 定数
	// --------------------------------------------------------
	static constexpr int kMaxPlayerObstacles = 16;
	static constexpr float kConnectDistance = 5.0f;
	static constexpr float kDisconnectDistance = 7.0f;
	static constexpr float kRopeShootProgressPerFrame = 0.08f;
	static constexpr float kPullSpeedPerFrame = 0.08f;
	static constexpr float kPlayerBlockSkin = 0.03f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {0.0f, 0.6f, -7.0f};

	// 3個を縦に並べ、中央通路への進路を塞ぐ
	const KamataEngine::Vector3 kBlockStartPositions_[kBlockCount] = {
		{0.0f, 1.0f, -2.0f},
		{0.0f, 1.0f, 0.1f},
		{0.0f, 1.0f, 2.2f},
	};

	// 画面右上寄り。ドア手前なので、閉じていても到達可能
	const KamataEngine::Vector3 kSwitchPosition_ = {4.7f, 0.10f, 2.1f};
	const KamataEngine::Vector3 kSwitchScale_ = {1.35f, 0.10f, 1.35f};

	// 中央の仕切り壁にある通路を塞ぐドア
	const KamataEngine::Vector3 kDoorPosition_ = {0.0f, 1.5f, 4.0f};
	const KamataEngine::Vector3 kDoorScale_ = {1.4f, 1.5f, 0.45f};

	// 壁の向こう側にあるクリア地点
	const KamataEngine::Vector3 kGoalPosition_ = {0.0f, 0.10f, 7.6f};
	const KamataEngine::Vector3 kGoalScale_ = {1.8f, 0.10f, 1.25f};
};
