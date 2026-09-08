#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"
#include "../Gimmick/AnchorSwingGimmick.h"
#include "../Gimmick/MovableBlockGimmick.h"
#include "../Gimmick/SwitchDoorGimmick.h"
#include "../Player/Player.h"

// ============================================================
// スウィング + ブロックギミックステージ
//
// 前半：アンカーに接続して大きな穴をスウィングで越える。
// 後半：右側足場でブロックをEで接続し、スイッチまで運ぶ。
//       スイッチ作動でドアが開き、奥のGOALへ到達するとクリア。
// ============================================================
class SwingDemoScene {
public:
	void Initialize();
	bool Update();
	void Draw();
	void Finalize();

	bool IsClear() const { return isClear_; }
	void ResetStage();

private:
	void InitializeTransform(
	    KamataEngine::WorldTransform& transform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

	KamataEngine::Vector3 GetPlayerMoveVelocity() const;

	// Eキーの状況別接続処理（アンカー / ブロック）
	void UpdateConnection();
	void UpdatePlayer();
	void UpdateRope();
	void UpdateBlockRope();
	void UpdateAnchorColor();

	// 右側足場のブロック・スイッチ・ドア
	void UpdateSwitch();
	void PrepareBlockObstacles();
	KamataEngine::Vector3 MoveGroundPlayerWithCollision(
	    const KamataEngine::Vector3& move,
	    bool ignoreBlock = false);
	KamataEngine::Vector3 UpdateConnectedBlockMovement(
	    const KamataEngine::Vector3& desiredMove);
	bool IsPlayerGrounded() const;
	bool HasGateBarrierCollision(const KamataEngine::Vector3& position) const;
	void ResolveGateBarrierCollision(
	    const KamataEngine::Vector3& previousPosition,
	    KamataEngine::Vector3& position);

	void UpdateGoal();
	void ApplyStageBounds(KamataEngine::Vector3& position);
	bool HasFloorSupportXZ(
	    const KamataEngine::Vector3& position,
	    const Collision::AABB& floorAABB) const;
	bool GetStandingY(
	    const KamataEngine::Vector3& position,
	    float& standingY) const;
	bool HasBlockSupportXZ(
	    const KamataEngine::Vector3& position) const;
	bool ResolveBlockCollision(
	    const KamataEngine::Vector3& previousPosition,
	    KamataEngine::Vector3& position);
	bool ResolveFloorLanding(
	    const KamataEngine::Vector3& previousPosition,
	    KamataEngine::Vector3& position);
	bool IsPlayerInsideGoal() const;
	bool IsSafeRespawnPosition(
	    const KamataEngine::Vector3& position,
	    int floorIndex) const;
	KamataEngine::Vector3 FindSafeRespawnPosition() const;
	void ResetPlayerAfterFall();
	void ResetDemo();

private:
	KamataEngine::Input* input_ = nullptr;

	Player* player_ = nullptr;
	AnchorSwingGimmick* anchorSwing_ = nullptr;
	MovableBlockGimmick* movableBlock_ = nullptr;
	SwitchDoorGimmick* door_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Model* anchorModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* wallModel_ = nullptr;
	KamataEngine::Model* goalModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* switchModel_ = nullptr;
	KamataEngine::Model* doorModel_ = nullptr;

	KamataEngine::WorldTransform anchorTransform_;

	static constexpr int kFloorCount = 2;
	KamataEngine::WorldTransform floorTransforms_[kFloorCount];
	Collision::AABB floorAABBs_[kFloorCount]{};

	// 0～3: 外周 / 4～5: 右足場の仕切り壁
	static constexpr int kWallCount = 6;
	KamataEngine::WorldTransform wallTransforms_[kWallCount];
	Collision::AABB wallAABBs_[kWallCount]{};

	KamataEngine::WorldTransform switchTransform_;
	Collision::AABB switchAABB_{};
	KamataEngine::ObjectColor switchColor_;
	bool switchActivated_ = false;

	KamataEngine::WorldTransform goalTransform_;
	Collision::AABB goalAABB_{};

	KamataEngine::WorldTransform ropeTransform_;
	KamataEngine::WorldTransform blockRopeTransform_;

	KamataEngine::ObjectColor anchorColor_;
	KamataEngine::ObjectColor floorColor_;
	KamataEngine::ObjectColor wallColor_;
	KamataEngine::ObjectColor goalColor_;
	KamataEngine::ObjectColor ropeColor_;
	KamataEngine::ObjectColor blockRopeColor_;

	KamataEngine::Vector3 playerVelocity_{};
	KamataEngine::Vector3 safeRespawnPosition_{};
	bool isClear_ = false;

	static constexpr float kDeltaTime = 1.0f / 60.0f;
	static constexpr float kGravity = 16.0f;
	static constexpr float kAirControlAcceleration = 5.0f;
	static constexpr float kAirDamping = 0.10f;
	static constexpr float kMaxHorizontalSpeed = 12.0f;
	static constexpr float kGroundTolerance = 0.05f;
	static constexpr float kMinFloorSupport = 0.18f;

	static constexpr float kBlockConnectDistance = 3.2f;
	static constexpr float kBlockDisconnectDistance = 4.8f;

	static constexpr float kStageMinX = -10.4f;
	static constexpr float kStageMaxX = 10.4f;
	static constexpr float kStageMinZ = -4.4f;
	static constexpr float kStageMaxZ = 4.4f;
	static constexpr float kStageMaxY = 12.0f;
	static constexpr float kFallResetY = -6.0f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {-8.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kLeftRespawnPosition_ = {-7.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kRightRespawnPosition_ = {6.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kAnchorPosition_ = {0.0f, 6.5f, 0.0f};

	// 右側足場のギミック配置
	const KamataEngine::Vector3 kBlockStartPosition_ = {5.3f, 0.8f, 2.5f};
	const KamataEngine::Vector3 kBlockScale_ = {0.8f, 0.8f, 0.8f};
	const KamataEngine::Vector3 kSwitchPosition_ = {6.6f, 0.10f, -2.5f};
	const KamataEngine::Vector3 kSwitchScale_ = {1.15f, 0.10f, 1.15f};
	const KamataEngine::Vector3 kDoorPosition_ = {8.3f, 1.5f, 0.0f};
	const KamataEngine::Vector3 kDoorScale_ = {0.35f, 1.5f, 1.0f};

	// ドアの奥にある地面GOAL
	const KamataEngine::Vector3 kGoalPosition_ = {9.65f, 0.10f, 0.0f};
	const KamataEngine::Vector3 kGoalScale_ = {0.80f, 0.10f, 1.10f};
	const KamataEngine::Vector3 kGoalTriggerHalfSize_ = {0.70f, 0.80f, 0.90f};
};
