#include "SwitchDoorGimmick.h"

#include <algorithm>

using namespace KamataEngine;

void SwitchDoorGimmick::Initialize(
    Model* model,
    const Vector3& closedPosition,
    const Vector3& scale,
    float openHeight) {

	model_ = model;
	closedPosition_ = closedPosition;
	halfSize_ = scale;
	openY_ = closedPosition.y + openHeight;

	worldTransform_.Initialize();
	worldTransform_.translation_ = closedPosition_;
	worldTransform_.scale_ = scale;
	worldTransform_.UpdateMatarix();

	objectColor_.Initialize();
	// 閉じているドアは濃い青で表示する
	objectColor_.SetColor({0.15f, 0.30f, 0.85f, 1.0f});
}

void SwitchDoorGimmick::Update() {
	if (!openRequested_ || isOpen_) {
		worldTransform_.UpdateMatarix();
		return;
	}

	// スイッチ作動後、ドアを上方向へ持ち上げる
	worldTransform_.translation_.y += openSpeed_;

	if (worldTransform_.translation_.y >= openY_) {
		worldTransform_.translation_.y = openY_;
		isOpen_ = true;
	}

	worldTransform_.UpdateMatarix();
}

void SwitchDoorGimmick::Draw(const Camera& camera) {
	if (model_ == nullptr) {
		return;
	}

	model_->Draw(worldTransform_, camera, &objectColor_);
}

void SwitchDoorGimmick::Open() {
	openRequested_ = true;
	// 開き始めたことが分かるように少し明るくする
	objectColor_.SetColor({0.25f, 0.55f, 1.0f, 1.0f});
}

void SwitchDoorGimmick::Reset() {
	worldTransform_.translation_ = closedPosition_;
	openRequested_ = false;
	isOpen_ = false;
	objectColor_.SetColor({0.15f, 0.30f, 0.85f, 1.0f});
	worldTransform_.UpdateMatarix();
}

bool SwitchDoorGimmick::IsBlocking() const {
	// 完全に上まで開くまでは当たり判定を残す
	return !isOpen_;
}

bool SwitchDoorGimmick::IsOpen() const {
	return isOpen_;
}

Collision::AABB SwitchDoorGimmick::GetAABB() const {
	return Collision::MakeAABB(worldTransform_.translation_, halfSize_);
}

// ----- 新規追加実装 -----
void SwitchDoorGimmick::SetPowered(bool powered) {
	if (powered) {
		Open();
	} else {
		Reset();
	}
}
