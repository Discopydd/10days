#include "PowerGimmick.h"

using namespace KamataEngine;

void PowerGimmick::Initialize(
    Model* model,
    const Vector3& position,
    const Vector3& scale) {

	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = scale;
	worldTransform_.UpdateMatarix();

	halfSize_ = scale;

	objectColor_.Initialize();
	// 電源は紫で表示（常にアクティブ扱いの簡易実装）
	objectColor_.SetColor({0.6f, 0.2f, 0.9f, 1.0f});

	isSelected_ = false;
}

void PowerGimmick::Draw(const Camera& camera) {
	if (model_ == nullptr) {
		return;
	}
	// 選択中は明るめにする
	if (isSelected_) {
		KamataEngine::ObjectColor temp = objectColor_;
		temp.SetColor({0.95f, 0.65f, 1.0f, 1.0f});
		model_->Draw(worldTransform_, camera, &temp);
	} else {
		model_->Draw(worldTransform_, camera, &objectColor_);
	}
}

void PowerGimmick::Reset(const Vector3& position) {
	worldTransform_.translation_ = position;
	isSelected_ = false;
	objectColor_.SetColor({0.6f, 0.2f, 0.9f, 1.0f});
	worldTransform_.UpdateMatarix();
}

void PowerGimmick::SetSelected(bool selected) {
	isSelected_ = selected;
}

bool PowerGimmick::IsSelected() const {
	return isSelected_;
}

Collision::AABB PowerGimmick::GetAABB() const {
	return Collision::MakeAABB(worldTransform_.translation_, halfSize_);
}

const KamataEngine::Vector3& PowerGimmick::GetPosition() const {
	return worldTransform_.translation_;
}