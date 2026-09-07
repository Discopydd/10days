#pragma once

#include "SceneType.h"

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

	GameScene* gameScene_ = nullptr;
	LevelSelectScene* levelSelectScene_ = nullptr;
	SwingDemoScene* swingScene_ = nullptr;
};
