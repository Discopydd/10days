#pragma once

#include <cstdint>

#include "SceneType.h"

namespace KamataEngine {
class Input;
class Audio;
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
	void DrawBackground();
	void DrawSprite();
	void Finalize();

private:
	void ChangeScene(SceneType nextScene);
	void DeleteCurrentScene();
	bool IsGameplayScene() const;
	void ShowClearScreen();
	void ResetSpacePrompt();

private:
	SceneType currentScene_ = SceneType::kNone;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Audio* audio_ = nullptr;
	uint32_t clearSoundHandle_ = 0u;
	KamataEngine::Sprite* howToSprite_ = nullptr;
	bool isHowToVisible_ = false;
	KamataEngine::Sprite* backgroundSprite_ = nullptr;
	KamataEngine::Sprite* clearSprite_ = nullptr;
	bool isClearVisible_ = false;
	bool clearSpaceReady_ = false;
	KamataEngine::Sprite* spacePromptSprite_ = nullptr;

	TitleScene* titleScene_ = nullptr;
	GameScene* gameScene_ = nullptr;
	LevelSelectScene* levelSelectScene_ = nullptr;
	SwingDemoScene* swingScene_ = nullptr;
	ThirdStageScene* thirdScene_ = nullptr;
};
