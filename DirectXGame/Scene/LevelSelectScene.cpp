#include "LevelSelectScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void LevelSelectScene::Initialize() {
	input_ = Input::GetInstance();

	panelModel_ = Model::CreateFromOBJ("select_panel");
	blockIconModel_ = Model::CreateFromOBJ("cubu");
	anchorIconModel_ = Model::CreateFromOBJ("anchor");
	playerIconModel_ = Model::CreateFromOBJ("PLAYER");
	ropeModel_ = Model::CreateFromOBJ("cube");
	floorModel_ = Model::CreateFromOBJ("floorwood");
	powerIconModel_ = Model::CreateFromOBJ("power");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 8.5f, -21.0f};
	camera_.rotation_ = {0.30f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	InitializeTransform(leftPanelTransform_, {-5.5f, 0.15f, 0.0f}, {2.2f, 0.15f, 2.6f});
	InitializeTransform(rightPanelTransform_, {0.0f, 0.15f, 0.0f}, {2.2f, 0.15f, 2.6f});
	InitializeTransform(thirdPanelTransform_, {5.5f, 0.15f, 0.0f}, {2.2f, 0.15f, 2.6f});

	InitializeTransform(blockIconTransform_, {-5.5f, 1.2f, 0.0f}, {0.9f, 0.9f, 0.9f});
	InitializeTransform(anchorIconTransform_, {0.0f, 2.4f, 0.0f}, {0.45f, 0.45f, 0.45f});

    InitializeTransform(playerIconTransform_, {0.0f, 0.95f, 0.0f}, {0.35f, 0.35f, 0.35f});
    playerIconTransform_.rotation_.y = 3.14159265f;
    playerIconTransform_.UpdateMatarix();

    InitializeTransform(ropeTransform_, {0.0f, 1.65f, 0.0f}, {0.04f, 0.04f, 0.8f});
	InitializeTransform(powerIconTransform_, {5.5f, 1.25f, 0.0f}, {0.75f, 0.75f, 0.75f});
	InitializeTransform(floorTransform_, {0.0f, -0.15f, 0.0f}, {9.0f, 0.1f, 5.0f});

	leftPanelColor_.Initialize();
	rightPanelColor_.Initialize();
	thirdPanelColor_.Initialize();
	blockIconColor_.Initialize();
	anchorIconColor_.Initialize();
	playerIconColor_.Initialize();
	ropeColor_.Initialize();
	powerIconColor_.Initialize();
	floorColor_.Initialize();

	blockIconColor_.SetColor({0.95f, 0.20f, 0.20f, 1.0f});
	anchorIconColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	playerIconColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	ropeColor_.SetColor({1.0f, 0.90f, 0.20f, 1.0f});
	powerIconColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	floorColor_.SetColor({0.94f, 0.91f, 0.86f, 1.0f});

	selectedIndex_ = 0;
	requestedScene_ = SceneType::kNone;
	UpdateSelectionColor();
	UpdateSwingRope();
}

bool LevelSelectScene::Update() {
	if (input_->TriggerKey(DIK_A) || input_->TriggerKey(DIK_LEFT)) {
		selectedIndex_ = (std::max)(0, selectedIndex_ - 1);
		UpdateSelectionColor();
	}

	if (input_->TriggerKey(DIK_D) || input_->TriggerKey(DIK_RIGHT)) {
		selectedIndex_ = (std::min)(2, selectedIndex_ + 1);
		UpdateSelectionColor();
	}

	if (input_->TriggerKey(DIK_1)) { requestedScene_ = SceneType::kGame; }
	if (input_->TriggerKey(DIK_2)) { requestedScene_ = SceneType::kSwing; }
	if (input_->TriggerKey(DIK_3)) { requestedScene_ = SceneType::kThird; }

	if (input_->TriggerKey(DIK_RETURN) || input_->TriggerKey(DIK_SPACE)) {
		switch (selectedIndex_) {
		case 0: requestedScene_ = SceneType::kGame; break;
		case 1: requestedScene_ = SceneType::kSwing; break;
		case 2: requestedScene_ = SceneType::kThird; break;
		default: break;
		}
	}

	return true;
}

void LevelSelectScene::Draw() {
	if (panelModel_ == nullptr || blockIconModel_ == nullptr || anchorIconModel_ == nullptr ||
		playerIconModel_ == nullptr || ropeModel_ == nullptr || floorModel_ == nullptr ||
		powerIconModel_ == nullptr) {
		return;
	}

	floorModel_->Draw(floorTransform_, camera_, &floorColor_);
	panelModel_->Draw(leftPanelTransform_, camera_, &leftPanelColor_);
	panelModel_->Draw(rightPanelTransform_, camera_, &rightPanelColor_);
	panelModel_->Draw(thirdPanelTransform_, camera_, &thirdPanelColor_);

	blockIconModel_->Draw(blockIconTransform_, camera_, &blockIconColor_);
	anchorIconModel_->Draw(anchorIconTransform_, camera_, &anchorIconColor_);
	playerIconModel_->Draw(playerIconTransform_, camera_, &playerIconColor_);
	ropeModel_->Draw(ropeTransform_, camera_, &ropeColor_);
	powerIconModel_->Draw(powerIconTransform_, camera_, &powerIconColor_);
}

void LevelSelectScene::Finalize() {
	delete panelModel_;
	delete blockIconModel_;
	delete anchorIconModel_;
	delete playerIconModel_;
	delete ropeModel_;
	delete floorModel_;
	delete powerIconModel_;

	panelModel_ = nullptr;
	blockIconModel_ = nullptr;
	anchorIconModel_ = nullptr;
	playerIconModel_ = nullptr;
	ropeModel_ = nullptr;
	floorModel_ = nullptr;
	powerIconModel_ = nullptr;
}

SceneType LevelSelectScene::GetRequestedScene() const { return requestedScene_; }
void LevelSelectScene::ResetRequest() { requestedScene_ = SceneType::kNone; }

void LevelSelectScene::InitializeTransform(
	WorldTransform& transform, const Vector3& position, const Vector3& scale) {
	transform.Initialize();
	transform.translation_ = position;
	transform.scale_ = scale;
	transform.UpdateMatarix();
}

void LevelSelectScene::UpdateSelectionColor() {
	const Vector4 normal1 = {0.85f, 0.20f, 0.20f, 1.0f};
	const Vector4 normal2 = {0.20f, 0.60f, 0.35f, 1.0f};
	const Vector4 normal3 = {0.55f, 0.25f, 0.75f, 1.0f};
	const Vector4 selected = {1.0f, 0.80f, 0.10f, 1.0f};

	leftPanelColor_.SetColor(selectedIndex_ == 0 ? selected : normal1);
	rightPanelColor_.SetColor(selectedIndex_ == 1 ? selected : normal2);
	thirdPanelColor_.SetColor(selectedIndex_ == 2 ? selected : normal3);
}

void LevelSelectScene::UpdateSwingRope() {
	const Vector3 start = anchorIconTransform_.translation_;
	const Vector3 end = playerIconTransform_.translation_;
	const Vector3 diff = {end.x - start.x, end.y - start.y, end.z - start.z};

	const float lengthXZ = std::sqrt(diff.x * diff.x + diff.z * diff.z);
	const float length = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

	ropeTransform_.translation_ = {
		(start.x + end.x) * 0.5f,
		(start.y + end.y) * 0.5f,
		(start.z + end.z) * 0.5f,
	};
	ropeTransform_.scale_ = {0.04f, 0.04f, length * 0.5f};
	ropeTransform_.rotation_.x = -std::atan2(diff.y, (std::max)(lengthXZ, 0.0001f));
	ropeTransform_.rotation_.y = std::atan2(diff.x, diff.z);
	ropeTransform_.UpdateMatarix();
}
