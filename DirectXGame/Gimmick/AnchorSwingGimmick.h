#pragma once

#include <KamataEngine.h>

// ============================================================
// アンカースウィングギミック
//
// アンカーへの接続、距離制限、重力、補助加速、
// 接続解除後に速度を残すための簡易スウィング物理を管理する。
// ============================================================
class AnchorSwingGimmick {
public:
	struct Settings {
		float connectDistance = 8.0f;
		float ropeLength = 7.0f;
		float gravity = 19.6f;
		float swingAssist = 4.0f;
		float maxSpeed = 30.0f;
	};

	void Initialize(const Settings& settings = Settings{});

	bool Connect(
	    const KamataEngine::Vector3& anchorPosition,
	    const KamataEngine::Vector3& playerPosition);

	void Disconnect();

	bool ToggleConnection(
	    const KamataEngine::Vector3& anchorPosition,
	    const KamataEngine::Vector3& playerPosition);

	void Update(
	    KamataEngine::Vector3& playerPosition,
	    KamataEngine::Vector3& playerVelocity,
	    const KamataEngine::Vector3& playerMoveVelocity,
	    float deltaTime);

	bool IsConnected() const;
	const KamataEngine::Vector3& GetAnchorPosition() const;
	float GetRopeLength() const;

	void SetSettings(const Settings& settings);
	const Settings& GetSettings() const;

private:
	void ApplyRopeConstraint(
	    KamataEngine::Vector3& playerPosition,
	    KamataEngine::Vector3& playerVelocity) const;

private:
	Settings settings_{};
	KamataEngine::Vector3 anchorPosition_{};
	float activeRopeLength_ = 0.0f;
	bool isConnected_ = false;
};
