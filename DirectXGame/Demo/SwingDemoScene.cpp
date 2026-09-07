#include "SwingDemoScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void SwingDemoScene::Initialize() {
	input_ = Input::GetInstance();

	player_ = new Player();
	anchorSwing_ = new AnchorSwingGimmick();
	camera_ = new Camera();

	anchorModel_ = Model::CreateSphere(12, 12);
	floorModel_ = Model::CreateFromOBJ("cube");
	wallModel_ = Model::CreateFromOBJ("cube");
	goalModel_ = Model::CreateFromOBJ("cube");
	ropeModel_ = Model::CreateFromOBJ("cube");

	camera_->Initialize();
	camera_->translation_ = {0.0f, 23.0f, -28.0f};
	camera_->rotation_ = {0.68f, 0.0f, 0.0f};
	camera_->UpdateMatrix();

	player_->Initialize(kPlayerStartPosition_);
	player_->SetMoveSpeed(0.10f);

	InitializeTransform(
		anchorTransform_,
		kAnchorPosition_,
		{0.45f, 0.45f, 0.45f});

	const Vector3 floorPositions[kFloorCount] = {
		{-7.0f, -0.15f, 0.0f},
		{7.0f, -0.15f, 0.0f},
	};

	const Vector3 floorScales[kFloorCount] = {
		{4.0f, 0.15f, 5.0f},
		{4.0f, 0.15f, 5.0f},
	};

	for (int i = 0; i < kFloorCount; ++i) {
		InitializeTransform(
			floorTransforms_[i],
			floorPositions[i],
			floorScales[i]);

		floorAABBs_[i] = Collision::MakeAABB(
			floorPositions[i],
			floorScales[i]);
	}

	const Vector3 wallPositions[kWallCount] = {
		{-10.8f, 0.5f, 0.0f},
		{10.8f, 0.5f, 0.0f},
		{0.0f, 0.5f, -4.8f},
		{0.0f, 0.5f, 4.8f},
	};

	const Vector3 wallScales[kWallCount] = {
		{0.25f, 0.5f, 5.0f},
		{0.25f, 0.5f, 5.0f},
		{11.0f, 0.5f, 0.25f},
		{11.0f, 0.5f, 0.25f},
	};

	for (int i = 0; i < kWallCount; ++i) {
		InitializeTransform(
			wallTransforms_[i],
			wallPositions[i],
			wallScales[i]);
	}

	InitializeTransform(
		goalTransform_,
		kGoalPosition_,
		kGoalScale_);

	goalAABB_ = Collision::MakeAABB(
		kGoalPosition_,
		kGoalScale_);

	InitializeTransform(
		ropeTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	anchorColor_.Initialize();
	anchorColor_.SetColor({1.0f, 0.25f, 0.20f, 1.0f});

	floorColor_.Initialize();
	floorColor_.SetColor({0.35f, 0.38f, 0.42f, 1.0f});

	wallColor_.Initialize();
	wallColor_.SetColor({0.20f, 0.22f, 0.26f, 1.0f});

	goalColor_.Initialize();
	goalColor_.SetColor({0.20f, 0.85f, 0.90f, 1.0f});

	ropeColor_.Initialize();
	ropeColor_.SetColor({1.0f, 0.90f, 0.20f, 1.0f});

	AnchorSwingGimmick::Settings settings{};
	settings.connectDistance = 7.2f;
	settings.ropeLength = 5.8f;
	settings.gravity = kGravity;
	settings.swingAssist = 5.0f;
	settings.maxSpeed = 18.0f;
	anchorSwing_->Initialize(settings);

	ResetDemo();
}

bool SwingDemoScene::Update() {
	if (input_->TriggerKey(DIK_R)) {
		ResetDemo();
		return true;
	}

	if (isClear_) {
		return true;
	}

	UpdateConnection();
	UpdatePlayer();
	UpdateGoal();
	UpdateRope();
	UpdateAnchorColor();

	return true;
}

void SwingDemoScene::Draw() {
	if (player_ == nullptr || anchorSwing_ == nullptr || camera_ == nullptr ||
		anchorModel_ == nullptr || floorModel_ == nullptr ||
		wallModel_ == nullptr || goalModel_ == nullptr || ropeModel_ == nullptr) {
		return;
	}

	for (int i = 0; i < kFloorCount; ++i) {
		floorModel_->Draw(
			floorTransforms_[i],
			*camera_,
			&floorColor_);
	}

	for (int i = 0; i < kWallCount; ++i) {
		wallModel_->Draw(
			wallTransforms_[i],
			*camera_,
			&wallColor_);
	}

	goalModel_->Draw(
		goalTransform_,
		*camera_,
		&goalColor_);

	anchorModel_->Draw(
		anchorTransform_,
		*camera_,
		&anchorColor_);

	player_->Draw(*camera_);

	if (anchorSwing_->IsConnected()) {
		ropeModel_->Draw(
			ropeTransform_,
			*camera_,
			&ropeColor_);
	}
}

void SwingDemoScene::Finalize() {
	if (player_ != nullptr) {
		player_->Finalize();
		delete player_;
		player_ = nullptr;
	}

	delete anchorSwing_;
	anchorSwing_ = nullptr;

	delete camera_;
	camera_ = nullptr;

	delete anchorModel_;
	delete floorModel_;
	delete wallModel_;
	delete goalModel_;
	delete ropeModel_;

	anchorModel_ = nullptr;
	floorModel_ = nullptr;
	wallModel_ = nullptr;
	goalModel_ = nullptr;
	ropeModel_ = nullptr;
}

void SwingDemoScene::ResetStage() {
	ResetDemo();
}

void SwingDemoScene::InitializeTransform(
    WorldTransform& transform,
    const Vector3& position,
    const Vector3& scale) {

	transform.Initialize();
	transform.translation_ = position;
	transform.scale_ = scale;
	transform.UpdateMatarix();
}

Vector3 SwingDemoScene::GetPlayerMoveVelocity() const {
	if (player_ == nullptr) {
		return {};
	}

	const Vector3 frameMove = player_->GetInputMove();

	return {
		frameMove.x / kDeltaTime,
		0.0f,
		frameMove.z / kDeltaTime,
	};
}

void SwingDemoScene::UpdateConnection() {
	if (player_ == nullptr || anchorSwing_ == nullptr) {
		return;
	}

	if (!input_->TriggerKey(DIK_E)) {
		return;
	}

	anchorSwing_->ToggleConnection(
		kAnchorPosition_,
		player_->GetPosition());
}

void SwingDemoScene::UpdatePlayer() {
	if (player_ == nullptr || anchorSwing_ == nullptr) {
		return;
	}

	Vector3 playerPosition = player_->GetPosition();
	const Vector3 moveVelocity = GetPlayerMoveVelocity();

	if (anchorSwing_->IsConnected()) {
		anchorSwing_->Update(
			playerPosition,
			playerVelocity_,
			moveVelocity,
			kDeltaTime);
	} else {
		const bool isGrounded =
			IsOnFloor(playerPosition) &&
			playerPosition.y <= kFloorHeight + 0.001f &&
			playerVelocity_.y <= 0.0f;

		if (isGrounded) {
			playerPosition.y = kFloorHeight;
			playerVelocity_.y = 0.0f;

			const Vector3 frameMove = player_->GetInputMove();
			playerPosition.x += frameMove.x;
			playerPosition.z += frameMove.z;

			playerVelocity_.x = frameMove.x / kDeltaTime;
			playerVelocity_.z = frameMove.z / kDeltaTime;
		} else {
			const float moveLengthXZ = std::sqrt(
				moveVelocity.x * moveVelocity.x +
				moveVelocity.z * moveVelocity.z);

			if (moveLengthXZ > 0.0001f) {
				playerVelocity_.x +=
					moveVelocity.x / moveLengthXZ *
					kAirControlAcceleration * kDeltaTime;

				playerVelocity_.z +=
					moveVelocity.z / moveLengthXZ *
					kAirControlAcceleration * kDeltaTime;
			}

			const float horizontalSpeed = std::sqrt(
				playerVelocity_.x * playerVelocity_.x +
				playerVelocity_.z * playerVelocity_.z);

			if (horizontalSpeed > kMaxHorizontalSpeed) {
				const float scale = kMaxHorizontalSpeed / horizontalSpeed;
				playerVelocity_.x *= scale;
				playerVelocity_.z *= scale;
			}

			playerVelocity_.y -= kGravity * kDeltaTime;

			playerPosition.x += playerVelocity_.x * kDeltaTime;
			playerPosition.y += playerVelocity_.y * kDeltaTime;
			playerPosition.z += playerVelocity_.z * kDeltaTime;
		}
	}

	if (IsOnFloor(playerPosition) &&
		playerPosition.y < kFloorHeight) {
		playerPosition.y = kFloorHeight;

		if (playerVelocity_.y < 0.0f) {
			playerVelocity_.y = 0.0f;
		}
	}

	ApplyStageBounds(playerPosition);

	if (playerPosition.y < kFallResetY) {
		ResetPlayerAfterFall(playerPosition);
		return;
	}

	player_->SetPosition(playerPosition);
}

void SwingDemoScene::UpdateRope() {
	if (player_ == nullptr || anchorSwing_ == nullptr ||
		!anchorSwing_->IsConnected()) {
		return;
	}

	const Vector3& playerPosition = player_->GetPosition();
	const Vector3& anchorPosition = anchorSwing_->GetAnchorPosition();

	const Vector3 difference = {
		playerPosition.x - anchorPosition.x,
		playerPosition.y - anchorPosition.y,
		playerPosition.z - anchorPosition.z,
	};

	const float horizontalLength = std::sqrt(
		difference.x * difference.x +
		difference.z * difference.z);

	const float ropeLength = std::sqrt(
		horizontalLength * horizontalLength +
		difference.y * difference.y);

	ropeTransform_.translation_ = {
		(playerPosition.x + anchorPosition.x) * 0.5f,
		(playerPosition.y + anchorPosition.y) * 0.5f,
		(playerPosition.z + anchorPosition.z) * 0.5f,
	};

	ropeTransform_.scale_ = {
		0.04f,
		0.04f,
		(std::max)(ropeLength * 0.5f, 0.001f),
	};

	ropeTransform_.rotation_.x =
		-std::atan2(
			difference.y,
			(std::max)(horizontalLength, 0.0001f));

	ropeTransform_.rotation_.y =
		std::atan2(
			difference.x,
			difference.z);

	ropeTransform_.UpdateMatarix();
}

void SwingDemoScene::UpdateAnchorColor() {
	if (player_ == nullptr || anchorSwing_ == nullptr) {
		return;
	}

	if (anchorSwing_->IsConnected()) {
		anchorColor_.SetColor({1.0f, 0.85f, 0.15f, 1.0f});
		return;
	}

	const float distance = Collision::Distance(
		player_->GetPosition(),
		kAnchorPosition_);

	if (distance <= anchorSwing_->GetSettings().connectDistance) {
		anchorColor_.SetColor({0.20f, 1.0f, 0.35f, 1.0f});
	} else {
		anchorColor_.SetColor({1.0f, 0.25f, 0.20f, 1.0f});
	}
}

void SwingDemoScene::UpdateGoal() {
	if (player_ == nullptr || isClear_) {
		return;
	}

	if (Collision::IsOverlap(
			player_->GetAABB(),
			goalAABB_)) {

		isClear_ = true;
		anchorSwing_->Disconnect();
		playerVelocity_ = {};

		goalColor_.SetColor({1.0f, 0.80f, 0.10f, 1.0f});
	}
}

void SwingDemoScene::ApplyStageBounds(Vector3& position) {
	if (position.x < kStageMinX) {
		position.x = kStageMinX;
		if (playerVelocity_.x < 0.0f) {
			playerVelocity_.x = 0.0f;
		}
	}

	if (position.x > kStageMaxX) {
		position.x = kStageMaxX;
		if (playerVelocity_.x > 0.0f) {
			playerVelocity_.x = 0.0f;
		}
	}

	if (position.z < kStageMinZ) {
		position.z = kStageMinZ;
		if (playerVelocity_.z < 0.0f) {
			playerVelocity_.z = 0.0f;
		}
	}

	if (position.z > kStageMaxZ) {
		position.z = kStageMaxZ;
		if (playerVelocity_.z > 0.0f) {
			playerVelocity_.z = 0.0f;
		}
	}

	if (position.y > kStageMaxY) {
		position.y = kStageMaxY;
		if (playerVelocity_.y > 0.0f) {
			playerVelocity_.y = 0.0f;
		}
	}

}

bool SwingDemoScene::IsOnFloor(const Vector3& position) const {
	if (player_ == nullptr) {
		return false;
	}

	const Collision::AABB playerAABB =
		player_->GetAABBAt({position.x, kFloorHeight, position.z});

	for (int i = 0; i < kFloorCount; ++i) {
		if (Collision::IsOverlapXZ(
				playerAABB,
				floorAABBs_[i])) {
			return true;
		}
	}

	return false;
}

void SwingDemoScene::ResetPlayerAfterFall(
    const Vector3& fallPosition) {

	if (player_ == nullptr || anchorSwing_ == nullptr) {
		return;
	}

	anchorSwing_->Disconnect();
	playerVelocity_ = {};

	if (fallPosition.x < 0.0f) {
		player_->Reset(kLeftRespawnPosition_);
	} else {
		player_->Reset(kRightRespawnPosition_);
	}

	UpdateAnchorColor();
}

void SwingDemoScene::ResetDemo() {
	if (player_ == nullptr || anchorSwing_ == nullptr) {
		return;
	}

	anchorSwing_->Disconnect();
	playerVelocity_ = {};
	player_->Reset(kPlayerStartPosition_);

	isClear_ = false;
	goalColor_.SetColor({0.20f, 0.85f, 0.90f, 1.0f});

	UpdateAnchorColor();
}
