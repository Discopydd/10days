#include "ThirdStageScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void ThirdStageScene::Initialize() {
	input_ = Input::GetInstance();

	player_ = new Player();
	anchorSwing_ = new AnchorSwingGimmick();
	movableBlock_ = new MovableBlockGimmick();
	door_ = new SwitchDoorGimmick();
	power_ = new PowerGimmick();
	camera_ = new Camera();

	anchorModel_ = Model::CreateFromOBJ("anchor");
	floorModel_ = Model::CreateFromOBJ("floorwood");
	wallModel_ = Model::CreateFromOBJ("wallwood");
	goalModel_ = Model::CreateFromOBJ("cube");
	ropeModel_ = Model::CreateFromOBJ("cube");
	blockModel_ = Model::CreateFromOBJ("cubu");
	switchModel_ = Model::CreateFromOBJ("cube");
	doorModel_ = Model::CreateFromOBJ("cube");
	powerModel_ = Model::CreateFromOBJ("cube");
	interactionPrompt_.Initialize();

	camera_->Initialize();
	camera_->translation_ = {0.8f, 22.0f, -27.5f};
	camera_->rotation_ = {0.66f, 0.0f, 0.0f};
	camera_->UpdateMatrix();

	player_->Initialize(kPlayerStartPosition_);
	player_->SetMoveSpeed(0.10f);

	InitializeTransform(
		anchorTransform_,
		kAnchorPosition_,
		{0.55f, 0.55f, 0.55f});

	// 第3关は第二关と逆の流れにする。
	// 0: 開始～電源/箱/ゲート～助走区まで続く大平台
	// 1: 最後にスウィングで渡るGOAL島
	// 落下穴が狭いとPlayerのAABBが両側の床へ同時に掛かり、
	// 落下中に床端へめり込んで見えるため、穴幅を約3.5まで広げる。
	// 大平台右端 x=4.5 / GOAL島左端 x=8.0。
	const Vector3 floorPositions[kFloorCount] = {
		{-3.5f, -0.15f, 0.0f},
		{10.5f, -0.15f, 0.0f},
	};

	const Vector3 floorScales[kFloorCount] = {
		{8.0f, 0.15f, 5.65f},
		{2.5f, 0.15f, 5.65f},
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

	// 外周4枚 + x=1.5のゲート仕切り壁2枚。
	// 開始側には落下穴を置かず、電源と箱の解謎を先に行う。
	// z=-1.2～1.2だけをドア通路として空け、開門後に助走区へ進む。
	const Vector3 wallPositions[kWallCount] = {
		{-11.75f, 0.5f, 0.0f},
		{13.25f, 0.5f, 0.0f},
		{0.75f, 0.5f, -5.85f},
		{0.75f, 0.5f, 5.85f},
		{1.5f, 1.5f, -3.6f},
		{1.5f, 1.5f, 3.6f},
	};

	const Vector3 wallScales[kWallCount] = {
		{0.25f, 0.5f, 6.0f},
		{0.25f, 0.5f, 6.0f},
		{12.75f, 0.5f, 0.25f},
		{12.75f, 0.5f, 0.25f},
		{0.35f, 1.5f, 2.4f},
		{0.35f, 1.5f, 2.4f},
	};

	for (int i = 0; i < kWallCount; ++i) {
		InitializeTransform(
			wallTransforms_[i],
			wallPositions[i],
			wallScales[i]);
		wallAABBs_[i] = Collision::MakeAABB(
			wallPositions[i],
			wallScales[i]);
	}

	// ゲート手前：移動ブロック
	movableBlock_->Initialize(
		blockModel_,
		kBlockStartPosition_,
		kBlockScale_);

	// ゲート手前：スイッチ
	InitializeTransform(
		switchTransform_,
		kSwitchPosition_,
		kSwitchScale_);
	switchAABB_ = Collision::MakeAABB(
		kSwitchPosition_,
		kSwitchScale_);

	// 中央ゲート
	door_->Initialize(
		doorModel_,
		kDoorPosition_,
		kDoorScale_,
		4.0f);

	// 第3关：電源装置
	power_->Initialize(
		powerModel_,
		kPowerPosition_,
		kPowerScale_);

	// 最終スウィング先：GOAL
	InitializeTransform(
		goalTransform_,
		kGoalPosition_,
		kGoalScale_);
	goalAABB_ = Collision::MakeAABB(
		kGoalPosition_,
		kGoalTriggerHalfSize_);

	InitializeTransform(
		ropeTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	InitializeTransform(
		blockRopeTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	InitializeTransform(
		deviceRopeTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	anchorColor_.Initialize();
	anchorColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	floorColor_.Initialize();
	floorColor_.SetColor({0.94f, 0.91f, 0.86f, 1.0f});

	wallColor_.Initialize();
	wallColor_.SetColor({0.78f, 0.80f, 0.83f, 1.0f});

	switchColor_.Initialize();
	switchColor_.SetColor({0.20f, 0.90f, 0.25f, 1.0f});

	goalColor_.Initialize();
	goalColor_.SetColor({0.20f, 0.85f, 0.90f, 1.0f});

	ropeColor_.Initialize();
	ropeColor_.SetColor({1.0f, 0.90f, 0.20f, 1.0f});

	blockRopeColor_.Initialize();
	blockRopeColor_.SetColor({1.0f, 0.60f, 0.10f, 1.0f});

	deviceRopeColor_.Initialize();
	deviceRopeColor_.SetColor({0.95f, 0.45f, 0.95f, 1.0f});

	AnchorSwingGimmick::Settings settings{};
	settings.connectDistance = 7.5f;
	settings.ropeLength = 5.8f;
	settings.gravity = kGravity;
	settings.swingAssist = 7.0f;
	settings.maxSpeed = 16.0f;
	settings.activeDamping = 0.08f;
	settings.idleDamping = 0.42f;
	settings.stopSpeed = 0.22f;
	anchorSwing_->Initialize(settings);

	ResetDemo();
}

bool ThirdStageScene::Update() {
	if (input_->TriggerKey(DIK_R)) {
		ResetDemo();
		return true;
	}

	// 双条件成立後、ドアは毎フレーム上昇する。
	door_->Update();
	interactionPrompt_.Update();

	if (isClear_) {
		return true;
	}

	UpdateConnection();
	UpdatePlayer();
	UpdateSwitch();
	UpdateDeviceConnectionInput();
	UpdateDoorActivation();
	UpdateGoal();
	UpdateRope();
	UpdateBlockRope();
	UpdateDeviceRope();
	UpdateAnchorColor();

	return true;
}

void ThirdStageScene::Draw() {
	if (player_ == nullptr || anchorSwing_ == nullptr || movableBlock_ == nullptr ||
		door_ == nullptr || power_ == nullptr || camera_ == nullptr || anchorModel_ == nullptr ||
		floorModel_ == nullptr || wallModel_ == nullptr || goalModel_ == nullptr ||
		ropeModel_ == nullptr || blockModel_ == nullptr ||
		switchModel_ == nullptr || doorModel_ == nullptr || powerModel_ == nullptr) {
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

	// GOAL / スイッチ / ドア / ブロック
	goalModel_->Draw(goalTransform_, *camera_, &goalColor_);
	switchModel_->Draw(switchTransform_, *camera_, &switchColor_);
	door_->Draw(*camera_);
	movableBlock_->Draw(*camera_);
	power_->Draw(*camera_);

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

	if (blockPulling_ || movableBlock_->IsConnected()) {
		ropeModel_->Draw(
			blockRopeTransform_,
			*camera_,
			&blockRopeColor_);
	}

	if (deviceSelecting_ || deviceConnected_) {
		ropeModel_->Draw(
			deviceRopeTransform_,
			*camera_,
			&deviceRopeColor_);
	}

	DrawInteractionPrompt();
}

void ThirdStageScene::DrawInteractionPrompt() {
	if (player_ == nullptr || anchorSwing_ == nullptr ||
		movableBlock_ == nullptr || power_ == nullptr ||
		door_ == nullptr || camera_ == nullptr || isClear_) {
		return;
	}

	const Vector3& playerPosition = player_->GetPosition();

	// E：箱またはアンカーへの接続 / 解除。
	if (blockPulling_ || movableBlock_->IsConnected()) {
		Vector3 promptPosition = movableBlock_->GetPosition();
		promptPosition.y += movableBlock_->GetHalfSize().y + 0.82f;
		interactionPrompt_.DrawE(promptPosition, *camera_);
	} else if (anchorSwing_->IsConnected()) {
		interactionPrompt_.DrawE(
			{kAnchorPosition_.x, kAnchorPosition_.y + 0.90f, kAnchorPosition_.z},
			*camera_);
	} else {
		const float blockDistance = Collision::Distance(
			playerPosition,
			movableBlock_->GetPosition());
		if (!movableBlock_->IsLocked() && IsPlayerGrounded() &&
			blockDistance <= kBlockConnectDistance) {
			Vector3 promptPosition = movableBlock_->GetPosition();
			promptPosition.y += movableBlock_->GetHalfSize().y + 0.82f;
			interactionPrompt_.DrawE(promptPosition, *camera_);
		} else {
			const float anchorDistance = Collision::Distance(
				playerPosition,
				kAnchorPosition_);
			if (anchorDistance <= anchorSwing_->GetSettings().connectDistance) {
				interactionPrompt_.DrawE(
					{kAnchorPosition_.x, kAnchorPosition_.y + 0.90f, kAnchorPosition_.z},
					*camera_);
			}
		}
	}

	// F：未接続時は選択可能な装置、選択中は次の装置、
	// 接続後は解除可能な電源の頭上に表示する。
	const float powerDistance = Collision::Distance(
		playerPosition,
		power_->GetPosition());
	const Vector3 doorPosition = GetDoorCenter();
	const float doorDistance = Collision::Distance(
		playerPosition,
		doorPosition);

	auto drawPowerPrompt = [this]() {
		Vector3 promptPosition = power_->GetPosition();
		promptPosition.y += kPowerScale_.y + 0.82f;
		interactionPrompt_.DrawF(promptPosition, *camera_);
	};

	auto drawDoorPrompt = [this, &doorPosition]() {
		Vector3 promptPosition = doorPosition;
		promptPosition.y += kDoorScale_.y + 0.82f;
		interactionPrompt_.DrawF(promptPosition, *camera_);
	};

	if (deviceConnected_) {
		if (powerDistance <= kDeviceConnectDistance) {
			drawPowerPrompt();
		}
		return;
	}

	if (deviceSelecting_) {
		if (selectedDeviceType_ == DeviceType::kPower &&
			doorDistance <= kDeviceConnectDistance) {
			drawDoorPrompt();
		} else if (selectedDeviceType_ == DeviceType::kDoor &&
			powerDistance <= kDeviceConnectDistance) {
			drawPowerPrompt();
		}
		return;
	}

	const bool canSelectPower = powerDistance <= kDeviceConnectDistance;
	const bool canSelectDoor = doorDistance <= kDeviceConnectDistance;
	if (canSelectPower && (!canSelectDoor || powerDistance <= doorDistance)) {
		drawPowerPrompt();
	} else if (canSelectDoor) {
		drawDoorPrompt();
	}
}

void ThirdStageScene::Finalize() {
	if (player_ != nullptr) {
		player_->Finalize();
		delete player_;
		player_ = nullptr;
	}

	delete anchorSwing_;
	anchorSwing_ = nullptr;

	delete movableBlock_;
	movableBlock_ = nullptr;

	delete door_;
	door_ = nullptr;

	delete power_;
	power_ = nullptr;
	interactionPrompt_.Finalize();

	delete camera_;
	camera_ = nullptr;

	delete anchorModel_;
	delete floorModel_;
	delete wallModel_;
	delete goalModel_;
	delete ropeModel_;
	delete blockModel_;
	delete switchModel_;
	delete doorModel_;
	delete powerModel_;

	anchorModel_ = nullptr;
	floorModel_ = nullptr;
	wallModel_ = nullptr;
	goalModel_ = nullptr;
	ropeModel_ = nullptr;
	blockModel_ = nullptr;
	switchModel_ = nullptr;
	doorModel_ = nullptr;
	powerModel_ = nullptr;
}

void ThirdStageScene::ResetStage() {
	ResetDemo();
}

void ThirdStageScene::InitializeTransform(
    WorldTransform& transform,
    const Vector3& position,
    const Vector3& scale) {

	transform.Initialize();
	transform.translation_ = position;
	transform.scale_ = scale;
	transform.UpdateMatarix();
}

Vector3 ThirdStageScene::GetPlayerMoveVelocity() const {
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

void ThirdStageScene::UpdateConnection() {
	if (player_ == nullptr || anchorSwing_ == nullptr || movableBlock_ == nullptr) {
		return;
	}

	if (!input_->TriggerKey(DIK_E)) {
		return;
	}

	// 箱を引き寄せ中、または接続済みならEでもう一度解除する。
	if (blockPulling_ || movableBlock_->IsConnected()) {
		CancelBlockConnection();
		return;
	}

	// すでにアンカー接続中ならEで解除。
	if (anchorSwing_->IsConnected()) {
		anchorSwing_->Disconnect();
		return;
	}

	// ゲート手前の大平台では箱接続を優先する。
	// 第一关と同様、すぐ接続完了にはせず「お互いに引き寄せる」状態へ入る。
	if (!movableBlock_->IsLocked() && IsPlayerGrounded()) {
		const float blockDistance = Collision::Distance(
			player_->GetPosition(),
			movableBlock_->GetPosition());

		if (blockDistance <= kBlockConnectDistance) {
			blockPulling_ = true;
			movableBlock_->SetPullingVisual(true);
			return;
		}
	}

	// 箱対象がなければ従来通りアンカー接続を試す。
	anchorSwing_->ToggleConnection(
		kAnchorPosition_,
		player_->GetPosition());
}

bool ThirdStageScene::IsPlayerGrounded() const {
	if (player_ == nullptr) {
		return false;
	}

	float standingY = 0.0f;
	if (!GetStandingY(player_->GetPosition(), standingY)) {
		return false;
	}

	return std::abs(player_->GetPosition().y - standingY) <= kGroundTolerance;
}

bool ThirdStageScene::HasGateBarrierCollision(
    const Vector3& position) const {

	if (player_ == nullptr) {
		return false;
	}

	const Collision::AABB playerAABB = player_->GetAABBAt(position);

	// 紫色電源は見た目だけではなく実体障害物。
	// 地上・空中・スウィング中のどの状態でも内部へ入れない。
	if (power_ != nullptr &&
		Collision::IsOverlap(playerAABB, power_->GetAABB())) {
		return true;
	}

	// ゲート仕切り壁は、スウィング中でも上から飛び越えられないよう
	// XZ平面で通行を制限する。
	for (int i = 4; i < kWallCount; ++i) {
		if (Collision::IsOverlapXZ(playerAABB, wallAABBs_[i])) {
			return true;
		}
	}

	// ドアが完全に開くまでは、空中・摆荡中も中央通路を塞ぐ。
	if (door_ != nullptr && door_->IsBlocking()) {
		if (Collision::IsOverlapXZ(playerAABB, door_->GetAABB())) {
			return true;
		}
	}

	return false;
}

void ThirdStageScene::ResolveGateBarrierCollision(
    const Vector3& previousPosition,
    Vector3& position) {

	if (!HasGateBarrierCollision(position)) {
		return;
	}

	// まずXだけ戻して、壁沿いにZ方向へ滑れるか試す。
	Vector3 candidate = position;
	candidate.x = previousPosition.x;
	if (!HasGateBarrierCollision(candidate)) {
		position = candidate;
		playerVelocity_.x = 0.0f;
		return;
	}

	// 次にZだけ戻して、X方向へ滑れるか試す。
	candidate = position;
	candidate.z = previousPosition.z;
	if (!HasGateBarrierCollision(candidate)) {
		position = candidate;
		playerVelocity_.z = 0.0f;
		return;
	}

	// どちらも通れない場合はXZを前フレーム位置へ戻す。
	position.x = previousPosition.x;
	position.z = previousPosition.z;
	playerVelocity_.x = 0.0f;
	playerVelocity_.z = 0.0f;
}

Vector3 ThirdStageScene::MoveGroundPlayerWithCollision(
    const Vector3& move,
    bool ignoreBlock) {

	if (player_ == nullptr) {
		return {};
	}

	Collision::AABB obstacles[kWallCount + 2]{};
	int count = 0;

	for (int i = 0; i < kWallCount; ++i) {
		obstacles[count++] = wallAABBs_[i];
	}

	if (door_ != nullptr && door_->IsBlocking()) {
		obstacles[count++] = door_->GetAABB();
	}

	// 紫色電源もPlayer用の固定障害物として登録する。
	if (power_ != nullptr) {
		obstacles[count++] = power_->GetAABB();
	}

	Collision::AABB blockAABB{};
	const Collision::AABB* dynamicObstacle = nullptr;
	if (!ignoreBlock && movableBlock_ != nullptr) {
		blockAABB = movableBlock_->GetAABB();
		dynamicObstacle = &blockAABB;
	}

	return player_->MoveWithCollision(
		move,
		obstacles,
		count,
		dynamicObstacle);
}

void ThirdStageScene::PrepareBlockObstacles() {
	if (movableBlock_ == nullptr) {
		return;
	}

	movableBlock_->ClearObstacles();

	// 第3关の箱は開始側の大平台で操作するため、外周・ゲート壁をすべて登録。
	for (int i = 0; i < kWallCount; ++i) {
		movableBlock_->AddObstacle(wallAABBs_[i]);
	}

	// 閉じている中央ゲート。
	if (door_ != nullptr && door_->IsBlocking()) {
		movableBlock_->AddObstacle(door_->GetAABB());
	}

	// 紫色電源は箱に対しても実体障害物。押し抜け・引き抜けを禁止する。
	if (power_ != nullptr) {
		movableBlock_->AddObstacle(power_->GetAABB());
	}

	// 箱が開始側の大平台から最後の穴へ落ちないための見えない境界。
	const Collision::AABB& mainFloor = floorAABBs_[0];
	const float floorCenterX = (mainFloor.min.x + mainFloor.max.x) * 0.5f;
	const float floorCenterZ = (mainFloor.min.z + mainFloor.max.z) * 0.5f;
	const float floorHalfX = (mainFloor.max.x - mainFloor.min.x) * 0.5f;
	const float floorHalfZ = (mainFloor.max.z - mainFloor.min.z) * 0.5f;
	constexpr float kBorderHalfThickness = 0.20f;

	movableBlock_->AddObstacle(Collision::MakeAABB(
		{mainFloor.min.x - kBorderHalfThickness, 1.0f, floorCenterZ},
		{kBorderHalfThickness, 1.0f, floorHalfZ}));
	movableBlock_->AddObstacle(Collision::MakeAABB(
		{mainFloor.max.x + kBorderHalfThickness, 1.0f, floorCenterZ},
		{kBorderHalfThickness, 1.0f, floorHalfZ}));
	movableBlock_->AddObstacle(Collision::MakeAABB(
		{floorCenterX, 1.0f, mainFloor.min.z - kBorderHalfThickness},
		{floorHalfX, 1.0f, kBorderHalfThickness}));
	movableBlock_->AddObstacle(Collision::MakeAABB(
		{floorCenterX, 1.0f, mainFloor.max.z + kBorderHalfThickness},
		{floorHalfX, 1.0f, kBorderHalfThickness}));
}

Vector3 ThirdStageScene::UpdateConnectedBlockMovement(
    const Vector3& desiredMove) {

	if (player_ == nullptr || movableBlock_ == nullptr ||
		!movableBlock_->IsConnected() || movableBlock_->IsLocked()) {
		return MoveGroundPlayerWithCollision(desiredMove);
	}

	Vector3 diff = {
		movableBlock_->GetPosition().x - player_->GetPosition().x,
		0.0f,
		movableBlock_->GetPosition().z - player_->GetPosition().z,
	};

	const float distanceSq = diff.x * diff.x + diff.z * diff.z;
	if (distanceSq <= 0.0001f) {
		return MoveGroundPlayerWithCollision(desiredMove, true);
	}

	const float distance = std::sqrt(distanceSq);
	const Vector3 direction = {
		diff.x / distance,
		0.0f,
		diff.z / distance,
	};

	const float moveAmount =
		desiredMove.x * direction.x +
		desiredMove.z * direction.z;

	PrepareBlockObstacles();

	Vector3 actualPlayerMove{};

	if (moveAmount > 0.0001f) {
		// 押す：ブロックを先に動かしてからプレイヤーを追従。
		movableBlock_->MoveBy({
			direction.x * moveAmount,
			0.0f,
			direction.z * moveAmount,
		});
		actualPlayerMove = MoveGroundPlayerWithCollision(desiredMove);
	} else if (moveAmount < -0.0001f) {
		// 引く：プレイヤーが実際に動けた分だけブロックを追従。
		actualPlayerMove = MoveGroundPlayerWithCollision(desiredMove);
		const float actualPullAmount =
			actualPlayerMove.x * direction.x +
			actualPlayerMove.z * direction.z;

		if (actualPullAmount < 0.0f) {
			movableBlock_->MoveBy({
				direction.x * actualPullAmount,
				0.0f,
				direction.z * actualPullAmount,
			});
		}
	} else {
		// 横方向へ回り込むときはプレイヤーのみ移動。
		actualPlayerMove = MoveGroundPlayerWithCollision(desiredMove);
	}

	const float blockDistance = Collision::Distance(
		player_->GetPosition(),
		movableBlock_->GetPosition());

	if (blockDistance > kBlockDisconnectDistance) {
		movableBlock_->SetConnected(false);
	}

	return actualPlayerMove;
}

Vector3 ThirdStageScene::UpdateBlockPullTogether() {
	if (player_ == nullptr || movableBlock_ == nullptr || !blockPulling_ ||
		movableBlock_->IsLocked()) {
		CancelBlockConnection();
		return {};
	}

	// 地面から外れた状態では箱の引き寄せを継続しない。
	if (!IsPlayerGrounded()) {
		CancelBlockConnection();
		return {};
	}

	Vector3 playerPosition = player_->GetPosition();
	Vector3 blockPosition = movableBlock_->GetPosition();
	Vector3 diff = {
		blockPosition.x - playerPosition.x,
		0.0f,
		blockPosition.z - playerPosition.z,
	};

	float distanceXZ = std::sqrt(diff.x * diff.x + diff.z * diff.z);
	if (distanceXZ <= 0.0001f) {
		CancelBlockConnection();
		return {};
	}

	Vector3 direction = {
		diff.x / distanceXZ,
		0.0f,
		diff.z / distanceXZ,
	};

	float targetDistance =
		CalculateBlockContactDistanceXZ(direction) + kBlockContactSkin;
	float remainingDistance = distanceXZ - targetDistance;

	if (remainingDistance <= 0.005f) {
		blockPulling_ = false;
		movableBlock_->SetPullingVisual(false);
		movableBlock_->SetConnected(true);
		return {};
	}

	PrepareBlockObstacles();

	// 第一关と同じ感覚になるよう、箱とPlayerを両側から近づける。
	const float blockStep = (std::min)(
		kBlockPullSpeedPerFrame,
		remainingDistance * 0.5f);

	movableBlock_->MoveBy({
		-direction.x * blockStep,
		0.0f,
		-direction.z * blockStep,
	});

	// 箱が動いた後の距離を再計算してPlayer側も近づける。
	playerPosition = player_->GetPosition();
	blockPosition = movableBlock_->GetPosition();
	diff = {
		blockPosition.x - playerPosition.x,
		0.0f,
		blockPosition.z - playerPosition.z,
	};
	distanceXZ = std::sqrt(diff.x * diff.x + diff.z * diff.z);

	if (distanceXZ <= 0.0001f) {
		CancelBlockConnection();
		return {};
	}

	direction = {
		diff.x / distanceXZ,
		0.0f,
		diff.z / distanceXZ,
	};
	targetDistance =
		CalculateBlockContactDistanceXZ(direction) + kBlockContactSkin;
	remainingDistance = distanceXZ - targetDistance;

	Vector3 actualPlayerMove{};
	if (remainingDistance > 0.0f) {
		const float playerStep = (std::min)(
			kBlockPullSpeedPerFrame,
			remainingDistance);
		actualPlayerMove = MoveGroundPlayerWithCollision({
			direction.x * playerStep,
			0.0f,
			direction.z * playerStep,
		}, true);
	}

	playerPosition = player_->GetPosition();
	blockPosition = movableBlock_->GetPosition();
	diff = {
		blockPosition.x - playerPosition.x,
		0.0f,
		blockPosition.z - playerPosition.z,
	};
	distanceXZ = std::sqrt(diff.x * diff.x + diff.z * diff.z);

	if (distanceXZ > 0.0001f) {
		direction = {
			diff.x / distanceXZ,
			0.0f,
			diff.z / distanceXZ,
		};
		targetDistance =
			CalculateBlockContactDistanceXZ(direction) + kBlockContactSkin;

		if (distanceXZ <= targetDistance + 0.01f) {
			blockPulling_ = false;
			movableBlock_->SetPullingVisual(false);
			movableBlock_->SetConnected(true);
		}
	}

	return actualPlayerMove;
}

float ThirdStageScene::CalculateBlockContactDistanceXZ(
    const Vector3& direction) const {

	if (player_ == nullptr || movableBlock_ == nullptr) {
		return 0.0f;
	}

	const Vector3& playerHalfSize = player_->GetHalfSize();
	const Vector3& blockHalfSize = movableBlock_->GetHalfSize();
	const float sumHalfX = playerHalfSize.x + blockHalfSize.x;
	const float sumHalfZ = playerHalfSize.z + blockHalfSize.z;
	const float absX = std::abs(direction.x);
	const float absZ = std::abs(direction.z);

	float contactX = 100000.0f;
	float contactZ = 100000.0f;
	if (absX > 0.0001f) {
		contactX = sumHalfX / absX;
	}
	if (absZ > 0.0001f) {
		contactZ = sumHalfZ / absZ;
	}

	const float result = (std::min)(contactX, contactZ);
	return result >= 99999.0f ? sumHalfX : result;
}

void ThirdStageScene::CancelBlockConnection() {
	blockPulling_ = false;
	if (movableBlock_ != nullptr) {
		movableBlock_->SetPullingVisual(false);
		movableBlock_->SetConnected(false);
	}
}

void ThirdStageScene::UpdatePlayer() {
	if (player_ == nullptr || anchorSwing_ == nullptr || movableBlock_ == nullptr) {
		return;
	}

	Vector3 playerPosition = player_->GetPosition();
	const Vector3 previousPosition = playerPosition;
	const Vector3 moveVelocity = GetPlayerMoveVelocity();

	if (anchorSwing_->IsConnected()) {
		// アンカー接続中はブロック操作を解除する。
		if (blockPulling_ || movableBlock_->IsConnected()) {
			CancelBlockConnection();
		}

		anchorSwing_->Update(
			playerPosition,
			playerVelocity_,
			moveVelocity,
			kDeltaTime);
	} else {
		float standingY = 0.0f;
		const bool hasStandingFloor = GetStandingY(playerPosition, standingY);
		const bool isGrounded =
			hasStandingFloor &&
			std::abs(playerPosition.y - standingY) <= kGroundTolerance &&
			playerVelocity_.y <= 0.0f;

		if (isGrounded) {
			player_->SetPosition({playerPosition.x, standingY, playerPosition.z});
			playerVelocity_.y = 0.0f;

			const Vector3 frameMove = player_->GetInputMove();
			Vector3 actualMove{};

			if (blockPulling_) {
				actualMove = UpdateBlockPullTogether();
			} else if (movableBlock_->IsConnected()) {
				actualMove = UpdateConnectedBlockMovement(frameMove);
			} else {
				actualMove = MoveGroundPlayerWithCollision(frameMove);
			}

			playerPosition = player_->GetPosition();
			playerVelocity_.x = actualMove.x / kDeltaTime;
			playerVelocity_.z = actualMove.z / kDeltaTime;
		} else {
			// 足場から離れたらブロック接続は解除する。
			if (blockPulling_ || movableBlock_->IsConnected()) {
				CancelBlockConnection();
			}

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

			const float airDamping = std::exp(-kAirDamping * kDeltaTime);
			playerVelocity_.x *= airDamping;
			playerVelocity_.z *= airDamping;

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

	// 地上だけでなく、空中・アンカー摆荡中も右側の仕切り壁と
	// 閉じたドアをすり抜けないようにする。
	ResolveGateBarrierCollision(previousPosition, playerPosition);

	// 空中・摆荡中も箱を立体障害物として扱う。
	// 上から落ちた場合は箱の上に着地し、横から入った場合は外へ押し戻す。
	const bool landedOnBlock =
		ResolveBlockCollision(previousPosition, playerPosition);
	const bool landedOnFloor =
		ResolveFloorLanding(previousPosition, playerPosition);
	const bool landed = landedOnBlock || landedOnFloor;

	if (landed && anchorSwing_->IsConnected()) {
		anchorSwing_->Disconnect();
	}

	ApplyStageBounds(playerPosition);

	// 右のGOAL島へ着地した後は、着地判定を1フレーム取りこぼしても
	// 次の接地フレームで確実に右側チェックポイントへ更新する。
	UpdateSafeRespawnCheckpoint(playerPosition);

	if (playerPosition.y < kFallResetY) {
		ResetPlayerAfterFall();
		return;
	}

	player_->SetPosition(playerPosition);
}

void ThirdStageScene::UpdateSwitch() {
	if (movableBlock_ == nullptr || door_ == nullptr || switchActivated_) {
		return;
	}

	if (movableBlock_->IsLocked()) {
		return;
	}

	if (!Collision::IsOverlapXZ(
			movableBlock_->GetAABB(),
			switchAABB_)) {
		return;
	}

	// 「少し触れただけ」で急に吸着しないように、
	// 箱の中心が机关中央付近まで入った時だけスナップ可能にする。
	const Vector3& blockPosition = movableBlock_->GetPosition();
	if (std::abs(blockPosition.x - kSwitchPosition_.x) > kSwitchSnapCenterTolerance ||
		std::abs(blockPosition.z - kSwitchPosition_.z) > kSwitchSnapCenterTolerance) {
		return;
	}

	const Vector3 snapPosition = {
		kSwitchPosition_.x,
		kBlockScale_.y,
		kSwitchPosition_.z,
	};

	const Collision::AABB snapAABB = Collision::MakeAABB(
		snapPosition,
		movableBlock_->GetHalfSize());

	// スナップ先が壁や閉じたドアと重なる場合は固定しない。
	// 机关配置を後から調整しても、壁越し吸着が起きないようにする。
	for (int i = 0; i < kWallCount; ++i) {
		if (Collision::IsOverlap(snapAABB, wallAABBs_[i])) {
			return;
		}
	}
	if (door_ != nullptr && door_->IsBlocking() &&
		Collision::IsOverlap(snapAABB, door_->GetAABB())) {
		return;
	}
	if (power_ != nullptr && Collision::IsOverlap(snapAABB, power_->GetAABB())) {
		return;
	}

	// プレイヤーが机关の下側など吸着先のすぐ近くにいる場合は、
	// 箱をスナップしてプレイヤーへ食い込ませない。少し余裕を持たせる。
	Collision::AABB playerClearanceAABB = snapAABB;
	playerClearanceAABB.min.x -= kSwitchSnapPlayerClearance;
	playerClearanceAABB.max.x += kSwitchSnapPlayerClearance;
	playerClearanceAABB.min.z -= kSwitchSnapPlayerClearance;
	playerClearanceAABB.max.z += kSwitchSnapPlayerClearance;

	if (player_ != nullptr &&
		Collision::IsOverlap(player_->GetAABB(), playerClearanceAABB)) {
		return;
	}

	// ブロックをスイッチ中央へ固定。
	blockPulling_ = false;
	movableBlock_->SetPullingVisual(false);
	movableBlock_->SnapAndLock(snapPosition);

	switchActivated_ = true;
	switchColor_.SetColor({1.0f, 0.85f, 0.15f, 1.0f});
	// 第3关では電源接続も完了するまでドアは開かない。
}

void ThirdStageScene::UpdateRope() {
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

void ThirdStageScene::UpdateBlockRope() {
	if (player_ == nullptr || movableBlock_ == nullptr ||
		(!blockPulling_ && !movableBlock_->IsConnected())) {
		return;
	}

	const Vector3 start = player_->GetPosition();
	const Vector3 end = movableBlock_->GetPosition();
	const Vector3 diff = {
		end.x - start.x,
		end.y - start.y,
		end.z - start.z,
	};

	const float lengthXZ = std::sqrt(diff.x * diff.x + diff.z * diff.z);
	const float length = std::sqrt(
		diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

	blockRopeTransform_.translation_ = {
		(start.x + end.x) * 0.5f,
		(start.y + end.y) * 0.5f,
		(start.z + end.z) * 0.5f,
	};

	blockRopeTransform_.scale_ = {
		0.04f,
		0.04f,
		(std::max)(length * 0.5f, 0.001f),
	};

	blockRopeTransform_.rotation_.x =
		-std::atan2(diff.y, (std::max)(lengthXZ, 0.0001f));
	blockRopeTransform_.rotation_.y = std::atan2(diff.x, diff.z);
	blockRopeTransform_.UpdateMatarix();
}

void ThirdStageScene::UpdateAnchorColor() {
	if (anchorSwing_ == nullptr) {
		return;
	}

	// テクスチャの「霊」を見やすくするため、アンカー色は固定で白にする。
	anchorColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}

Vector3 ThirdStageScene::GetDoorCenter() const {
	if (door_ == nullptr) {
		return kDoorPosition_;
	}

	// 接続線は当たり判定ではなく、実際に描画されているドア位置へ追従させる。
	// GetAABB() は開き切るまで閉位置を維持する仕様なので使用しない。
	return door_->GetCurrentPosition();
}

void ThirdStageScene::UpdateDeviceConnectionInput() {
    if (input_ == nullptr || player_ == nullptr || power_ == nullptr || door_ == nullptr) {
        return;
    }

    // Eはアンカー/箱で使用済みのため、第3关の装置接続はFを使う。
    if (!input_->TriggerKey(DIK_F)) {
        return;
    }

    const Vector3 playerPosition = player_->GetPosition();
    const float powerDistance = Collision::Distance(playerPosition, power_->GetPosition());

    // 接続解除は電源の近くでだけ許可する。
    // ドアの向こう側で誤ってFを押し、閉じ込められるのを防ぐ。
    if (deviceConnected_) {
        if (powerDistance <= kDeviceConnectDistance) {
            deviceConnected_ = false;
            deviceSelecting_ = false;
            selectedDeviceType_ = DeviceType::kNone;
            power_->SetSelected(false);
        }
        return;
    }

    const float doorDistance = Collision::Distance(playerPosition, GetDoorCenter());

    if (!deviceSelecting_) {
        const bool canSelectPower = powerDistance <= kDeviceConnectDistance;
        const bool canSelectDoor = doorDistance <= kDeviceConnectDistance;

        if (!canSelectPower && !canSelectDoor) {
            return;
        }

        deviceSelecting_ = true;
        if (canSelectPower && (!canSelectDoor || powerDistance <= doorDistance)) {
            selectedDeviceType_ = DeviceType::kPower;
            power_->SetSelected(true);
        } else {
            selectedDeviceType_ = DeviceType::kDoor;
            power_->SetSelected(false);
        }
        return;
    }

    bool connected = false;
    if (selectedDeviceType_ == DeviceType::kPower &&
        doorDistance <= kDeviceConnectDistance) {
        connected = true;
    } else if (selectedDeviceType_ == DeviceType::kDoor &&
        powerDistance <= kDeviceConnectDistance) {
        connected = true;
    }

    if (connected) {
        deviceConnected_ = true;
    }

    deviceSelecting_ = false;
    selectedDeviceType_ = DeviceType::kNone;
    power_->SetSelected(false);
}

void ThirdStageScene::UpdateDoorActivation() {
    if (door_ == nullptr) {
        return;
    }

    const bool shouldOpen = switchActivated_ && deviceConnected_;

    if (shouldOpen && !doorActivationApplied_) {
        door_->Open();
        doorActivationApplied_ = true;
    } else if (!shouldOpen && doorActivationApplied_) {
        door_->Reset();
        doorActivationApplied_ = false;
    }
}

void ThirdStageScene::UpdateDeviceRope() {
    if (player_ == nullptr || power_ == nullptr || door_ == nullptr) {
        return;
    }

    if (deviceConnected_) {
        SetDeviceRopeTransform(power_->GetPosition(), GetDoorCenter());
        return;
    }

    if (!deviceSelecting_) {
        return;
    }

    if (selectedDeviceType_ == DeviceType::kPower) {
        SetDeviceRopeTransform(power_->GetPosition(), player_->GetPosition());
    } else if (selectedDeviceType_ == DeviceType::kDoor) {
        SetDeviceRopeTransform(GetDoorCenter(), player_->GetPosition());
    }
}

void ThirdStageScene::SetDeviceRopeTransform(
    const Vector3& start,
    const Vector3& end) {

    const Vector3 diff = {
        end.x - start.x,
        end.y - start.y,
        end.z - start.z,
    };

    const float lengthXZ = std::sqrt(diff.x * diff.x + diff.z * diff.z);
    const float length = std::sqrt(
        diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    deviceRopeTransform_.translation_ = {
        (start.x + end.x) * 0.5f,
        (start.y + end.y) * 0.5f,
        (start.z + end.z) * 0.5f,
    };

    deviceRopeTransform_.scale_ = {
        0.04f,
        0.04f,
        (std::max)(length * 0.5f, 0.001f),
    };

    deviceRopeTransform_.rotation_.x =
        -std::atan2(diff.y, (std::max)(lengthXZ, 0.0001f));
    deviceRopeTransform_.rotation_.y = std::atan2(diff.x, diff.z);
    deviceRopeTransform_.UpdateMatarix();
}

void ThirdStageScene::UpdateGoal() {
	if (player_ == nullptr || anchorSwing_ == nullptr || door_ == nullptr ||
		isClear_) {
		return;
	}

	// 箱スイッチ + 電源接続 + ドア完全開放のすべてを満たした後だけGOAL有効。
	// 第3关ではこの後に最後の短いスウィングを成功させる必要がある。
	if (!switchActivated_ || !deviceConnected_ || !door_->IsOpen()) {
		return;
	}

	if (IsPlayerInsideGoal()) {
		isClear_ = true;
		anchorSwing_->Disconnect();
		if (movableBlock_ != nullptr &&
			(blockPulling_ || movableBlock_->IsConnected())) {
			CancelBlockConnection();
		}
		playerVelocity_ = {};

		goalColor_.SetColor({1.0f, 0.80f, 0.10f, 1.0f});
	}
}

void ThirdStageScene::ApplyStageBounds(Vector3& position) {
	if (player_ == nullptr) {
		return;
	}

	const Vector3& halfSize = player_->GetHalfSize();
	const float minX = kStageMinX + halfSize.x;
	const float maxX = kStageMaxX - halfSize.x;
	const float minZ = kStageMinZ + halfSize.z;
	const float maxZ = kStageMaxZ - halfSize.z;
	const float maxY = kStageMaxY - halfSize.y;

	if (position.x < minX) {
		position.x = minX;
		if (playerVelocity_.x < 0.0f) {
			playerVelocity_.x = 0.0f;
		}
	}

	if (position.x > maxX) {
		position.x = maxX;
		if (playerVelocity_.x > 0.0f) {
			playerVelocity_.x = 0.0f;
		}
	}

	if (position.z < minZ) {
		position.z = minZ;
		if (playerVelocity_.z < 0.0f) {
			playerVelocity_.z = 0.0f;
		}
	}

	if (position.z > maxZ) {
		position.z = maxZ;
		if (playerVelocity_.z > 0.0f) {
			playerVelocity_.z = 0.0f;
		}
	}

	if (position.y > maxY) {
		position.y = maxY;
		if (playerVelocity_.y > 0.0f) {
			playerVelocity_.y = 0.0f;
		}
	}
}

bool ThirdStageScene::HasFloorSupportXZ(
    const Vector3& position,
    const Collision::AABB& floorAABB) const {

	if (player_ == nullptr) {
		return false;
	}

	const Collision::AABB playerAABB =
		player_->GetAABBAt({position.x, position.y, position.z});

	const float overlapX =
		(std::min)(playerAABB.max.x, floorAABB.max.x) -
		(std::max)(playerAABB.min.x, floorAABB.min.x);

	const float overlapZ =
		(std::min)(playerAABB.max.z, floorAABB.max.z) -
		(std::max)(playerAABB.min.z, floorAABB.min.z);

	return overlapX >= kMinFloorSupport &&
		overlapZ >= kMinFloorSupport;
}

bool ThirdStageScene::GetStandingY(
    const Vector3& position,
    float& standingY) const {

	if (player_ == nullptr) {
		return false;
	}

	bool foundSupport = false;
	float highestStandingY = -100000.0f;
	const float playerHalfY = player_->GetHalfSize().y;

	// 通常の左右足場。
	for (int i = 0; i < kFloorCount; ++i) {
		if (!HasFloorSupportXZ(position, floorAABBs_[i])) {
			continue;
		}

		const float candidateY =
			floorAABBs_[i].max.y + playerHalfY;

		if (!foundSupport || candidateY > highestStandingY) {
			highestStandingY = candidateY;
			foundSupport = true;
		}
	}

	// 箱の上面も「立てる地面」として扱う。
	// これがないと、一度箱の上へ着地しても次フレームで再び落下してしまう。
	if (movableBlock_ != nullptr && HasBlockSupportXZ(position)) {
		const float candidateY =
			movableBlock_->GetAABB().max.y + playerHalfY;

		if (!foundSupport || candidateY > highestStandingY) {
			highestStandingY = candidateY;
			foundSupport = true;
		}
	}

	if (foundSupport) {
		standingY = highestStandingY;
	}

	return foundSupport;
}

bool ThirdStageScene::HasBlockSupportXZ(
    const Vector3& position) const {

	if (player_ == nullptr || movableBlock_ == nullptr) {
		return false;
	}

	const Collision::AABB playerAABB =
		player_->GetAABBAt(position);
	const Collision::AABB blockAABB = movableBlock_->GetAABB();

	const float overlapX =
		(std::min)(playerAABB.max.x, blockAABB.max.x) -
		(std::max)(playerAABB.min.x, blockAABB.min.x);

	const float overlapZ =
		(std::min)(playerAABB.max.z, blockAABB.max.z) -
		(std::max)(playerAABB.min.z, blockAABB.min.z);

	return overlapX >= kMinFloorSupport &&
		overlapZ >= kMinFloorSupport;
}

bool ThirdStageScene::ResolveBlockCollision(
    const Vector3& previousPosition,
    Vector3& position) {

	if (player_ == nullptr || movableBlock_ == nullptr) {
		return false;
	}

	const Collision::AABB blockAABB = movableBlock_->GetAABB();
	const float playerHalfY = player_->GetHalfSize().y;

	// --------------------------------------------------------
	// 1) 上から箱へ落ちた場合は箱の天面に着地させる。
	// --------------------------------------------------------
	if (playerVelocity_.y <= 0.0f && HasBlockSupportXZ(position)) {
		const float blockTop = blockAABB.max.y;
		const float previousBottom = previousPosition.y - playerHalfY;
		const float currentBottom = position.y - playerHalfY;

		const bool wasAboveBlock =
			previousBottom >= blockTop - kGroundTolerance;
		const bool reachedBlockTop =
			currentBottom <= blockTop + kGroundTolerance;

		if (wasAboveBlock && reachedBlockTop) {
			position.y = blockTop + playerHalfY;
			playerVelocity_.y = 0.0f;
			return true;
		}
	}

	// --------------------------------------------------------
	// 2) 横から箱へ入った場合は、XZ方向の浅い方へ押し戻す。
	// 摆荡の勢いで箱の内部へ入ってそのまま固まるのを防ぐ。
	// --------------------------------------------------------
	Collision::AABB playerAABB = player_->GetAABBAt(position);
	if (!Collision::IsOverlap(playerAABB, blockAABB)) {
		return false;
	}

	const float overlapX =
		(std::min)(playerAABB.max.x, blockAABB.max.x) -
		(std::max)(playerAABB.min.x, blockAABB.min.x);
	const float overlapZ =
		(std::min)(playerAABB.max.z, blockAABB.max.z) -
		(std::max)(playerAABB.min.z, blockAABB.min.z);

	constexpr float kBlockCollisionSkin = 0.03f;
	Vector3 corrected = position;

	if (overlapX <= overlapZ) {
		if (position.x < movableBlock_->GetPosition().x) {
			corrected.x =
				blockAABB.min.x - player_->GetHalfSize().x - kBlockCollisionSkin;
		} else {
			corrected.x =
				blockAABB.max.x + player_->GetHalfSize().x + kBlockCollisionSkin;
		}
		playerVelocity_.x = 0.0f;
	} else {
		if (position.z < movableBlock_->GetPosition().z) {
			corrected.z =
				blockAABB.min.z - player_->GetHalfSize().z - kBlockCollisionSkin;
		} else {
			corrected.z =
				blockAABB.max.z + player_->GetHalfSize().z + kBlockCollisionSkin;
		}
		playerVelocity_.z = 0.0f;
	}

	// 押し出し先が門・仕切り壁へ入る場合は、直前のXZ位置へ戻す。
	if (HasGateBarrierCollision(corrected)) {
		corrected.x = previousPosition.x;
		corrected.z = previousPosition.z;
		playerVelocity_.x = 0.0f;
		playerVelocity_.z = 0.0f;
	}

	position = corrected;
	return false;
}

bool ThirdStageScene::ResolveFloorLanding(
    const Vector3& previousPosition,
    Vector3& position) {

	if (player_ == nullptr || playerVelocity_.y > 0.0f) {
		return false;
	}

	const float playerHalfY = player_->GetHalfSize().y;

	for (int i = 0; i < kFloorCount; ++i) {
		if (!HasFloorSupportXZ(position, floorAABBs_[i])) {
			continue;
		}

		const float floorTop = floorAABBs_[i].max.y;
		const float standingY = floorTop + playerHalfY;
		const float previousBottom = previousPosition.y - playerHalfY;
		const float currentBottom = position.y - playerHalfY;

		const bool crossedFloorTop =
			previousBottom >= floorTop - kGroundTolerance &&
			currentBottom <= floorTop + kGroundTolerance;

		// 高速落下やアンカー拘束で1フレームだけ床面を越えた場合も、
		// Player中心がまだ床面より上なら床上へ戻して穿模を防ぐ。
		const bool shallowPenetrationFromAbove =
			position.y >= floorTop &&
			currentBottom < floorTop;

		if (crossedFloorTop || shallowPenetrationFromAbove) {
			position.y = standingY;
			playerVelocity_.y = 0.0f;

			if (i == 1) {
				safeRespawnPosition_ = kRightRespawnPosition_;
			}

			return true;
		}
	}

	return false;
}

bool ThirdStageScene::IsPlayerInsideGoal() const {
	if (player_ == nullptr) {
		return false;
	}

	const Vector3& position = player_->GetPosition();

	return
		position.x >= goalAABB_.min.x &&
		position.x <= goalAABB_.max.x &&
		position.y >= goalAABB_.min.y &&
		position.y <= goalAABB_.max.y &&
		position.z >= goalAABB_.min.z &&
		position.z <= goalAABB_.max.z;
}

bool ThirdStageScene::IsSafeRespawnPosition(
    const Vector3& position,
    int floorIndex) const {

	if (player_ == nullptr || floorIndex < 0 || floorIndex >= kFloorCount) {
		return false;
	}

	// 必ず最後に到達した同じ足場の上だけを候補にする。
	if (!HasFloorSupportXZ(position, floorAABBs_[floorIndex])) {
		return false;
	}

	const Collision::AABB playerAABB = player_->GetAABBAt(position);

	// 外周・仕切り壁と重なる位置には復活しない。
	for (int i = 0; i < kWallCount; ++i) {
		if (Collision::IsOverlap(playerAABB, wallAABBs_[i])) {
			return false;
		}
	}

	// 閉じている途中のドアとも重ならないようにする。
	if (door_ != nullptr && door_->IsBlocking() &&
		Collision::IsOverlap(playerAABB, door_->GetAABB())) {
		return false;
	}

	// 移動中・固定済みを問わず、箱がある場所には復活しない。
	if (movableBlock_ != nullptr &&
		Collision::IsOverlap(playerAABB, movableBlock_->GetAABB())) {
		return false;
	}

	// 紫色電源の中にも復活しない。
	if (power_ != nullptr &&
		Collision::IsOverlap(playerAABB, power_->GetAABB())) {
		return false;
	}

	return true;
}

Vector3 ThirdStageScene::FindSafeRespawnPosition() const {
	// 負のX側の復活点=開始大平台、正のX側=GOAL島。
	const int floorIndex = safeRespawnPosition_.x < 0.0f ? 0 : 1;

	const Vector3 kCandidateOffsets[] = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.4f},
		{0.0f, 0.0f, -1.4f},
		{-1.2f, 0.0f, 0.0f},
		{1.2f, 0.0f, 0.0f},
		{-1.2f, 0.0f, 1.4f},
		{-1.2f, 0.0f, -1.4f},
		{1.2f, 0.0f, 1.4f},
		{1.2f, 0.0f, -1.4f},
		{0.0f, 0.0f, 2.8f},
		{0.0f, 0.0f, -2.8f},
	};

	for (const Vector3& offset : kCandidateOffsets) {
		const Vector3 candidate = {
			safeRespawnPosition_.x + offset.x,
			safeRespawnPosition_.y,
			safeRespawnPosition_.z + offset.z,
		};

		if (IsSafeRespawnPosition(candidate, floorIndex)) {
			return candidate;
		}
	}

	const Vector3 fallback =
		(floorIndex == 0) ? Vector3{-9.4f, 0.6f, -2.0f}
		                  : Vector3{9.4f, 0.6f, 0.0f};

	if (IsSafeRespawnPosition(fallback, floorIndex)) {
		return fallback;
	}

	return safeRespawnPosition_;
}

void ThirdStageScene::UpdateSafeRespawnCheckpoint(
    const Vector3& position) {

	if (player_ == nullptr) {
		return;
	}

	// 第3关では右側GOAL島が最終チェックポイント。
	// 一度そこへ安全に接地したら、その後は落下しても開始地点へ戻さない。
	if (safeRespawnPosition_.x >= 0.0f) {
		return;
	}

	if (!HasFloorSupportXZ(position, floorAABBs_[1])) {
		return;
	}

	const float standingY =
		floorAABBs_[1].max.y + player_->GetHalfSize().y;

	if (std::abs(position.y - standingY) <= kGroundTolerance * 2.0f &&
		playerVelocity_.y <= 0.0f) {
		safeRespawnPosition_ = kRightRespawnPosition_;
	}
}

void ThirdStageScene::ResetPlayerAfterFall() {
	if (player_ == nullptr || anchorSwing_ == nullptr) {
		return;
	}

	anchorSwing_->Disconnect();
	if (movableBlock_ != nullptr &&
		(blockPulling_ || movableBlock_->IsConnected())) {
		CancelBlockConnection();
	}
	playerVelocity_ = {};

	// 箱や壁で固定復活点が塞がれている場合は、
	// 最後に到達した同じ足場の中から空いている位置を自動で探す。
	player_->Reset(FindSafeRespawnPosition());

	UpdateAnchorColor();
}

void ThirdStageScene::ResetDemo() {
	if (player_ == nullptr || anchorSwing_ == nullptr ||
		movableBlock_ == nullptr || door_ == nullptr || power_ == nullptr) {
		return;
	}

	anchorSwing_->Disconnect();
	movableBlock_->Reset(kBlockStartPosition_);
	blockPulling_ = false;
	door_->Reset();
	power_->Reset(kPowerPosition_);
	deviceSelecting_ = false;
	deviceConnected_ = false;
	doorActivationApplied_ = false;
	selectedDeviceType_ = DeviceType::kNone;

	playerVelocity_ = {};
	safeRespawnPosition_ = kLeftRespawnPosition_;
	player_->Reset(kPlayerStartPosition_);

	switchActivated_ = false;
	switchColor_.SetColor({0.20f, 0.90f, 0.25f, 1.0f});

	isClear_ = false;
	goalColor_.SetColor({0.20f, 0.85f, 0.90f, 1.0f});
	interactionPrompt_.ResetAnimation();

	UpdateAnchorColor();
}
