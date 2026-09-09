#pragma once

#include <cstdint>

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
	bool IsTitlePage() const { return page_ == Page::kTitle; }

private:
	enum class Page {
		kTitle,
		kInstruction,
	};

private:
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Audio* audio_ = nullptr;
	uint32_t decisionSoundHandle_ = 0u;
	KamataEngine::Sprite* titleSprite_ = nullptr;
	KamataEngine::Sprite* instructionSprite_ = nullptr;

	Page page_ = Page::kTitle;
	SceneType requestedScene_ = SceneType::kNone;
};
