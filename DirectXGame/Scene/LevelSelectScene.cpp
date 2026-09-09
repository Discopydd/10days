#include "LevelSelectScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void LevelSelectScene::Initialize() {
	input_ = Input::GetInstance();

	panelModel_ = Model::CreateFromOBJ("cube");
	blockIconModel_ = Model::CreateFromOBJ("cubu");
	sphereModel_ = Model::CreateSphere(12, 12);
	ropeModel_ = Model::CreateFromOBJ("cube");
	floorModel_ = Model::CreateFromOBJ("floorwood");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 8.0f, -18.0f};
	camera_.rotation_ = {0.28f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	InitializeTransform(
		leftPanelTransform_,
		{-3.5f, 0.15f, 0.0f},
		{2.6f, 0.15f, 2.6f});

	InitializeTransform(
		rightPanelTransform_,
		{3.5f, 0.15f, 0.0f},
		{2.6f, 0.15f, 2.6f});

	InitializeTransform(
		blockIconTransform_,
		{-3.5f, 1.2f, 0.0f},
		{0.9f, 0.9f, 0.9f});

	InitializeTransform(
		anchorIconTransform_,
		{3.5f, 2.4f, 0.0f},
		{0.45f, 0.45f, 0.45f});

	InitializeTransform(
		playerIconTransform_,
		{3.5f, 0.95f, 0.0f},
		{0.35f, 0.35f, 0.35f});

	InitializeTransform(
		ropeTransform_,
		{3.5f, 1.65f, 0.0f},
		{0.04f, 0.04f, 0.8f});

	InitializeTransform(
		floorTransform_,
		{0.0f, -0.15f, 0.0f},
		{8.0f, 0.1f, 5.0f});

	leftPanelColor_.Initialize();
	rightPanelColor_.Initialize();
	blockIconColor_.Initialize();
	anchorIconColor_.Initialize();
	playerIconColor_.Initialize();
	ropeColor_.Initialize();
	floorColor_.Initialize();

	blockIconColor_.SetColor({0.95f, 0.20f, 0.20f, 1.0f});
	anchorIconColor_.SetColor({0.20f, 0.85f, 0.35f, 1.0f});
	playerIconColor_.SetColor({0.20f, 0.70f, 1.0f, 1.0f});
	ropeColor_.SetColor({1.0f, 0.90f, 0.20f, 1.0f});
	floorColor_.SetColor({0.94f, 0.91f, 0.86f, 1.0f});

	selectedIndex_ = 0;
	requestedScene_ = SceneType::kNone;

	UpdateSelectionColor();
	UpdateSwingRope();
}

bool LevelSelectScene::Update() {
	if (input_->TriggerKey(DIK_A) || input_->TriggerKey(DIK_LEFT)) {
		selectedIndex_ = 0;
		UpdateSelectionColor();
	}

	if (input_->TriggerKey(DIK_D) || input_->TriggerKey(DIK_RIGHT)) {
		selectedIndex_ = 1;
		UpdateSelectionColor();
	}

	if (input_->TriggerKey(DIK_1)) {
		requestedScene_ = SceneType::kGame;
	}

	if (input_->TriggerKey(DIK_2)) {
		requestedScene_ = SceneType::kSwing;
	}

	if (input_->TriggerKey(DIK_RETURN) || input_->TriggerKey(DIK_SPACE)) {
		requestedScene_ =
			selectedIndex_ == 0 ? SceneType::kGame : SceneType::kSwing;
	}

	return true;
}

void LevelSelectScene::Draw() {
	if (panelModel_ == nullptr || blockIconModel_ == nullptr ||
		sphereModel_ == nullptr || ropeModel_ == nullptr ||
		floorModel_ == nullptr) {
		return;
	}

	floorModel_->Draw(floorTransform_, camera_, &floorColor_);

	panelModel_->Draw(leftPanelTransform_, camera_, &leftPanelColor_);
	panelModel_->Draw(rightPanelTransform_, camera_, &rightPanelColor_);

	blockIconModel_->Draw(blockIconTransform_, camera_, &blockIconColor_);

	sphereModel_->Draw(anchorIconTransform_, camera_, &anchorIconColor_);
	sphereModel_->Draw(playerIconTransform_, camera_, &playerIconColor_);
	ropeModel_->Draw(ropeTransform_, camera_, &ropeColor_);
}

void LevelSelectScene::Finalize() {
	delete panelModel_;
	delete blockIconModel_;
	delete sphereModel_;
	delete ropeModel_;
	delete floorModel_;

	panelModel_ = nullptr;
	blockIconModel_ = nullptr;
	sphereModel_ = nullptr;
	ropeModel_ = nullptr;
	floorModel_ = nullptr;
}

SceneType LevelSelectScene::GetRequestedScene() const {
	return requestedScene_;
}

void LevelSelectScene::ResetRequest() {
	requestedScene_ = SceneType::kNone;
}

void LevelSelectScene::InitializeTransform(
	WorldTransform& transform,
	const Vector3& position,
	const Vector3& scale) {

	transform.Initialize();
	transform.translation_ = position;
	transform.scale_ = scale;
	transform.UpdateMatarix();
}

void LevelSelectScene::UpdateSelectionColor() {
	const Vector4 normalLeft = {0.20f, 0.45f, 0.85f, 1.0f};
	const Vector4 normalRight = {0.20f, 0.60f, 0.35f, 1.0f};
	const Vector4 selected = {1.0f, 0.80f, 0.10f, 1.0f};

	leftPanelColor_.SetColor(selectedIndex_ == 0 ? selected : normalLeft);
	rightPanelColor_.SetColor(selectedIndex_ == 1 ? selected : normalRight);
}

void LevelSelectScene::UpdateSwingRope() {
	const Vector3 start = anchorIconTransform_.translation_;
	const Vector3 end = playerIconTransform_.translation_;
	const Vector3 diff = {
		end.x - start.x,
		end.y - start.y,
		end.z - start.z,
	};

	const float lengthXZ = std::sqrt(diff.x * diff.x + diff.z * diff.z);
	const float length = std::sqrt(
		diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

	ropeTransform_.translation_ = {
		(start.x + end.x) * 0.5f,
		(start.y + end.y) * 0.5f,
		(start.z + end.z) * 0.5f,
	};

	ropeTransform_.scale_ = {0.04f, 0.04f, length * 0.5f};
	ropeTransform_.rotation_.x =
		-std::atan2(diff.y, (std::max)(lengthXZ, 0.0001f));
	ropeTransform_.rotation_.y = std::atan2(diff.x, diff.z);
	ropeTransform_.UpdateMatarix();
}
