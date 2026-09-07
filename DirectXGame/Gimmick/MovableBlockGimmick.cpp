#include "MovableBlockGimmick.h"

#include <cmath>

using namespace KamataEngine;

void MovableBlockGimmick::Initialize(
    Model* model,
    const Vector3& position,
    const Vector3& scale) {

	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = scale;
	worldTransform_.UpdateMatarix();

	// 当たり判定の半径も表示サイズと同じ値にする
	halfSize_ = scale;

	objectColor_.Initialize();
	SetNormalColor();
}

void MovableBlockGimmick::Update(
    const Vector3& playerPosition,
    const Vector3& playerMoveDelta) {

	// スイッチなどに固定済みなら絶対に動かさない
	if (isLocked_) {
		worldTransform_.UpdateMatarix();
		return;
	}

	// 接続していない間はプレイヤーが動いてもブロックは動かない
	if (!isConnected_) {
		worldTransform_.UpdateMatarix();
		return;
	}

	// --------------------------------------------------------
	// プレイヤーからブロックへの方向をXZ平面で求める
	// --------------------------------------------------------
	Vector3 direction = {
		worldTransform_.translation_.x - playerPosition.x,
		0.0f,
		worldTransform_.translation_.z - playerPosition.z,
	};

	const float lengthSq =
		direction.x * direction.x + direction.z * direction.z;

	if (lengthSq > 0.0001f) {
		const float length = std::sqrt(lengthSq);
		direction.x /= length;
		direction.z /= length;

		// ----------------------------------------------------
		// プレイヤーの移動量を「プレイヤー→ブロック方向」へ射影する
		//
		// moveAmount > 0 : ブロック方向へ進んだ = 押す
		// moveAmount < 0 : ブロックから離れた = 引く
		// ----------------------------------------------------
		const float moveAmount =
			playerMoveDelta.x * direction.x +
			playerMoveDelta.z * direction.z;

		const Vector3 blockMove = {
			direction.x * moveAmount,
			0.0f,
			direction.z * moveAmount,
		};

		MoveBy(blockMove);
	}

	worldTransform_.UpdateMatarix();
}

void MovableBlockGimmick::Draw(const Camera& camera) {
	if (model_ != nullptr) {
		model_->Draw(worldTransform_, camera, &objectColor_);
	}
}

void MovableBlockGimmick::SetConnected(bool connected) {
	// 固定後は再接続できない
	if (isLocked_) {
		isConnected_ = false;
		return;
	}

	isConnected_ = connected;

	if (isConnected_) {
		// 接続完了後は黄色にする
		objectColor_.SetColor({1.0f, 0.85f, 0.15f, 1.0f});
	} else {
		SetNormalColor();
	}
}

void MovableBlockGimmick::SetPullingVisual(bool pulling) {
	if (isLocked_ || isConnected_) {
		return;
	}

	if (pulling) {
		// 引き寄せ中はオレンジ色で状態を分かりやすくする
		objectColor_.SetColor({1.0f, 0.45f, 0.10f, 1.0f});
	} else {
		SetNormalColor();
	}
}

Vector3 MovableBlockGimmick::MoveBy(const Vector3& move) {
	if (isLocked_) {
		return {0.0f, 0.0f, 0.0f};
	}

	const Vector3 oldPosition = worldTransform_.translation_;

	TryMoveXZ(move);
	worldTransform_.UpdateMatarix();

	return {
		worldTransform_.translation_.x - oldPosition.x,
		worldTransform_.translation_.y - oldPosition.y,
		worldTransform_.translation_.z - oldPosition.z,
	};
}

void MovableBlockGimmick::SnapAndLock(const Vector3& position) {
	worldTransform_.translation_ = position;

	// 固定した瞬間に接続も解除する
	isConnected_ = false;
	isLocked_ = true;

	// 固定成功は緑色で表示する
	objectColor_.SetColor({0.2f, 1.0f, 0.35f, 1.0f});
	worldTransform_.UpdateMatarix();
}

void MovableBlockGimmick::Reset(const Vector3& position) {
	worldTransform_.translation_ = position;
	isConnected_ = false;
	isLocked_ = false;
	SetNormalColor();
	worldTransform_.UpdateMatarix();
}

void MovableBlockGimmick::AddObstacle(const Collision::AABB& obstacle) {
	if (obstacleCount_ >= kMaxObstacles) {
		return;
	}

	obstacles_[obstacleCount_] = obstacle;
	++obstacleCount_;
}

void MovableBlockGimmick::ClearObstacles() {
	obstacleCount_ = 0;
}

Collision::AABB MovableBlockGimmick::GetAABB() const {
	return Collision::MakeAABB(worldTransform_.translation_, halfSize_);
}

const Vector3& MovableBlockGimmick::GetPosition() const {
	return worldTransform_.translation_;
}

const Vector3& MovableBlockGimmick::GetHalfSize() const {
	return halfSize_;
}

bool MovableBlockGimmick::IsConnected() const {
	return isConnected_;
}

bool MovableBlockGimmick::IsLocked() const {
	return isLocked_;
}

void MovableBlockGimmick::SetNormalColor() {
	// 通常時は赤色
	objectColor_.SetColor({1.0f, 0.15f, 0.15f, 1.0f});
}

bool MovableBlockGimmick::CanMoveTo(const Vector3& position) const {
	const Collision::AABB nextAABB =
		Collision::MakeAABB(position, halfSize_);

	for (int i = 0; i < obstacleCount_; ++i) {
		if (Collision::IsOverlap(nextAABB, obstacles_[i])) {
			return false;
		}
	}

	return true;
}

void MovableBlockGimmick::TryMoveXZ(const Vector3& move) {
	Vector3 next = worldTransform_.translation_;

	// --------------------------------------------------------
	// X方向だけ先に試す
	// --------------------------------------------------------
	next.x += move.x;
	if (CanMoveTo(next)) {
		worldTransform_.translation_.x = next.x;
	}

	// --------------------------------------------------------
	// Z方向を別に試す
	// XとZを分けることで、斜めに壁へ当たった時も滑りやすい
	// --------------------------------------------------------
	next = worldTransform_.translation_;
	next.z += move.z;
	if (CanMoveTo(next)) {
		worldTransform_.translation_.z = next.z;
	}
}
