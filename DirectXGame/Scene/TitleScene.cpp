#include "TitleScene.h"

using namespace KamataEngine;

void TitleScene::Initialize() {
	input_ = Input::GetInstance();

	const uint32_t titleTextureHandle =
		TextureManager::Load("title/title.png");
	const uint32_t instructionTextureHandle =
		TextureManager::Load("instruction/instruction.png");

	titleSprite_ = Sprite::Create(titleTextureHandle, {0.0f, 0.0f});
	instructionSprite_ =
		Sprite::Create(instructionTextureHandle, {0.0f, 0.0f});

	// 画像をウィンドウ全体に描画する。
	if (titleSprite_ != nullptr) {
		titleSprite_->SetSize({1280.0f, 720.0f});
	}
	if (instructionSprite_ != nullptr) {
		instructionSprite_->SetSize({1280.0f, 720.0f});
	}

	Reset();
}

bool TitleScene::Update() {
	if (input_ == nullptr) {
		return false;
	}

	if (!input_->TriggerKey(DIK_SPACE)) {
		return true;
	}

	if (page_ == Page::kTitle) {
		page_ = Page::kInstruction;
		return true;
	}

	requestedScene_ = SceneType::kGame;
	return true;
}

void TitleScene::Draw() {
	Sprite* sprite =
		page_ == Page::kTitle ? titleSprite_ : instructionSprite_;

	if (sprite == nullptr) {
		return;
	}

	sprite->Draw();
}

void TitleScene::Finalize() {
	delete titleSprite_;
	delete instructionSprite_;

	titleSprite_ = nullptr;
	instructionSprite_ = nullptr;
}

SceneType TitleScene::GetRequestedScene() const {
	return requestedScene_;
}

void TitleScene::Reset() {
	page_ = Page::kTitle;
	requestedScene_ = SceneType::kNone;
}
