#include "SceneManager.h"

#include "GameScene.h"
#include "TitleScene.h"
#include "LevelSelectScene.h"
#include "../Demo/SwingDemoScene.h"
#include "../Demo/ThirdStageScene.h"

void SceneManager::Initialize() {
	howToTextureHandle_ = KamataEngine::TextureManager::Load("Howto/Howto.png");
	howToSprite_ = KamataEngine::Sprite::Create(
		howToTextureHandle_,
		{0.0f, 0.0f});
	howToSprite_->SetSize({
		static_cast<float>(KamataEngine::WinApp::kWindowWidth),
		static_cast<float>(KamataEngine::WinApp::kWindowHeight)});

	menuTextureHandle_ = KamataEngine::TextureManager::Load("Howto/Menu.png");
	menuSprite_ = KamataEngine::Sprite::Create(
		menuTextureHandle_,
		{16.0f, 16.0f});
	menuSprite_->SetSize({240.0f, 80.0f});

	ChangeScene(SceneType::kTitle);
}

bool SceneManager::Update() {
	const bool isGameScene =
		currentScene_ == SceneType::kGame ||
		currentScene_ == SceneType::kSwing ||
		currentScene_ == SceneType::kThird;
	if (!isGameScene) {
		isHowToVisible_ = false;
	} else if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_TAB)) {
		isHowToVisible_ = !isHowToVisible_;
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
			ChangeScene(SceneType::kLevelSelect);
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
			ChangeScene(SceneType::kLevelSelect);
		}
		break;
	}

	case SceneType::kThird: {
		if (thirdScene_ == nullptr || !thirdScene_->Update()) {
			return false;
		}

		if (thirdScene_->IsClear()) {
			ChangeScene(SceneType::kLevelSelect);
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
		if (titleScene_ != nullptr) {
			titleScene_->Draw();
		}
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

void SceneManager::DrawUI() {
	const bool isGameScene =
		currentScene_ == SceneType::kGame ||
		currentScene_ == SceneType::kSwing ||
		currentScene_ == SceneType::kThird;
	if (isGameScene && menuSprite_ != nullptr) {
		menuSprite_->Draw();
	}

	if (isHowToVisible_ && howToSprite_ != nullptr) {
		howToSprite_->Draw();
	}
}

void SceneManager::Finalize() {
	DeleteCurrentScene();

	delete howToSprite_;
	howToSprite_ = nullptr;

	KamataEngine::TextureManager::Unload(howToTextureHandle_);
	howToTextureHandle_ = 0;

	delete menuSprite_;
	menuSprite_ = nullptr;

	KamataEngine::TextureManager::Unload(menuTextureHandle_);
	menuTextureHandle_ = 0;
}

void SceneManager::ChangeScene(SceneType nextScene) {
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
