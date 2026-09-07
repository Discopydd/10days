#include "AnchorSwingGimmick.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr float kEpsilon = 0.000001f;
constexpr float kRestDirectionY = -0.995f;

float LengthSquared(const Vector3& value) {
	return value.x * value.x +
		value.y * value.y +
		value.z * value.z;
}

float Dot(const Vector3& a, const Vector3& b) {
	return a.x * b.x +
		a.y * b.y +
		a.z * b.z;
}

Vector3 Scale(const Vector3& value, float scale) {
	return {
		value.x * scale,
		value.y * scale,
		value.z * scale,
	};
}
} // namespace

void AnchorSwingGimmick::Initialize(const Settings& settings) {
	SetSettings(settings);
	anchorPosition_ = {};
	activeRopeLength_ = settings_.ropeLength;
	isConnected_ = false;
}

bool AnchorSwingGimmick::Connect(
    const Vector3& anchorPosition,
    const Vector3& playerPosition) {

	const Vector3 difference = {
		playerPosition.x - anchorPosition.x,
		playerPosition.y - anchorPosition.y,
		playerPosition.z - anchorPosition.z,
	};

	const float distance =
		std::sqrt(LengthSquared(difference));

	if (distance > settings_.connectDistance ||
		distance <= kEpsilon) {
		return false;
	}

	anchorPosition_ = anchorPosition;
	activeRopeLength_ =
		(std::min)(settings_.ropeLength, distance);
	isConnected_ = true;

	return true;
}

void AnchorSwingGimmick::Disconnect() {
	isConnected_ = false;
}

bool AnchorSwingGimmick::ToggleConnection(
    const Vector3& anchorPosition,
    const Vector3& playerPosition) {

	if (isConnected_) {
		Disconnect();
		return true;
	}

	return Connect(
		anchorPosition,
		playerPosition);
}

void AnchorSwingGimmick::Update(
    Vector3& playerPosition,
    Vector3& playerVelocity,
    const Vector3& playerMoveVelocity,
    float deltaTime) {

	if (!isConnected_ || deltaTime <= 0.0f) {
		return;
	}

	Vector3 rope = {
		playerPosition.x - anchorPosition_.x,
		playerPosition.y - anchorPosition_.y,
		playerPosition.z - anchorPosition_.z,
	};

	const float ropeLengthSq = LengthSquared(rope);
	const float inputLengthSq =
		playerMoveVelocity.x * playerMoveVelocity.x +
		playerMoveVelocity.z * playerMoveVelocity.z;
	const bool hasMoveInput = inputLengthSq > kEpsilon;

	if (ropeLengthSq > kEpsilon && hasMoveInput) {
		const float inverseLength =
			1.0f / std::sqrt(ropeLengthSq);

		const Vector3 ropeDirection =
			Scale(rope, inverseLength);

		const float radialMove =
			Dot(playerMoveVelocity, ropeDirection);

		const Vector3 tangentMove = {
			playerMoveVelocity.x - ropeDirection.x * radialMove,
			playerMoveVelocity.y - ropeDirection.y * radialMove,
			playerMoveVelocity.z - ropeDirection.z * radialMove,
		};

		const float tangentLengthSq = LengthSquared(tangentMove);

		if (tangentLengthSq > kEpsilon) {
			const Vector3 tangentDirection =
				Scale(tangentMove, 1.0f / std::sqrt(tangentLengthSq));

			playerVelocity.x +=
				tangentDirection.x * settings_.swingAssist * deltaTime;
			playerVelocity.y +=
				tangentDirection.y * settings_.swingAssist * deltaTime;
			playerVelocity.z +=
				tangentDirection.z * settings_.swingAssist * deltaTime;
		}
	}

	playerVelocity.y -=
		settings_.gravity * deltaTime;

	if (settings_.maxSpeed > 0.0f) {
		const float speedSq =
			LengthSquared(playerVelocity);

		const float maxSpeedSq =
			settings_.maxSpeed * settings_.maxSpeed;

		if (speedSq > maxSpeedSq) {
			playerVelocity = Scale(
				playerVelocity,
				settings_.maxSpeed / std::sqrt(speedSq));
		}
	}

	playerPosition.x += playerVelocity.x * deltaTime;
	playerPosition.y += playerVelocity.y * deltaTime;
	playerPosition.z += playerVelocity.z * deltaTime;

	ApplyRopeConstraint(
		playerPosition,
		playerVelocity);

	const float damping = hasMoveInput
		? settings_.activeDamping
		: settings_.idleDamping;

	if (damping > 0.0f) {
		const float dampingFactor =
			std::exp(-damping * deltaTime);
		playerVelocity = Scale(playerVelocity, dampingFactor);
	}

	if (!hasMoveInput && settings_.stopSpeed > 0.0f) {
		rope = {
			playerPosition.x - anchorPosition_.x,
			playerPosition.y - anchorPosition_.y,
			playerPosition.z - anchorPosition_.z,
		};

		const float currentRopeLengthSq = LengthSquared(rope);
		const float stopSpeedSq = settings_.stopSpeed * settings_.stopSpeed;

		if (currentRopeLengthSq > kEpsilon &&
			LengthSquared(playerVelocity) <= stopSpeedSq) {

			const Vector3 ropeDirection =
				Scale(rope, 1.0f / std::sqrt(currentRopeLengthSq));

			if (ropeDirection.y <= kRestDirectionY) {
				playerPosition = {
					anchorPosition_.x,
					anchorPosition_.y - activeRopeLength_,
					anchorPosition_.z,
				};
				playerVelocity = {};
			}
		}
	}
}

bool AnchorSwingGimmick::IsConnected() const {
	return isConnected_;
}

const Vector3& AnchorSwingGimmick::GetAnchorPosition() const {
	return anchorPosition_;
}

float AnchorSwingGimmick::GetRopeLength() const {
	return activeRopeLength_;
}

void AnchorSwingGimmick::SetSettings(
    const Settings& settings) {

	settings_ = settings;

	settings_.connectDistance =
		(std::max)(0.0f, settings_.connectDistance);

	settings_.ropeLength =
		(std::max)(0.0f, settings_.ropeLength);

	settings_.gravity =
		(std::max)(0.0f, settings_.gravity);

	settings_.swingAssist =
		(std::max)(0.0f, settings_.swingAssist);

	settings_.maxSpeed =
		(std::max)(0.0f, settings_.maxSpeed);

	settings_.activeDamping =
		(std::max)(0.0f, settings_.activeDamping);

	settings_.idleDamping =
		(std::max)(0.0f, settings_.idleDamping);

	settings_.stopSpeed =
		(std::max)(0.0f, settings_.stopSpeed);
}

const AnchorSwingGimmick::Settings&
AnchorSwingGimmick::GetSettings() const {
	return settings_;
}

void AnchorSwingGimmick::ApplyRopeConstraint(
    Vector3& playerPosition,
    Vector3& playerVelocity) const {

	Vector3 rope = {
		playerPosition.x - anchorPosition_.x,
		playerPosition.y - anchorPosition_.y,
		playerPosition.z - anchorPosition_.z,
	};

	const float distanceSq = LengthSquared(rope);

	if (distanceSq <= kEpsilon ||
		activeRopeLength_ <= kEpsilon) {
		return;
	}

	const float distance = std::sqrt(distanceSq);
	const Vector3 ropeDirection =
		Scale(rope, 1.0f / distance);

	playerPosition = {
		anchorPosition_.x + ropeDirection.x * activeRopeLength_,
		anchorPosition_.y + ropeDirection.y * activeRopeLength_,
		anchorPosition_.z + ropeDirection.z * activeRopeLength_,
	};

	const float radialSpeed =
		Dot(playerVelocity, ropeDirection);

	playerVelocity.x -= ropeDirection.x * radialSpeed;
	playerVelocity.y -= ropeDirection.y * radialSpeed;
	playerVelocity.z -= ropeDirection.z * radialSpeed;
}
