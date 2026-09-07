#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"
#include "../Gimmick/AnchorSwingGimmick.h"
#include "../Player/Player.h"

// ============================================================
// スウィングステージ
//
// ・アンカーへの接続処理
// ・アンカーとプレイヤーの距離制限
// ・簡易スウィング物理
// ・重力
// ・プレイヤー移動速度を利用した補助加速
// ・接続解除後の速度維持
// ・ステージ範囲制限
// ・GOAL到達によるクリア判定
// を確認するシーン。
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
	void UpdateConnection();
	void UpdatePlayer();
	void UpdateRope();
	void UpdateAnchorColor();
	void UpdateGoal();
	void ApplyStageBounds(KamataEngine::Vector3& position);
	bool HasFloorSupportXZ(
	    const KamataEngine::Vector3& position,
	    const Collision::AABB& floorAABB) const;
	bool GetStandingY(
	    const KamataEngine::Vector3& position,
	    float& standingY) const;
	bool ResolveFloorLanding(
	    const KamataEngine::Vector3& previousPosition,
	    KamataEngine::Vector3& position);
	bool IsPlayerInsideGoal() const;
	void ResetPlayerAfterFall(const KamataEngine::Vector3& fallPosition);
	void ResetDemo();

private:
	KamataEngine::Input* input_ = nullptr;

	Player* player_ = nullptr;
	AnchorSwingGimmick* anchorSwing_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Model* anchorModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* wallModel_ = nullptr;
	KamataEngine::Model* goalModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;

	KamataEngine::WorldTransform anchorTransform_;

	static constexpr int kFloorCount = 2;
	KamataEngine::WorldTransform floorTransforms_[kFloorCount];
	Collision::AABB floorAABBs_[kFloorCount]{};

	static constexpr int kWallCount = 4;
	KamataEngine::WorldTransform wallTransforms_[kWallCount];

	KamataEngine::WorldTransform goalTransform_;
	Collision::AABB goalAABB_{};

	KamataEngine::WorldTransform ropeTransform_;

	KamataEngine::ObjectColor anchorColor_;
	KamataEngine::ObjectColor floorColor_;
	KamataEngine::ObjectColor wallColor_;
	KamataEngine::ObjectColor goalColor_;
	KamataEngine::ObjectColor ropeColor_;

	KamataEngine::Vector3 playerVelocity_{};
	bool isClear_ = false;

	static constexpr float kDeltaTime = 1.0f / 60.0f;
	static constexpr float kGravity = 16.0f;
	static constexpr float kAirControlAcceleration = 5.0f;
	static constexpr float kAirDamping = 0.10f;
	static constexpr float kMaxHorizontalSpeed = 12.0f;
	static constexpr float kGroundTolerance = 0.05f;
	static constexpr float kMinFloorSupport = 0.18f;

	static constexpr float kStageMinX = -10.4f;
	static constexpr float kStageMaxX = 10.4f;
	static constexpr float kStageMinZ = -4.4f;
	static constexpr float kStageMaxZ = 4.4f;
	static constexpr float kStageMaxY = 12.0f;
	static constexpr float kFallResetY = -6.0f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {-8.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kLeftRespawnPosition_ = {-4.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kRightRespawnPosition_ = {4.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kAnchorPosition_ = {0.0f, 6.5f, 0.0f};
	const KamataEngine::Vector3 kGoalPosition_ = {7.0f, 2.2f, 0.0f};
	const KamataEngine::Vector3 kGoalScale_ = {1.1f, 1.0f, 1.2f};
	const KamataEngine::Vector3 kGoalTriggerHalfSize_ = {0.75f, 0.65f, 0.85f};
};
