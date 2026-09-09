#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"
#include "../Common/InteractionPrompt.h"
#include "../Gimmick/AnchorSwingGimmick.h"
#include "../Gimmick/MovableBlockGimmick.h"
#include "../Gimmick/PowerGimmick.h"
#include "../Gimmick/SwitchDoorGimmick.h"
#include "../Player/Player.h"

// ============================================================
// 第3ステージ：電源 + ブロック解謎 → 最後にスウィング
//
// 第二关との差別化：
// ・開始地点は大きな連続床。最初に落下穴を越える必要はない。
// ・紫色の電源は実体のある障害物で、Player / Blockともに通り抜け不可。
// ・EでBlockを引き寄せてスイッチへ運ぶ。
// ・Fで紫色電源とドアを接続する。
// ・「スイッチON + 電源接続」の両条件で中央ゲートが開く。
// ・ゲートの奥へ進んだ後、最後に短い穴をアンカーでスウィングしてGOAL島へ渡る。
// ============================================================
class ThirdStageScene {
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
	void DrawInteractionPrompt();

	// Fキー：電源 <-> ドアの装置接続
	void UpdateDeviceConnectionInput();
	void UpdateDeviceRope();
	void SetDeviceRopeTransform(
	    const KamataEngine::Vector3& start,
	    const KamataEngine::Vector3& end);
	KamataEngine::Vector3 GetDoorCenter() const;
	void UpdateDoorActivation();

	// ゲート手前のブロック・スイッチ・ドア
	void UpdateSwitch();
	void PrepareBlockObstacles();
	KamataEngine::Vector3 MoveGroundPlayerWithCollision(
	    const KamataEngine::Vector3& move,
	    bool ignoreBlock = false);
	KamataEngine::Vector3 UpdateConnectedBlockMovement(
	    const KamataEngine::Vector3& desiredMove);
	KamataEngine::Vector3 UpdateBlockPullTogether();
	float CalculateBlockContactDistanceXZ(
	    const KamataEngine::Vector3& direction) const;
	void CancelBlockConnection();
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
	void UpdateSafeRespawnCheckpoint(const KamataEngine::Vector3& position);
	void ResetPlayerAfterFall();
	void ResetDemo();

private:
	KamataEngine::Input* input_ = nullptr;

	Player* player_ = nullptr;
	AnchorSwingGimmick* anchorSwing_ = nullptr;
	MovableBlockGimmick* movableBlock_ = nullptr;
	SwitchDoorGimmick* door_ = nullptr;
	PowerGimmick* power_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Model* anchorModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* wallModel_ = nullptr;
	KamataEngine::Model* goalModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* switchModel_ = nullptr;
	KamataEngine::Model* doorModel_ = nullptr;
	KamataEngine::Model* powerModel_ = nullptr;

	KamataEngine::WorldTransform anchorTransform_;

	static constexpr int kFloorCount = 2;
	KamataEngine::WorldTransform floorTransforms_[kFloorCount];
	Collision::AABB floorAABBs_[kFloorCount]{};

	// 0～3: 外周 / 4～5: 中央ゲートの仕切り壁
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
	KamataEngine::WorldTransform deviceRopeTransform_;

	KamataEngine::ObjectColor anchorColor_;
	KamataEngine::ObjectColor floorColor_;
	KamataEngine::ObjectColor wallColor_;
	KamataEngine::ObjectColor goalColor_;
	KamataEngine::ObjectColor ropeColor_;
	KamataEngine::ObjectColor blockRopeColor_;
	KamataEngine::ObjectColor deviceRopeColor_;

	KamataEngine::Vector3 playerVelocity_{};
	KamataEngine::Vector3 safeRespawnPosition_{};
	bool blockPulling_ = false;

	enum class DeviceType {
		kNone,
		kPower,
		kDoor,
	};
	bool deviceSelecting_ = false;
	bool deviceConnected_ = false;
	bool doorActivationApplied_ = false;
	DeviceType selectedDeviceType_ = DeviceType::kNone;
	InteractionPrompt interactionPrompt_;

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
	static constexpr float kBlockPullSpeedPerFrame = 0.08f;
	static constexpr float kBlockContactSkin = 0.03f;
	static constexpr float kSwitchSnapCenterTolerance = 0.70f;
	static constexpr float kSwitchSnapPlayerClearance = 0.25f;
	static constexpr float kDeviceConnectDistance = 3.4f;

	static constexpr float kStageMinX = -11.5f;
	static constexpr float kStageMaxX = 13.0f;
	static constexpr float kStageMinZ = -5.6f;
	static constexpr float kStageMaxZ = 5.6f;
	static constexpr float kStageMaxY = 12.0f;
	static constexpr float kFallResetY = -6.0f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {-9.2f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kLeftRespawnPosition_ = {-9.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kRightRespawnPosition_ = {9.4f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kAnchorPosition_ = {6.0f, 6.5f, 0.0f};

	// ゲート手前の大平台に配置するギミック
	const KamataEngine::Vector3 kBlockStartPosition_ = {-8.0f, 0.8f, 3.1f};
	const KamataEngine::Vector3 kBlockScale_ = {0.8f, 0.8f, 0.8f};
	const KamataEngine::Vector3 kSwitchPosition_ = {-2.6f, 0.10f, -3.1f};
	const KamataEngine::Vector3 kSwitchScale_ = {1.15f, 0.10f, 1.15f};
	const KamataEngine::Vector3 kDoorPosition_ = {1.5f, 1.5f, 0.0f};
	const KamataEngine::Vector3 kDoorScale_ = {0.35f, 1.5f, 1.20f};

	// 第3关新增：紫色电源。Fで電源とドアを順番に選択して接続する。
	const KamataEngine::Vector3 kPowerPosition_ = {-5.1f, 0.75f, 0.2f};
	const KamataEngine::Vector3 kPowerScale_ = {0.70f, 0.70f, 0.70f};

	// 最後のスウィング先にあるGOAL島
	const KamataEngine::Vector3 kGoalPosition_ = {10.4f, 0.10f, 0.0f};
	const KamataEngine::Vector3 kGoalScale_ = {1.00f, 0.10f, 1.20f};
	const KamataEngine::Vector3 kGoalTriggerHalfSize_ = {0.80f, 0.80f, 1.00f};
};
