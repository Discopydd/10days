#pragma once

#include "SceneType.h"

namespace KamataEngine {
class Input;
class Sprite;
}

class TitleScene;
class GameScene;
class LevelSelectScene;
class SwingDemoScene;
class ThirdStageScene;

class SceneManager {
public:
	void Initialize();
	bool Update();
	void Draw();
	void DrawSprite();
	void Finalize();

private:
	void ChangeScene(SceneType nextScene);
	void DeleteCurrentScene();
	bool IsGameplayScene() const;

private:
	SceneType currentScene_ = SceneType::kNone;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Sprite* howToSprite_ = nullptr;
	bool isHowToVisible_ = false;

	TitleScene* titleScene_ = nullptr;
	GameScene* gameScene_ = nullptr;
	LevelSelectScene* levelSelectScene_ = nullptr;
	SwingDemoScene* swingScene_ = nullptr;
	ThirdStageScene* thirdScene_ = nullptr;
};
