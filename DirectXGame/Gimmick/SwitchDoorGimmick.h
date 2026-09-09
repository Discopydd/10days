#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"

// ============================================================
// スイッチ連動ドアギミック
//
// スイッチが押されるまでは閉じた状態で通路を塞ぎ、
// Open() が呼ばれると上方向へ移動して通路を開ける。
// ============================================================
class SwitchDoorGimmick {
public:
	// ドアを初期化する
	void Initialize(
	    KamataEngine::Model* model,
	    const KamataEngine::Vector3& closedPosition,
	    const KamataEngine::Vector3& scale,
	    float openHeight);

	// 開閉アニメーションを更新する
	void Update();

	// ドアを描画する
	void Draw(const KamataEngine::Camera& camera);

	// ドアを開く
	void Open();

	// 初期状態へ戻す
	void Reset();

	// 現在も通路を塞いでいるか
	bool IsBlocking() const;

	// 完全に開いたか
	bool IsOpen() const;

	// 現在の当たり判定を取得する
	Collision::AABB GetAABB() const;

	// 描画中のドア本体の現在位置を取得する。
	// GetAABB() は開き切るまで閉位置を返すため、演出追従用はこちらを使う。
	const KamataEngine::Vector3& GetCurrentPosition() const;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;

	KamataEngine::Vector3 closedPosition_{};
	KamataEngine::Vector3 halfSize_{};

	float openY_ = 0.0f;
	float openSpeed_ = 0.10f;

	bool openRequested_ = false;
	bool isOpen_ = false;
};
