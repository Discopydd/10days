#pragma once

#include <KamataEngine.h>

#include <cmath>

// ============================================================
// 机关の頭上へ表示する、点滅する操作キー画像。
//
// 3D空間上の薄いパネルとして描画するため、カメラの角度や
// ウィンドウサイズに依存せず、対象の机关へ自然に追従する。
// ============================================================
class InteractionPrompt {
public:
	~InteractionPrompt() { Finalize(); }

	void Initialize() {
		Finalize();

		keyEModel_ = KamataEngine::Model::CreateFromOBJ("key_e");
		keyFModel_ = KamataEngine::Model::CreateFromOBJ("key_f");

		worldTransform_.Initialize();
		objectColor_.Initialize();
		ResetAnimation();
	}

	void Update() {
		blinkTime_ += kDeltaTime;
		if (blinkTime_ >= kBlinkPeriod) {
			blinkTime_ -= kBlinkPeriod;
		}
	}

	void DrawE(
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Camera& camera) {
		Draw(keyEModel_, position, kEAspectRatio, camera);
	}

	void DrawF(
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Camera& camera) {
		Draw(keyFModel_, position, kFAspectRatio, camera);
	}

	void ResetAnimation() { blinkTime_ = 0.0f; }

	void Finalize() {
		delete keyEModel_;
		delete keyFModel_;
		keyEModel_ = nullptr;
		keyFModel_ = nullptr;
	}

private:
	void Draw(
	    KamataEngine::Model* model,
	    const KamataEngine::Vector3& position,
	    float aspectRatio,
	    const KamataEngine::Camera& camera) {

		if (model == nullptr) {
			return;
		}

		const float phase = blinkTime_ / kBlinkPeriod * kTwoPi;
		const float pulse = 0.5f + std::sin(phase) * 0.5f;
		const float scale = kBaseScale * (0.90f + pulse * 0.20f);
		const float brightness = 0.85f + pulse * 0.45f;

		worldTransform_.translation_ = {
			position.x,
			position.y + pulse * kBobHeight,
			position.z,
		};
		// カメラと同じ回転を与え、画像の正面を常に画面へ向ける。
		worldTransform_.rotation_ = {
			camera.rotation_.x,
			camera.rotation_.y,
			0.0f,
		};
		worldTransform_.scale_ = {
			scale * aspectRatio,
			scale,
			kPanelDepth,
		};
		worldTransform_.UpdateMatarix();

		// RGBの明るさを周期的に変え、透過設定に依存しない点滅にする。
		objectColor_.SetColor({
			brightness,
			brightness,
			brightness,
			1.0f,
		});

		model->Draw(worldTransform_, camera, &objectColor_);
	}

private:
	KamataEngine::Model* keyEModel_ = nullptr;
	KamataEngine::Model* keyFModel_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;
	float blinkTime_ = 0.0f;

	static constexpr float kDeltaTime = 1.0f / 60.0f;
	static constexpr float kBlinkPeriod = 0.75f;
	static constexpr float kTwoPi = 6.28318530718f;
	static constexpr float kBaseScale = 0.50f;
	static constexpr float kPanelDepth = 0.035f;
	static constexpr float kBobHeight = 0.18f;
	static constexpr float kEAspectRatio = 764.0f / 716.0f;
	static constexpr float kFAspectRatio = 745.0f / 714.0f;
};
