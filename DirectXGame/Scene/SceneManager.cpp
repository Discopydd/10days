#include "SceneManager.h"

#include <KamataEngine.h>

#include "GameScene.h"
#include "TitleScene.h"
#include "LevelSelectScene.h"
#include "../Demo/SwingDemoScene.h"
#include "../Demo/ThirdStageScene.h"

using namespace KamataEngine;

void SceneManager::Initialize() {
	input_ = Input::GetInstance();

	const uint32_t howToTextureHandle =
		TextureManager::Load("instruction/instruction.png");
	howToSprite_ = Sprite::Create(howToTextureHandle, {0.0f, 0.0f});
	if (howToSprite_ != nullptr) {
		howToSprite_->SetSize({1280.0f, 720.0f});
	}

	spacePromptSprite_ = Sprite::Create(
		TextureManager::Load("ui/space_prompt.png"), {460.0f, 520.0f});
	if (spacePromptSprite_ != nullptr) {
		spacePromptSprite_->SetSize({360.0f, 120.0f});
	}

	ChangeScene(SceneType::kTitle);

	backgroundSprite_ = Sprite::Create(
		TextureManager::Load("ui/background.png"), {0.0f, 0.0f});
	if (backgroundSprite_ != nullptr) {
		backgroundSprite_->SetSize({1280.0f, 720.0f});
	}
	clearSprite_ = Sprite::Create(
		TextureManager::Load("ui/clear.png"), {0.0f, 0.0f});
	if (clearSprite_ != nullptr) {
		clearSprite_->SetSize({1280.0f, 720.0f});
		// クリア画像の不透明度。背後に通過時のステージを残す。
		constexpr float kClearOpacity = 0.65f;
		clearSprite_->SetColor({1.0f, 1.0f, 1.0f, kClearOpacity});
	}
}

bool SceneManager::Update() {
	if (isClearVisible_) {
		// クリア時に押していたSPACEでは進めず、押し直しを待つ。
		if (input_ != nullptr) {
			if (!input_->PushKey(DIK_SPACE)) {
				clearSpaceReady_ = true;
			}
			if (clearSpaceReady_ && input_->TriggerKey(DIK_SPACE)) {
				ChangeScene(SceneType::kLevelSelect);
			}
		}
		return true;
	}

	if (IsGameplayScene() && input_ != nullptr &&
		input_->TriggerKey(DIK_TAB)) {
		isHowToVisible_ = !isHowToVisible_;
		return true;
	}

	// 説明画面の表示中はゲームを一時停止する。
	if (isHowToVisible_) {
		return true;
	}

	switch (currentScene_) {
	case SceneType::kTitle: {
		if (titleScene_ == nullptr || !titleScene_->Update()) {
			return false;
		}

		const SceneType requestedScene =
			titleScene_->GetRequestedScene();

		if (requestedScene != SceneType::kNone) {
			ChangeScene(requestedScene);
		}
		break;
	}

	case SceneType::kGame: {
		if (gameScene_ == nullptr || !gameScene_->Update()) {
			return false;
		}

		if (gameScene_->IsClear()) {
			ShowClearScreen();
		}
		break;
	}

	case SceneType::kLevelSelect: {
		if (levelSelectScene_ == nullptr || !levelSelectScene_->Update()) {
			return false;
		}

		const SceneType requestedScene =
			levelSelectScene_->GetRequestedScene();

		if (requestedScene != SceneType::kNone) {
			ChangeScene(requestedScene);
		}
		break;
	}

	case SceneType::kSwing: {
		if (swingScene_ == nullptr || !swingScene_->Update()) {
			return false;
		}

		if (swingScene_->IsClear()) {
			ShowClearScreen();
		}
		break;
	}

	case SceneType::kThird: {
		if (thirdScene_ == nullptr || !thirdScene_->Update()) {
			return false;
		}

		if (thirdScene_->IsClear()) {
			ShowClearScreen();
		}
		break;
	}

	case SceneType::kNone:
	default:
		return false;
	}

	return true;
}

void SceneManager::Draw() {
	switch (currentScene_) {
	case SceneType::kTitle:
		break;

	case SceneType::kGame:
		if (gameScene_ != nullptr) {
			gameScene_->Draw();
		}
		break;

	case SceneType::kLevelSelect:
		if (levelSelectScene_ != nullptr) {
			levelSelectScene_->Draw();
		}
		break;

	case SceneType::kSwing:
		if (swingScene_ != nullptr) {
			swingScene_->Draw();
		}
		break;

	case SceneType::kThird:
		if (thirdScene_ != nullptr) {
			thirdScene_->Draw();
		}
		break;

	case SceneType::kNone:
	default:
		break;
	}
}

void SceneManager::DrawBackground() {
	if ((IsGameplayScene() || currentScene_ == SceneType::kLevelSelect) &&
		backgroundSprite_ != nullptr) {
		backgroundSprite_->Draw();
	}
}

void SceneManager::DrawSprite() {
	if (currentScene_ == SceneType::kTitle && titleScene_ != nullptr) {
		titleScene_->Draw();
	}

	if (isHowToVisible_ && howToSprite_ != nullptr) {
		howToSprite_->Draw();
	}
	if (isClearVisible_ && clearSprite_ != nullptr) {
		clearSprite_->Draw();
	}

	const bool showTitlePrompt = currentScene_ == SceneType::kTitle &&
		titleScene_ != nullptr && titleScene_->IsTitlePage();
	if ((showTitlePrompt || isClearVisible_) && spacePromptSprite_ != nullptr) {
		// 半透明のCLEAR画像より後に、不透明なキーを描画する。
		spacePromptSprite_->Draw();
	}
}

void SceneManager::Finalize() {
	DeleteCurrentScene();

	delete howToSprite_;
	howToSprite_ = nullptr;
	delete backgroundSprite_;
	backgroundSprite_ = nullptr;
	delete clearSprite_;
	clearSprite_ = nullptr;
	delete spacePromptSprite_;
	spacePromptSprite_ = nullptr;
	isHowToVisible_ = false;
	isClearVisible_ = false;
	clearSpaceReady_ = false;
	input_ = nullptr;
}

void SceneManager::ChangeScene(SceneType nextScene) {
	ResetSpacePrompt();
	isHowToVisible_ = false;
	isClearVisible_ = false;
	clearSpaceReady_ = false;
	currentScene_ = nextScene;

	switch (currentScene_) {
	case SceneType::kTitle:
		if (titleScene_ == nullptr) {
			titleScene_ = new TitleScene();
			titleScene_->Initialize();
		} else {
			titleScene_->Reset();
		}
		break;

	case SceneType::kGame:
		if (gameScene_ == nullptr) {
			gameScene_ = new GameScene();
			gameScene_->Initialize();
		} else {
			gameScene_->ResetStage();
		}
		break;

	case SceneType::kLevelSelect:
		if (levelSelectScene_ == nullptr) {
			levelSelectScene_ = new LevelSelectScene();
			levelSelectScene_->Initialize();
		}
		levelSelectScene_->ResetRequest();
		break;

	case SceneType::kSwing:
		if (swingScene_ == nullptr) {
			swingScene_ = new SwingDemoScene();
			swingScene_->Initialize();
		} else {
			swingScene_->ResetStage();
		}
		break;

	case SceneType::kThird:
		if (thirdScene_ == nullptr) {
			thirdScene_ = new ThirdStageScene();
			thirdScene_->Initialize();
		} else {
			thirdScene_->ResetStage();
		}
		break;

	case SceneType::kNone:
	default:
		break;
	}
}

void SceneManager::DeleteCurrentScene() {
	if (titleScene_ != nullptr) {
		titleScene_->Finalize();
		delete titleScene_;
		titleScene_ = nullptr;
	}

	if (gameScene_ != nullptr) {
		gameScene_->Finalize();
		delete gameScene_;
		gameScene_ = nullptr;
	}

	if (levelSelectScene_ != nullptr) {
		levelSelectScene_->Finalize();
		delete levelSelectScene_;
		levelSelectScene_ = nullptr;
	}

	if (swingScene_ != nullptr) {
		swingScene_->Finalize();
		delete swingScene_;
		swingScene_ = nullptr;
	}

	if (thirdScene_ != nullptr) {
		thirdScene_->Finalize();
		delete thirdScene_;
		thirdScene_ = nullptr;
	}

	currentScene_ = SceneType::kNone;
}

bool SceneManager::IsGameplayScene() const {
	return currentScene_ == SceneType::kGame ||
		currentScene_ == SceneType::kSwing ||
		currentScene_ == SceneType::kThird;
}

void SceneManager::ShowClearScreen() {
	ResetSpacePrompt();
	isClearVisible_ = true;
	isHowToVisible_ = false;
	clearSpaceReady_ = input_ != nullptr && !input_->PushKey(DIK_SPACE);
}

void SceneManager::ResetSpacePrompt() {
	if (spacePromptSprite_ != nullptr) {
		spacePromptSprite_->SetPosition({460.0f, 520.0f});
	}
}
