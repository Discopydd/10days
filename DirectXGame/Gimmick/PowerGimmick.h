#pragma once

#include <KamataEngine.h>
#include "../Common/Collision.h"

// 紫色の電源ギミック（簡易）
// - 常に設置され、選択状態の表示と当たり判定取得を提供する
class PowerGimmick {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& scale);
	void Draw(const KamataEngine::Camera& camera);
	void Reset(const KamataEngine::Vector3& position);

	// 選択表示
	void SetSelected(bool selected);
	bool IsSelected() const;

	// 描画用 / 判定用
	Collision::AABB GetAABB() const;
	const KamataEngine::Vector3& GetPosition() const;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector3 halfSize_ = {1.0f, 1.0f, 1.0f};

	bool isSelected_ = false;
};