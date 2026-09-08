#pragma once

#include "SceneType.h"

class TitleScene;
class GameScene;
class LevelSelectScene;
class SwingDemoScene;

class SceneManager {
public:
	void Initialize();
	bool Update();
	void Draw();
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
};
