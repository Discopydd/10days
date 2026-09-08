#include "Player.h"

#include <cmath>

using namespace KamataEngine;

void Player::Initialize(const Vector3& position) {
	input_ = Input::GetInstance();

	// デモ用のプレイヤーモデル
	model_ = Model::CreateSphere(12, 12);

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = halfSize_;
	worldTransform_.UpdateMatarix();

	objectColor_.Initialize();
	objectColor_.SetColor({0.2f, 0.7f, 1.0f, 1.0f});
}

void Player::Finalize() {
	delete model_;
	model_ = nullptr;
}

void Player::Draw(const Camera& camera) {
	if (model_ == nullptr) {
		return;
	}

	model_->Draw(
		worldTransform_,
		camera,
		&objectColor_);
}

Vector3 Player::GetInputMove() const {
	Vector3 move = {0.0f, 0.0f, 0.0f};

	if (input_ == nullptr) {
		return move;
	}

	if (input_->PushKey(DIK_W)) {
		move.z += moveSpeed_;
	}
	if (input_->PushKey(DIK_S)) {
		move.z -= moveSpeed_;
	}
	if (input_->PushKey(DIK_A)) {
		move.x -= moveSpeed_;
	}
	if (input_->PushKey(DIK_D)) {
		move.x += moveSpeed_;
	}

	// 斜め入力時も直進時と同じ速度になるように正規化する。
	const float lengthSq = move.x * move.x + move.z * move.z;
	if (lengthSq > moveSpeed_ * moveSpeed_) {
		const float scale = moveSpeed_ / std::sqrt(lengthSq);
		move.x *= scale;
		move.z *= scale;
	}

	return move;
}

Vector3 Player::MoveWithCollision(
    const Vector3& move,
    const Collision::AABB* staticObstacles,
    int obstacleCount,
    const Collision::AABB* dynamicObstacle) {

	const Vector3 oldPosition = worldTransform_.translation_;

	// --------------------------------------------------------
	// まずXZをまとめて移動できるか試す。
	// 斜め方向へ移動できる場合は、そのまま移動する。
	// --------------------------------------------------------
	Vector3 next = {
		worldTransform_.translation_.x + move.x,
		worldTransform_.translation_.y,
		worldTransform_.translation_.z + move.z,
	};

	if (CanMoveTo(
			next,
			staticObstacles,
			obstacleCount,
			dynamicObstacle)) {

		worldTransform_.translation_.x = next.x;
		worldTransform_.translation_.z = next.z;

	} else {
		// ----------------------------------------------------
		// 斜め移動できない場合はX・Zを別々に試す。
		// これにより壁に当たった時も壁沿いに滑りやすくする。
		// ----------------------------------------------------
		next = worldTransform_.translation_;
		next.x += move.x;

		if (CanMoveTo(
				next,
				staticObstacles,
				obstacleCount,
				dynamicObstacle)) {

			worldTransform_.translation_.x = next.x;
		}

		next = worldTransform_.translation_;
		next.z += move.z;

		if (CanMoveTo(
				next,
				staticObstacles,
				obstacleCount,
				dynamicObstacle)) {

			worldTransform_.translation_.z = next.z;
		}
	}

	worldTransform_.UpdateMatarix();

	// 実際に移動できた量を返す
	return {
		worldTransform_.translation_.x - oldPosition.x,
		worldTransform_.translation_.y - oldPosition.y,
		worldTransform_.translation_.z - oldPosition.z,
	};
}

bool Player::CanMoveTo(
    const Vector3& position,
    const Collision::AABB* staticObstacles,
    int obstacleCount,
    const Collision::AABB* dynamicObstacle) const {

	const Collision::AABB playerAABB = GetAABBAt(position);

	// --------------------------------------------------------
	// 壁などの固定障害物との当たり判定
	// --------------------------------------------------------
	if (staticObstacles != nullptr) {
		for (int i = 0; i < obstacleCount; ++i) {
			if (Collision::IsOverlap(
					playerAABB,
					staticObstacles[i])) {

				return false;
			}
		}
	}

	// --------------------------------------------------------
	// ブロックなどの動的障害物との当たり判定
	// --------------------------------------------------------
	if (dynamicObstacle != nullptr) {
		if (Collision::IsOverlap(
				playerAABB,
				*dynamicObstacle)) {

			return false;
		}
	}

	return true;
}

void Player::SetPosition(const Vector3& position) {
	worldTransform_.translation_ = position;
	worldTransform_.UpdateMatarix();
}

void Player::Reset(const Vector3& position) {
	SetPosition(position);
}

const Vector3& Player::GetPosition() const {
	return worldTransform_.translation_;
}

const Vector3& Player::GetHalfSize() const {
	return halfSize_;
}

Collision::AABB Player::GetAABB() const {
	return GetAABBAt(worldTransform_.translation_);
}

Collision::AABB Player::GetAABBAt(const Vector3& position) const {
	return Collision::MakeAABB(position, halfSize_);
}

void Player::SetMoveSpeed(float speed) {
	moveSpeed_ = speed;
}
