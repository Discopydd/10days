#pragma once

#include <KamataEngine.h>

#include "SceneType.h"

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
	void DrawUI();
	void Finalize();

private:
	void ChangeScene(SceneType nextScene);
	void DeleteCurrentScene();

private:
	SceneType currentScene_ = SceneType::kNone;

	TitleScene* titleScene_ = nullptr;
	GameScene* gameScene_ = nullptr;
	LevelSelectScene* levelSelectScene_ = nullptr;
	SwingDemoScene* swingScene_ = nullptr;
	ThirdStageScene* thirdScene_ = nullptr;

	KamataEngine::Sprite* howToSprite_ = nullptr;
	uint32_t howToTextureHandle_ = 0;
	bool isHowToVisible_ = false;
};
