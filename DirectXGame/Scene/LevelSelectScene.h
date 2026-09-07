#pragma once

#include <KamataEngine.h>

#include "SceneType.h"

class LevelSelectScene {
public:
	void Initialize();
	bool Update();
	void Draw();
	void Finalize();

	SceneType GetRequestedScene() const;
	void ResetRequest();

private:
	void InitializeTransform(
	    KamataEngine::WorldTransform& transform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

	void UpdateSelectionColor();
	void UpdateSwingRope();

private:
	KamataEngine::Input* input_ = nullptr;

	KamataEngine::Model* panelModel_ = nullptr;
	KamataEngine::Model* blockIconModel_ = nullptr;
	KamataEngine::Model* sphereModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;

	KamataEngine::Camera camera_;

	KamataEngine::WorldTransform leftPanelTransform_;
	KamataEngine::WorldTransform rightPanelTransform_;
	KamataEngine::WorldTransform blockIconTransform_;
	KamataEngine::WorldTransform anchorIconTransform_;
	KamataEngine::WorldTransform playerIconTransform_;
	KamataEngine::WorldTransform ropeTransform_;
	KamataEngine::WorldTransform floorTransform_;

	KamataEngine::ObjectColor leftPanelColor_;
	KamataEngine::ObjectColor rightPanelColor_;
	KamataEngine::ObjectColor blockIconColor_;
	KamataEngine::ObjectColor anchorIconColor_;
	KamataEngine::ObjectColor playerIconColor_;
	KamataEngine::ObjectColor ropeColor_;
	KamataEngine::ObjectColor floorColor_;

	int selectedIndex_ = 0;
	SceneType requestedScene_ = SceneType::kNone;
};
