#pragma once

#include <KamataEngine.h>

#include "SceneType.h"

// ============================================================
// 一時的なタイトルシーン
//
// 1回目のSPACE : タイトル -> 操作説明
// 2回目のSPACE : 操作説明 -> GameScene
// ============================================================
class TitleScene {
public:
	void Initialize();
	bool Update();
	void Draw();
	void Finalize();

	SceneType GetRequestedScene() const;
	void Reset();

private:
	enum class Page {
		kTitle,
		kInstruction,
	};

	void InitializeTransform(
	    KamataEngine::WorldTransform& transform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

private:
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Audio* audio_ = nullptr;
	uint32_t decisionSoundHandle_ = 0;
	KamataEngine::Camera camera_;

	KamataEngine::Model* titleModel_ = nullptr;
	KamataEngine::Model* instructionModel_ = nullptr;

	KamataEngine::WorldTransform panelTransform_;
	KamataEngine::ObjectColor panelColor_;

	Page page_ = Page::kTitle;
	SceneType requestedScene_ = SceneType::kNone;
};
