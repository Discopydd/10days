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
    objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    isSelected_ = false;
}

void PowerGimmick::Draw(const Camera& camera) {
    if (model_ == nullptr) {
        return;
    }

    model_->Draw(worldTransform_, camera, &objectColor_);
}

void PowerGimmick::Reset(const Vector3& position) {
    worldTransform_.translation_ = position;
    worldTransform_.UpdateMatarix();
    SetSelected(false);
}

void PowerGimmick::SetSelected(bool selected) {
    isSelected_ = selected;
    if (isSelected_) {
        objectColor_.SetColor({1.0f, 0.85f, 1.0f, 1.0f});
    } else {
        objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    }
}

Collision::AABB PowerGimmick::GetAABB() const {
    return Collision::MakeAABB(worldTransform_.translation_, halfSize_);
}

const Vector3& PowerGimmick::GetPosition() const {
    return worldTransform_.translation_;
}
