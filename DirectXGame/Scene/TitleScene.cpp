#include "TitleScene.h"

using namespace KamataEngine;

void TitleScene::Initialize() {
	input_ = Input::GetInstance();

	// 文字入りの一時テクスチャを貼ったcubeを画面正面に表示する。
	titleModel_ = Model::CreateFromOBJ("title");
	instructionModel_ = Model::CreateFromOBJ("instruction");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.rotation_ = {0.0f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	InitializeTransform(
		panelTransform_,
		{0.0f, 0.0f, 0.0f},
		{5.6f, 3.15f, 0.05f});

	panelColor_.Initialize();
	panelColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	Reset();
}

bool TitleScene::Update() {
	if (input_ == nullptr) {
		return false;
	}

	if (!input_->TriggerKey(DIK_SPACE)) {
		return true;
	}

	if (page_ == Page::kTitle) {
		page_ = Page::kInstruction;
		return true;
	}

	requestedScene_ = SceneType::kGame;
	return true;
}

void TitleScene::Draw() {
	Model* model =
		page_ == Page::kTitle ? titleModel_ : instructionModel_;

	if (model == nullptr) {
		return;
	}

	model->Draw(panelTransform_, camera_, &panelColor_);
}

void TitleScene::Finalize() {
	delete titleModel_;
	delete instructionModel_;

	titleModel_ = nullptr;
	instructionModel_ = nullptr;
}

SceneType TitleScene::GetRequestedScene() const {
	return requestedScene_;
}

void TitleScene::Reset() {
	page_ = Page::kTitle;
	requestedScene_ = SceneType::kNone;
}

void TitleScene::InitializeTransform(
    WorldTransform& transform,
    const Vector3& position,
    const Vector3& scale) {

	transform.Initialize();
	transform.translation_ = position;
	transform.scale_ = scale;
	transform.UpdateMatarix();
}
