#pragma once

#include <KamataEngine.h>

// アンカー接続中のプレイヤーへ、簡易的な振り子運動を適用する。
// このクラスは位置と速度だけを扱うため、プレイヤークラスへ依存しない。
class AnchorSwingGimmick {
public:
	struct Settings {
		// この距離以内にいるときだけ接続できる
		float connectDistance = 8.0f;

		// 接続中に許されるアンカーからの最大距離
		float ropeLength = 7.0f;

		// Y軸下向きに加える加速度（1秒あたり）
		float gravity = 19.6f;

		// 入力速度を接線方向の加速度へ変換する強さ
		float swingAssist = 4.0f;

		// 異常に大きな速度を防ぐ上限。0以下なら制限しない
		float maxSpeed = 30.0f;
	};

	void Initialize(const Settings& settings = Settings{});

	// 範囲内ならアンカーへ接続する。接続に成功した場合はtrue
	bool Connect(
	    const KamataEngine::Vector3& anchorPosition,
	    const KamataEngine::Vector3& playerPosition);

	// 速度は変更せず接続だけを解除する
	void Disconnect();

	// 接続状態を切り替える。接続できなかった場合はfalse
	bool ToggleConnection(
	    const KamataEngine::Vector3& anchorPosition,
	    const KamataEngine::Vector3& playerPosition);

	// プレイヤーの位置・速度を1フレーム進める。
	// playerMoveVelocityには通常移動で作った速度（WASD等）を渡す。
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

	Settings settings_{};
	KamataEngine::Vector3 anchorPosition_{};
	float activeRopeLength_ = 0.0f;
	bool isConnected_ = false;
};
