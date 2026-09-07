#include "SceneManager.h"

#include "GameScene.h"
#include "LevelSelectScene.h"
#include "../Demo/SwingDemoScene.h"

void SceneManager::Initialize() {
	ChangeScene(SceneType::kGame);
}

bool SceneManager::Update() {
	switch (currentScene_) {
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

	case SceneType::kNone:
	default:
		return false;
	}

	return true;
}

void SceneManager::Draw() {
	switch (currentScene_) {
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

	case SceneType::kNone:
	default:
		break;
	}
}

void SceneManager::Finalize() {
	DeleteCurrentScene();
}

void SceneManager::ChangeScene(SceneType nextScene) {
	currentScene_ = nextScene;

	switch (currentScene_) {
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

	case SceneType::kNone:
	default:
		break;
	}
}

void SceneManager::DeleteCurrentScene() {
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

	currentScene_ = SceneType::kNone;
}
