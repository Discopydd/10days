#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"

// ============================================================
// 電源ギミック
// Fキーによる装置接続の始点として使用する。
// ============================================================
class PowerGimmick {
public:
    void Initialize(
        KamataEngine::Model* model,
        const KamataEngine::Vector3& position,
        const KamataEngine::Vector3& scale);

    void Draw(const KamataEngine::Camera& camera);
    void Reset(const KamataEngine::Vector3& position);

    void SetSelected(bool selected);
    bool IsSelected() const { return isSelected_; }

    Collision::AABB GetAABB() const;
    const KamataEngine::Vector3& GetPosition() const;

private:
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::WorldTransform worldTransform_;
    KamataEngine::ObjectColor objectColor_;
    KamataEngine::Vector3 halfSize_ = {1.0f, 1.0f, 1.0f};
    bool isSelected_ = false;
};
