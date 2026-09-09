#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"
#include "../Gimmick/SwitchDoorGimmick.h"
#include "../Gimmick/PowerGimmick.h"
#include "../Player/Player.h"

// ============================================================
// 1ステージ分のゲームシーン（スイッチと移動ブロックを削除済）
// ============================================================
class GameScene {
public:
	void Initialize();
	bool Update();
	void Draw();
	void Finalize();

private:
	void InitializeTransform(
	    KamataEngine::WorldTransform& worldTransform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

	// --------------------------------------------------------
	// Player
	// --------------------------------------------------------
	KamataEngine::Vector3 GetPlayerInputMove() const;

	KamataEngine::Vector3 MovePlayerWithCollision(
	    const KamataEngine::Vector3& move);

	bool CanPlayerMoveTo(
	    const KamataEngine::Vector3& position) const;

	Collision::AABB GetPlayerAABBAt(
	    const KamataEngine::Vector3& position) const;

	int BuildPlayerObstacleList(
	    Collision::AABB* outObstacles,
	    int maxCount) const;

	// --------------------------------------------------------
	// デバイス間接続（Power <-> Device）
	// --------------------------------------------------------
	void UpdateDeviceConnectionInput(); // Fキーで電源/装置を選択して接続
	void UpdateDeviceRope();
	void SetDeviceRopeTransform(const KamataEngine::Vector3& start, const KamataEngine::Vector3& end);

	// --------------------------------------------------------
	// ステージギミック
	// --------------------------------------------------------
	void UpdateGoal();
	void ResetGame();

private:
	KamataEngine::Input* input_ = nullptr;

	Player* player_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	// --------------------------------------------------------
	// モデル
	// --------------------------------------------------------
	KamataEngine::Model* switchModel_ = nullptr; // 電源の見た目に使う
	KamataEngine::Model* doorModel_ = nullptr;
	KamataEngine::Model* goalModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* wallModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;

	// --------------------------------------------------------
	// ドア
	// --------------------------------------------------------
	SwitchDoorGimmick* door_ = nullptr;

	// --------------------------------------------------------
	// 電源
	// --------------------------------------------------------
	PowerGimmick* power_ = nullptr;
	// 電源の設置位置（ステージ定数）
	const KamataEngine::Vector3 kPowerPosition_ = {-5.0f, 1.0f, -4.0f};
	const KamataEngine::Vector3 kPowerScale_ = {0.8f, 0.8f, 0.8f};

	// デバイス接続状態
	bool deviceSelecting_ = false; // 1つ目選択済みか
	bool deviceConnected_ = false; // 接続確立済みか
	enum class DeviceType { None, Power, Door };
	DeviceType selectedDeviceType_ = DeviceType::None;

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

	// デバイス接続用の糸（Power <-> Device）
	KamataEngine::WorldTransform deviceRopeWorldTransform_;
	KamataEngine::ObjectColor deviceRopeColor_;

	// --------------------------------------------------------
	// 定数
	// --------------------------------------------------------
	static constexpr int kMaxPlayerObstacles = 16;
	static constexpr float kConnectDistance = 5.0f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {0.0f, 0.6f, -7.0f};

	// 中央の仕切り壁にある通路を塞ぐドア
	const KamataEngine::Vector3 kDoorPosition_ = {0.0f, 1.5f, 4.0f};
	const KamataEngine::Vector3 kDoorScale_ = {1.4f, 1.5f, 0.45f};

	// 壁の向こう側にあるクリア地点
	const KamataEngine::Vector3 kGoalPosition_ = {0.0f, 0.10f, 7.6f};
	const KamataEngine::Vector3 kGoalScale_ = {1.8f, 0.10f, 1.25f};
};
