#include "GameScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void GameScene::Initialize() {
	input_ = Input::GetInstance();

	camera_ = new Camera();
	player_ = new Player();
	door_ = new SwitchDoorGimmick();

	for (int i = 0; i < kBlockCount; ++i) {
		movableBlocks_[i] = new MovableBlockGimmick();
	}

	// --------------------------------------------------------
	// モデル
	// --------------------------------------------------------
	blockModel_ = Model::CreateFromOBJ("cube");
	switchModel_ = Model::CreateFromOBJ("cube");
	doorModel_ = Model::CreateFromOBJ("cube");
	goalModel_ = Model::CreateFromOBJ("cube");
	floorModel_ = Model::CreateFromOBJ("cube");
	wallModel_ = Model::CreateFromOBJ("cube");
	ropeModel_ = Model::CreateFromOBJ("cube");

	// --------------------------------------------------------
	// カメラ
	// ステージ全体が見えるように少し高めに配置する
	// --------------------------------------------------------
	camera_->Initialize();
	camera_->translation_ = {0.0f, 23.0f, -28.0f};
	camera_->rotation_ = {0.68f, 0.0f, 0.0f};
	camera_->UpdateMatrix();

	// --------------------------------------------------------
	// Player
	// --------------------------------------------------------
	player_->Initialize(kPlayerStartPosition_);

	// --------------------------------------------------------
	// 移動ブロック3個
	// --------------------------------------------------------
	for (int i = 0; i < kBlockCount; ++i) {
		movableBlocks_[i]->Initialize(
			blockModel_,
			kBlockStartPositions_[i],
			{1.0f, 1.0f, 1.0f});
	}

	// --------------------------------------------------------
	// 右上のスイッチ
	// --------------------------------------------------------
	InitializeTransform(
		switchWorldTransform_,
		kSwitchPosition_,
		kSwitchScale_);

	switchColor_.Initialize();
	switchColor_.SetColor({0.20f, 0.90f, 0.25f, 1.0f});

	switchAABB_ = Collision::MakeAABB(
		kSwitchPosition_,
		kSwitchScale_);

	// --------------------------------------------------------
	// 中央ドア
	// --------------------------------------------------------
	door_->Initialize(
		doorModel_,
		kDoorPosition_,
		kDoorScale_,
		4.0f);

	// --------------------------------------------------------
	// GOAL
	// --------------------------------------------------------
	InitializeTransform(
		goalWorldTransform_,
		kGoalPosition_,
		kGoalScale_);

	goalColor_.Initialize();
	goalColor_.SetColor({0.20f, 0.85f, 0.90f, 1.0f});

	goalAABB_ = Collision::MakeAABB(
		kGoalPosition_,
		kGoalScale_);

	// --------------------------------------------------------
	// 床
	// --------------------------------------------------------
	InitializeTransform(
		floorWorldTransform_,
		{0.0f, -0.15f, 0.0f},
		{8.0f, 0.10f, 10.0f});

	floorColor_.Initialize();
	floorColor_.SetColor({0.55f, 0.55f, 0.55f, 1.0f});

	// --------------------------------------------------------
	// 壁
	// 0:左外壁 / 1:右外壁 / 2:手前外壁 / 3:奥外壁
	// 4:中央左壁 / 5:中央右壁
	//
	// 中央壁はx=-1.5～1.5を空け、そこをドアで塞ぐ。
	// --------------------------------------------------------
	const Vector3 wallPositions[kWallCount] = {
		{-7.5f, 1.0f, 0.0f},
		{7.5f, 1.0f, 0.0f},
		{0.0f, 1.0f, -10.0f},
		{0.0f, 1.0f, 10.0f},
		{-4.75f, 1.0f, 4.0f},
		{4.75f, 1.0f, 4.0f},
	};

	const Vector3 wallScales[kWallCount] = {
		{0.5f, 1.0f, 10.0f},
		{0.5f, 1.0f, 10.0f},
		{8.0f, 1.0f, 0.5f},
		{8.0f, 1.0f, 0.5f},
		{3.25f, 1.0f, 0.5f},
		{3.25f, 1.0f, 0.5f},
	};

	for (int i = 0; i < kWallCount; ++i) {
		InitializeTransform(
			wallWorldTransforms_[i],
			wallPositions[i],
			wallScales[i]);

		wallAABBs_[i] = Collision::MakeAABB(
			wallPositions[i],
			wallScales[i]);
	}

	wallColor_.Initialize();
	wallColor_.SetColor({0.25f, 0.25f, 0.30f, 1.0f});

	// --------------------------------------------------------
	// 糸
	// --------------------------------------------------------
	InitializeTransform(
		ropeWorldTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	ropeColor_.Initialize();
	ropeColor_.SetColor({1.0f, 0.90f, 0.20f, 1.0f});

	connectionState_ = ConnectionState::kIdle;
	ropeShootProgress_ = 0.0f;
	activeBlockIndex_ = -1;
	switchActivated_ = false;
	isClear_ = false;
}

bool GameScene::Update() {
	// --------------------------------------------------------
	// R : ステージを最初からやり直す
	// --------------------------------------------------------
	if (input_->TriggerKey(DIK_R)) {
		ResetGame();
		return true;
	}

	// ドアの開閉アニメーションは毎フレーム更新する
	door_->Update();

	// クリア後はその場で停止し、Rでやり直せるようにする
	if (isClear_) {
		return true;
	}

	UpdateConnectionInput();

	switch (connectionState_) {
	case ConnectionState::kIdle: {
		const Vector3 move = GetPlayerInputMove();
		MovePlayerWithCollision(move);
		break;
	}

	case ConnectionState::kShooting:
		UpdateRopeShot();
		break;

	case ConnectionState::kPulling:
		UpdatePullTogether();
		break;

	case ConnectionState::kConnected:
		UpdateConnectedMovement();
		break;
	}

	// 接続中のブロックから離れすぎたら自動解除する
	if (connectionState_ == ConnectionState::kConnected) {
		const MovableBlockGimmick* activeBlock = GetActiveBlock();

		if (activeBlock == nullptr) {
			CancelConnection();
		} else {
			const float distanceToBlock = Collision::Distance(
				player_->GetPosition(),
				activeBlock->GetPosition());

			if (distanceToBlock > kDisconnectDistance) {
				CancelConnection();
			}
		}
	}

	// スイッチ → ドア → GOALの順にゲーム進行を判定する
	UpdateSwitch();
	UpdateGoal();

	// 接続対象とのめり込みを最後に保険として解消する
	if (activeBlockIndex_ >= 0) {
		ResolvePlayerBlockOverlap(activeBlockIndex_);
	}

	UpdateRope();

	return true;
}

bool GameScene::IsClear() const {
	return isClear_;
}

void GameScene::ResetStage() {
	ResetGame();
}

void GameScene::Draw() {
	if (player_ == nullptr || camera_ == nullptr || door_ == nullptr ||
		blockModel_ == nullptr || switchModel_ == nullptr ||
		doorModel_ == nullptr || goalModel_ == nullptr ||
		floorModel_ == nullptr || wallModel_ == nullptr ||
		ropeModel_ == nullptr) {
		return;
	}

	// 床
	floorModel_->Draw(
		floorWorldTransform_,
		*camera_,
		&floorColor_);

	// 外壁・中央仕切り壁
	for (int i = 0; i < kWallCount; ++i) {
		wallModel_->Draw(
			wallWorldTransforms_[i],
			*camera_,
			&wallColor_);
	}

	// GOAL
	goalModel_->Draw(
		goalWorldTransform_,
		*camera_,
		&goalColor_);

	// スイッチ
	switchModel_->Draw(
		switchWorldTransform_,
		*camera_,
		&switchColor_);

	// ドア
	door_->Draw(*camera_);

	// Player
	player_->Draw(*camera_);

	// 3個のBlock
	for (int i = 0; i < kBlockCount; ++i) {
		movableBlocks_[i]->Draw(*camera_);
	}

	// 接続動作中だけ糸を描画する
	if (connectionState_ != ConnectionState::kIdle &&
		activeBlockIndex_ >= 0) {
		ropeModel_->Draw(
			ropeWorldTransform_,
			*camera_,
			&ropeColor_);
	}
}

void GameScene::Finalize() {
	if (player_ != nullptr) {
		player_->Finalize();
		delete player_;
		player_ = nullptr;
	}

	for (int i = 0; i < kBlockCount; ++i) {
		delete movableBlocks_[i];
		movableBlocks_[i] = nullptr;
	}

	delete door_;
	door_ = nullptr;

	delete camera_;
	camera_ = nullptr;

	delete blockModel_;
	delete switchModel_;
	delete doorModel_;
	delete goalModel_;
	delete floorModel_;
	delete wallModel_;
	delete ropeModel_;

	blockModel_ = nullptr;
	switchModel_ = nullptr;
	doorModel_ = nullptr;
	goalModel_ = nullptr;
	floorModel_ = nullptr;
	wallModel_ = nullptr;
	ropeModel_ = nullptr;
}

void GameScene::InitializeTransform(
    WorldTransform& worldTransform,
    const Vector3& position,
    const Vector3& scale) {

	worldTransform.Initialize();
	worldTransform.translation_ = position;
	worldTransform.scale_ = scale;
	worldTransform.UpdateMatarix();
}

Vector3 GameScene::GetPlayerInputMove() const {
	return player_->GetInputMove();
}

int GameScene::BuildPlayerObstacleList(
    Collision::AABB* outObstacles,
    int maxCount,
    int ignoreBlockIndex) const {

	if (outObstacles == nullptr || maxCount <= 0) {
		return 0;
	}

	int count = 0;

	// 外壁・中央壁
	for (int i = 0; i < kWallCount && count < maxCount; ++i) {
		outObstacles[count++] = wallAABBs_[i];
	}

	// ドアが完全に開くまでは障害物として扱う
	if (door_->IsBlocking() && count < maxCount) {
		outObstacles[count++] = door_->GetAABB();
	}

	// 3個のBlockもPlayerにとって障害物
	for (int i = 0; i < kBlockCount && count < maxCount; ++i) {
		if (i == ignoreBlockIndex) {
			continue;
		}
		outObstacles[count++] = movableBlocks_[i]->GetAABB();
	}

	return count;
}

Vector3 GameScene::MovePlayerWithCollision(
    const Vector3& move,
    int ignoreBlockIndex) {

	Collision::AABB obstacles[kMaxPlayerObstacles]{};
	const int obstacleCount = BuildPlayerObstacleList(
		obstacles,
		kMaxPlayerObstacles,
		ignoreBlockIndex);

	return player_->MoveWithCollision(
		move,
		obstacles,
		obstacleCount,
		nullptr);
}

bool GameScene::CanPlayerMoveTo(
    const Vector3& position,
    int ignoreBlockIndex) const {

	Collision::AABB obstacles[kMaxPlayerObstacles]{};
	const int obstacleCount = BuildPlayerObstacleList(
		obstacles,
		kMaxPlayerObstacles,
		ignoreBlockIndex);

	return player_->CanMoveTo(
		position,
		obstacles,
		obstacleCount,
		nullptr);
}

Collision::AABB GameScene::GetPlayerAABBAt(
    const Vector3& position) const {

	return player_->GetAABBAt(position);
}

MovableBlockGimmick* GameScene::GetActiveBlock() {
	if (activeBlockIndex_ < 0 || activeBlockIndex_ >= kBlockCount) {
		return nullptr;
	}
	return movableBlocks_[activeBlockIndex_];
}

const MovableBlockGimmick* GameScene::GetActiveBlock() const {
	if (activeBlockIndex_ < 0 || activeBlockIndex_ >= kBlockCount) {
		return nullptr;
	}
	return movableBlocks_[activeBlockIndex_];
}

int GameScene::FindNearestConnectableBlock() const {
	int nearestIndex = -1;
	float nearestDistance = kConnectDistance + 1.0f;

	for (int i = 0; i < kBlockCount; ++i) {
		// スイッチ上で固定されたBlockには再接続しない
		if (movableBlocks_[i]->IsLocked()) {
			continue;
		}

		const float distance = Collision::Distance(
			player_->GetPosition(),
			movableBlocks_[i]->GetPosition());

		if (distance <= kConnectDistance &&
			distance < nearestDistance) {
			nearestDistance = distance;
			nearestIndex = i;
		}
	}

	return nearestIndex;
}

Vector3 GameScene::MoveBlockWithCollision(
    int blockIndex,
    const Vector3& move) {

	if (blockIndex < 0 || blockIndex >= kBlockCount) {
		return {0.0f, 0.0f, 0.0f};
	}

	MovableBlockGimmick* block = movableBlocks_[blockIndex];
	if (block == nullptr) {
		return {0.0f, 0.0f, 0.0f};
	}
	block->ClearObstacles();

	// 外壁・中央壁
	for (int i = 0; i < kWallCount; ++i) {
		block->AddObstacle(wallAABBs_[i]);
	}

	// 閉じているドア
	if (door_->IsBlocking()) {
		block->AddObstacle(door_->GetAABB());
	}

	// 他の2個のBlock
	for (int i = 0; i < kBlockCount; ++i) {
		if (i == blockIndex) {
			continue;
		}
		block->AddObstacle(movableBlocks_[i]->GetAABB());
	}

	return block->MoveBy(move);
}

void GameScene::UpdateConnectedMovement() {
	MovableBlockGimmick* activeBlock = GetActiveBlock();
	if (activeBlock == nullptr || activeBlock->IsLocked()) {
		CancelConnection();
		return;
	}

	const Vector3 desiredMove = GetPlayerInputMove();

	const float moveLengthSq =
		desiredMove.x * desiredMove.x +
		desiredMove.z * desiredMove.z;

	if (moveLengthSq <= 0.000001f) {
		return;
	}

	Vector3 diff = {
		activeBlock->GetPosition().x - player_->GetPosition().x,
		0.0f,
		activeBlock->GetPosition().z - player_->GetPosition().z,
	};

	const float distanceSq =
		diff.x * diff.x + diff.z * diff.z;

	if (distanceSq <= 0.000001f) {
		ResolvePlayerBlockOverlap(activeBlockIndex_);
		return;
	}

	const float distance = std::sqrt(distanceSq);
	Vector3 direction = {
		diff.x / distance,
		0.0f,
		diff.z / distance,
	};

	const float moveAmount =
		desiredMove.x * direction.x +
		desiredMove.z * direction.z;

	if (moveAmount > 0.0001f) {
		// ====================================================
		// 押す
		// Blockを先に動かし、その後Playerを動かす。
		// ====================================================
		MoveBlockWithCollision(
			activeBlockIndex_,
			{
				direction.x * moveAmount,
				0.0f,
				direction.z * moveAmount,
			});

		MovePlayerWithCollision(desiredMove);

	} else if (moveAmount < -0.0001f) {
		// ====================================================
		// 引く
		// Playerが実際に動けた分だけBlockを追従させる。
		// ====================================================
		const Vector3 actualPlayerMove =
			MovePlayerWithCollision(desiredMove);

		diff = {
			activeBlock->GetPosition().x - player_->GetPosition().x,
			0.0f,
			activeBlock->GetPosition().z - player_->GetPosition().z,
		};

		const float newDistanceSq =
			diff.x * diff.x + diff.z * diff.z;

		if (newDistanceSq > 0.000001f) {
			const float newDistance = std::sqrt(newDistanceSq);

			direction = {
				diff.x / newDistance,
				0.0f,
				diff.z / newDistance,
			};

			const float actualMoveAmount =
				actualPlayerMove.x * direction.x +
				actualPlayerMove.z * direction.z;

			if (actualMoveAmount < 0.0f) {
				MoveBlockWithCollision(
					activeBlockIndex_,
					{
						direction.x * actualMoveAmount,
						0.0f,
						direction.z * actualMoveAmount,
					});
			}
		}

	} else {
		// 横方向へ回り込む場合はPlayerだけ移動する
		MovePlayerWithCollision(desiredMove);
	}

	ResolvePlayerBlockOverlap(activeBlockIndex_);
}

void GameScene::ResolvePlayerBlockOverlap(int blockIndex) {
	if (blockIndex < 0 || blockIndex >= kBlockCount) {
		return;
	}

	Collision::AABB playerAABB = player_->GetAABB();
	Collision::AABB blockAABB = movableBlocks_[blockIndex]->GetAABB();

	if (!Collision::IsOverlap(playerAABB, blockAABB)) {
		return;
	}

	const float overlapX =
		(std::min)(playerAABB.max.x, blockAABB.max.x) -
		(std::max)(playerAABB.min.x, blockAABB.min.x);

	const float overlapZ =
		(std::min)(playerAABB.max.z, blockAABB.max.z) -
		(std::max)(playerAABB.min.z, blockAABB.min.z);

	if (overlapX <= 0.0f || overlapZ <= 0.0f) {
		return;
	}

	Vector3 correction = {0.0f, 0.0f, 0.0f};

	if (overlapX <= overlapZ) {
		const float sign =
			(player_->GetPosition().x < movableBlocks_[blockIndex]->GetPosition().x)
			? -1.0f
			: 1.0f;

		correction.x = sign * (overlapX + kPlayerBlockSkin);
	} else {
		const float sign =
			(player_->GetPosition().z < movableBlocks_[blockIndex]->GetPosition().z)
			? -1.0f
			: 1.0f;

		correction.z = sign * (overlapZ + kPlayerBlockSkin);
	}

	const Vector3 correctedPlayerPosition = {
		player_->GetPosition().x + correction.x,
		player_->GetPosition().y,
		player_->GetPosition().z + correction.z,
	};

	// 対象Blockだけ一時的に除外し、他の壁・Block・Doorは判定する
	if (CanPlayerMoveTo(correctedPlayerPosition, blockIndex)) {
		player_->SetPosition(correctedPlayerPosition);
		return;
	}

	// Playerを戻せない場合はBlock側を反対へ逃がす
	MoveBlockWithCollision(
		blockIndex,
		{-correction.x, 0.0f, -correction.z});
}

void GameScene::UpdateConnectionInput() {
	if (!input_->TriggerKey(DIK_E)) {
		return;
	}

	// 接続動作中・接続済みならEでもう一度解除する
	if (connectionState_ != ConnectionState::kIdle) {
		CancelConnection();
		return;
	}

	// 3個のうち、接続距離内で最も近いBlockを選択する
	activeBlockIndex_ = FindNearestConnectableBlock();

	if (activeBlockIndex_ < 0) {
		return;
	}

	connectionState_ = ConnectionState::kShooting;
	ropeShootProgress_ = 0.0f;
	movableBlocks_[activeBlockIndex_]->SetConnected(false);
	movableBlocks_[activeBlockIndex_]->SetPullingVisual(false);
}

void GameScene::UpdateRopeShot() {
	MovableBlockGimmick* activeBlock = GetActiveBlock();
	if (activeBlock == nullptr || activeBlock->IsLocked()) {
		CancelConnection();
		return;
	}

	ropeShootProgress_ += kRopeShootProgressPerFrame;
	ropeShootProgress_ = (std::min)(ropeShootProgress_, 1.0f);

	if (ropeShootProgress_ >= 1.0f) {
		connectionState_ = ConnectionState::kPulling;
		activeBlock->SetPullingVisual(true);
	}
}

void GameScene::UpdatePullTogether() {
	MovableBlockGimmick* activeBlock = GetActiveBlock();
	if (activeBlock == nullptr || activeBlock->IsLocked()) {
		CancelConnection();
		return;
	}

	Vector3 playerPosition = player_->GetPosition();
	Vector3 blockPosition = activeBlock->GetPosition();

	Vector3 diff = {
		blockPosition.x - playerPosition.x,
		0.0f,
		blockPosition.z - playerPosition.z,
	};

	float distanceXZ = std::sqrt(
		diff.x * diff.x + diff.z * diff.z);

	if (distanceXZ <= 0.0001f) {
		ResolvePlayerBlockOverlap(activeBlockIndex_);
		activeBlock->SetPullingVisual(false);
		activeBlock->SetConnected(true);
		connectionState_ = ConnectionState::kConnected;
		return;
	}

	Vector3 direction = {
		diff.x / distanceXZ,
		0.0f,
		diff.z / distanceXZ,
	};

	const float targetDistance =
		CalculateContactDistanceXZ(direction) +
		kPlayerBlockSkin;

	float remainingDistance = distanceXZ - targetDistance;

	if (remainingDistance <= 0.005f) {
		ResolvePlayerBlockOverlap(activeBlockIndex_);
		activeBlock->SetPullingVisual(false);
		activeBlock->SetConnected(true);
		connectionState_ = ConnectionState::kConnected;
		return;
	}

	// BlockとPlayerを両側から近づける
	const float blockStep = (std::min)(
		kPullSpeedPerFrame,
		remainingDistance * 0.5f);

	MoveBlockWithCollision(
		activeBlockIndex_,
		{
			-direction.x * blockStep,
			0.0f,
			-direction.z * blockStep,
		});

	playerPosition = player_->GetPosition();
	blockPosition = activeBlock->GetPosition();

	diff = {
		blockPosition.x - playerPosition.x,
		0.0f,
		blockPosition.z - playerPosition.z,
	};

	distanceXZ = std::sqrt(
		diff.x * diff.x + diff.z * diff.z);

	if (distanceXZ > 0.0001f) {
		direction = {
			diff.x / distanceXZ,
			0.0f,
			diff.z / distanceXZ,
		};
	}

	const float newTargetDistance =
		CalculateContactDistanceXZ(direction) +
		kPlayerBlockSkin;

	remainingDistance = distanceXZ - newTargetDistance;

	if (remainingDistance > 0.0f) {
		const float playerStep = (std::min)(
			kPullSpeedPerFrame,
			remainingDistance);

		MovePlayerWithCollision({
			direction.x * playerStep,
			0.0f,
			direction.z * playerStep,
		});
	}

	ResolvePlayerBlockOverlap(activeBlockIndex_);

	// 接続完了判定
	playerPosition = player_->GetPosition();
	blockPosition = activeBlock->GetPosition();

	diff = {
		blockPosition.x - playerPosition.x,
		0.0f,
		blockPosition.z - playerPosition.z,
	};

	distanceXZ = std::sqrt(
		diff.x * diff.x + diff.z * diff.z);

	if (distanceXZ > 0.0001f) {
		direction = {
			diff.x / distanceXZ,
			0.0f,
			diff.z / distanceXZ,
		};
	}

	const float finalTargetDistance =
		CalculateContactDistanceXZ(direction) +
		kPlayerBlockSkin;

	if (distanceXZ <= finalTargetDistance + 0.01f) {
		ResolvePlayerBlockOverlap(activeBlockIndex_);
		activeBlock->SetPullingVisual(false);
		activeBlock->SetConnected(true);
		connectionState_ = ConnectionState::kConnected;
	}
}

float GameScene::CalculateContactDistanceXZ(
    const Vector3& direction) const {

	const MovableBlockGimmick* activeBlock = GetActiveBlock();
	if (activeBlock == nullptr) {
		return 0.0f;
	}

	const Vector3& playerHalfSize = player_->GetHalfSize();
	const Vector3& blockHalfSize = activeBlock->GetHalfSize();

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

	if (result >= 99999.0f) {
		return sumHalfX;
	}

	return result;
}

void GameScene::CancelConnection() {
	MovableBlockGimmick* activeBlock = GetActiveBlock();
	if (activeBlock != nullptr) {
		activeBlock->SetPullingVisual(false);
		activeBlock->SetConnected(false);
	}

	connectionState_ = ConnectionState::kIdle;
	ropeShootProgress_ = 0.0f;
	activeBlockIndex_ = -1;
}

void GameScene::UpdateSwitch() {
	// 1回作動したら再判定しない
	if (switchActivated_) {
		return;
	}

	for (int i = 0; i < kBlockCount; ++i) {
		if (movableBlocks_[i]->IsLocked()) {
			continue;
		}

		if (!Collision::IsOverlapXZ(
				movableBlocks_[i]->GetAABB(),
				switchAABB_)) {
			continue;
		}

		const Vector3 snapPosition = {
			kSwitchPosition_.x,
			1.2f,
			kSwitchPosition_.z,
		};

		const Collision::AABB snapAABB = Collision::MakeAABB(
			snapPosition,
			movableBlocks_[i]->GetHalfSize());

		// スナップ先にPlayerがいる場合は、Playerが離れるまで固定しない
		if (Collision::IsOverlap(player_->GetAABB(), snapAABB)) {
			continue;
		}

		// スナップ先に別のBlockがある場合も固定しない
		bool canSnap = true;
		for (int j = 0; j < kBlockCount; ++j) {
			if (j == i) {
				continue;
			}

			if (Collision::IsOverlap(snapAABB, movableBlocks_[j]->GetAABB())) {
				canSnap = false;
				break;
			}
		}

		if (!canSnap) {
			continue;
		}

		// ----------------------------------------------------
		// Blockをスイッチ中央へ固定
		// ----------------------------------------------------
		movableBlocks_[i]->SnapAndLock(snapPosition);

		switchActivated_ = true;
		switchColor_.SetColor({1.0f, 0.85f, 0.15f, 1.0f});

		// ----------------------------------------------------
		// スイッチ作動 → 中央ドアOPEN
		// ----------------------------------------------------
		door_->Open();

		// 操作中のBlockを置いた場合は接続解除
		if (activeBlockIndex_ == i) {
			CancelConnection();
		}

		break;
	}
}

void GameScene::UpdateGoal() {
	if (isClear_) {
		return;
	}

	// 壁の向こう側のGOALへPlayerが到達したら1ステージクリア
	if (Collision::IsOverlapXZ(
			player_->GetAABB(),
			goalAABB_)) {

		isClear_ = true;
		CancelConnection();

		// クリアしたことが見た目で分かるようにGOALを黄色へ変更する
		goalColor_.SetColor({1.0f, 0.80f, 0.10f, 1.0f});
	}
}

void GameScene::UpdateRope() {
	if (connectionState_ == ConnectionState::kIdle) {
		return;
	}

	const MovableBlockGimmick* activeBlock = GetActiveBlock();
	if (activeBlock == nullptr) {
		return;
	}

	const Vector3 start = player_->GetPosition();
	const Vector3 target = activeBlock->GetPosition();

	Vector3 end = target;

	if (connectionState_ == ConnectionState::kShooting) {
		// EaseOutCubicで糸先端を勢いよく飛ばす
		const float t = ropeShootProgress_;
		const float oneMinusT = 1.0f - t;
		const float easedT =
			1.0f - oneMinusT * oneMinusT * oneMinusT;

		end = {
			start.x + (target.x - start.x) * easedT,
			start.y + (target.y - start.y) * easedT,
			start.z + (target.z - start.z) * easedT,
		};
	}

	SetRopeTransform(start, end);
}

void GameScene::SetRopeTransform(
    const Vector3& start,
    const Vector3& end) {

	const Vector3 diff = {
		end.x - start.x,
		end.y - start.y,
		end.z - start.z,
	};

	const float lengthXZ = std::sqrt(
		diff.x * diff.x + diff.z * diff.z);

	const float length = std::sqrt(
		diff.x * diff.x +
		diff.y * diff.y +
		diff.z * diff.z);

	ropeWorldTransform_.translation_ = {
		(start.x + end.x) * 0.5f,
		(start.y + end.y) * 0.5f,
		(start.z + end.z) * 0.5f,
	};

	ropeWorldTransform_.scale_ = {
		0.04f,
		0.04f,
		(std::max)(length * 0.5f, 0.001f),
	};

	ropeWorldTransform_.rotation_.y =
		std::atan2(diff.x, diff.z);

	ropeWorldTransform_.rotation_.x =
		-std::atan2(
			diff.y,
			(std::max)(lengthXZ, 0.0001f));

	ropeWorldTransform_.UpdateMatarix();
}

void GameScene::ResetGame() {
	player_->Reset(kPlayerStartPosition_);

	for (int i = 0; i < kBlockCount; ++i) {
		movableBlocks_[i]->Reset(kBlockStartPositions_[i]);
	}

	door_->Reset();

	connectionState_ = ConnectionState::kIdle;
	ropeShootProgress_ = 0.0f;
	activeBlockIndex_ = -1;

	switchActivated_ = false;
	switchColor_.SetColor({0.20f, 0.90f, 0.25f, 1.0f});

	isClear_ = false;
	goalColor_.SetColor({0.20f, 0.85f, 0.90f, 1.0f});
}
