#include "GameScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void GameScene::Initialize() {
	input_ = Input::GetInstance();

	camera_ = new Camera();
	player_ = new Player();
	door_ = new SwitchDoorGimmick();

	power_ = new PowerGimmick();

	// --------------------------------------------------------
	// モデル
	// --------------------------------------------------------
	switchModel_ = Model::CreateFromOBJ("cube"); // 電源の見た目に使う
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
	// 中央ドア
	// --------------------------------------------------------
	door_->Initialize(
		doorModel_,
		kDoorPosition_,
		kDoorScale_,
		4.0f);

	// --------------------------------------------------------
	// 電源
	// --------------------------------------------------------
	power_->Initialize(
		switchModel_, // cubeを電源の見た目に使用
		kPowerPosition_,
		kPowerScale_);

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
	// 糸（電源<->装置用）
	// --------------------------------------------------------
	InitializeTransform(
		deviceRopeWorldTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	deviceRopeColor_.Initialize();
	deviceRopeColor_.SetColor({0.95f, 0.45f, 0.95f, 1.0f}); // 電源間は薄紫系

	deviceSelecting_ = false;
	deviceConnected_ = false;
	selectedDeviceType_ = DeviceType::None;

	isClear_ = false;
}

bool GameScene::Update() {
	// R : ステージを最初からやり直す
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

	// デバイス間接続用入力（Fキー）
	UpdateDeviceConnectionInput();

	UpdateDeviceRope();

	// プレイヤー移動
	const Vector3 move = GetPlayerInputMove();
	MovePlayerWithCollision(move);

	// ゴール判定
	UpdateGoal();

	return true;
}

void GameScene::Draw() {
	if (player_ == nullptr || camera_ == nullptr || door_ == nullptr ||
		switchModel_ == nullptr || doorModel_ == nullptr || goalModel_ == nullptr ||
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

	// ドア
	door_->Draw(*camera_);

	// 電源
	power_->Draw(*camera_);

	// Player
	player_->Draw(*camera_);

	// 電源と装置の接続糸（存在すれば描画）
	if (deviceSelecting_ || deviceConnected_) {
		ropeModel_->Draw(
			deviceRopeWorldTransform_,
			*camera_,
			&deviceRopeColor_);
	}
}

void GameScene::Finalize() {
	if (player_ != nullptr) {
		player_->Finalize();
		delete player_;
		player_ = nullptr;
	}

	delete door_;
	door_ = nullptr;

	delete camera_;
	camera_ = nullptr;

	delete power_;
	power_ = nullptr;

	delete switchModel_;
	delete doorModel_;
	delete goalModel_;
	delete floorModel_;
	delete wallModel_;
	delete ropeModel_;

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
    int maxCount) const {

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

	return count;
}

Vector3 GameScene::MovePlayerWithCollision(
    const Vector3& move) {

	Collision::AABB obstacles[kMaxPlayerObstacles]{};
	const int obstacleCount = BuildPlayerObstacleList(
		obstacles,
		kMaxPlayerObstacles);

	return player_->MoveWithCollision(
		move,
		obstacles,
		obstacleCount,
		nullptr);
}

bool GameScene::CanPlayerMoveTo(
    const Vector3& position) const {

	Collision::AABB obstacles[kMaxPlayerObstacles]{};
	const int obstacleCount = BuildPlayerObstacleList(
		obstacles,
		kMaxPlayerObstacles);

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

void GameScene::UpdateGoal() {
	if (isClear_) {
		return;
	}

	// 壁の向こう側のGOALへPlayerが到達したら1ステージクリア
	if (Collision::IsOverlapXZ(
			player_->GetAABB(),
			goalAABB_)) {

		isClear_ = true;

		// クリアしたことが見た目で分かるようにGOALを黄色へ変更する
		goalColor_.SetColor({1.0f, 0.80f, 0.10f, 1.0f});
	}
}

void GameScene::ResetGame() {
	player_->Reset(kPlayerStartPosition_);

	door_->Reset();

	deviceSelecting_ = false;
	deviceConnected_ = false;
	selectedDeviceType_ = DeviceType::None;
	power_->Reset(kPowerPosition_);
	door_->Reset();

	isClear_ = false;
	goalColor_.SetColor({0.20f, 0.85f, 0.90f, 1.0f});
}

void GameScene::UpdateDeviceConnectionInput() {
	// Fキーでデバイス選択 / 接続 / 解除
	if (!input_->TriggerKey(DIK_E)) {
		return;
	}

	// 既に接続が確立している場合：Eで切断（ドアの電源を切る）
	if (deviceConnected_) {
		deviceConnected_ = false;
		// 電源 -> ドア の接続と仮定してドアをリセット（給電解除）
		door_->SetPowered(false);
		return;
	}

	// 1) まだ1つも選んでいない：近くの電源またはドアを選択する
	if (!deviceSelecting_) {
		// 電源に近ければ電源を選択
		if (Collision::Distance(player_->GetPosition(), power_->GetPosition()) <= kConnectDistance) {
			deviceSelecting_ = true;
			selectedDeviceType_ = DeviceType::Power;
			power_->SetSelected(true);
			return;
		}

		// ドアに近ければドアを選択
		const Collision::AABB doorAABB = door_->GetAABB();
		const Vector3 doorPos = { (doorAABB.min.x + doorAABB.max.x) * 0.5f,
								  (doorAABB.min.y + doorAABB.max.y) * 0.5f,
								  (doorAABB.min.z + doorAABB.max.z) * 0.5f };
		if (Collision::Distance(player_->GetPosition(), doorPos) <= kConnectDistance) {
			deviceSelecting_ = true;
			selectedDeviceType_ = DeviceType::Door;
			return;
		}

		// 何も選べなかった
		return;
	}

	// 2) 既に1つ選んでいる：2つ目を選んで接続を確立する
	if (deviceSelecting_) {
		if (selectedDeviceType_ == DeviceType::Power) {
			// 2つ目がドアなら接続成立
			const Collision::AABB doorAABB = door_->GetAABB();
			const Vector3 doorPos = { (doorAABB.min.x + doorAABB.max.x) * 0.5f,
									  (doorAABB.min.y + doorAABB.max.y) * 0.5f,
									  (doorAABB.min.z + doorAABB.max.z) * 0.5f };
			if (Collision::Distance(player_->GetPosition(), doorPos) <= kConnectDistance) {
				// 接続成立：電源からドアへ給電
				deviceConnected_ = true;
				deviceSelecting_ = false;
				selectedDeviceType_ = DeviceType::None;
				power_->SetSelected(false);
				door_->SetPowered(true);
				return;
			}
		} else if (selectedDeviceType_ == DeviceType::Door) {
			// 2つ目が電源なら接続成立
			if (Collision::Distance(player_->GetPosition(), power_->GetPosition()) <= kConnectDistance) {
				deviceConnected_ = true;
				deviceSelecting_ = false;
				selectedDeviceType_ = DeviceType::None;
				door_->SetPowered(true);
				return;
			}
		}

		// 2つ目が見つからない/遠い場合は選択解除
		deviceSelecting_ = false;
		selectedDeviceType_ = DeviceType::None;
		power_->SetSelected(false);
	}
}

void GameScene::UpdateDeviceRope() {
	// 接続中（両端確定）なら電源 <-> ドア間の糸を描画
	if (deviceConnected_) {
		const Vector3 start = power_->GetPosition();
		const Collision::AABB doorAABB = door_->GetAABB();
		const Vector3 end = {
			(doorAABB.min.x + doorAABB.max.x) * 0.5f,
			(doorAABB.min.y + doorAABB.max.y) * 0.5f,
			(doorAABB.min.z + doorAABB.max.z) * 0.5f,
		};
		SetDeviceRopeTransform(start, end);
		return;
	}

	// 選択中は選択対象 <-> Player の糸を描画して接続先へ誘導する
	if (deviceSelecting_) {
		if (selectedDeviceType_ == DeviceType::Power) {
			const Vector3 start = power_->GetPosition();
			const Vector3 end = player_->GetPosition();
			SetDeviceRopeTransform(start, end);
		} else if (selectedDeviceType_ == DeviceType::Door) {
			const Collision::AABB doorAABB = door_->GetAABB();
			const Vector3 start = {
				(doorAABB.min.x + doorAABB.max.x) * 0.5f,
				(doorAABB.min.y + doorAABB.max.y) * 0.5f,
				(doorAABB.min.z + doorAABB.max.z) * 0.5f,
			};
			const Vector3 end = player_->GetPosition();
			SetDeviceRopeTransform(start, end);
		}
	}
}

void GameScene::SetDeviceRopeTransform(
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

	deviceRopeWorldTransform_.translation_ = {
		(start.x + end.x) * 0.5f,
		(start.y + end.y) * 0.5f,
		(start.z + end.z) * 0.5f,
	};

	deviceRopeWorldTransform_.scale_ = {
		0.04f,
		0.04f,
		(std::max)(length * 0.5f, 0.001f),
	};

	deviceRopeWorldTransform_.rotation_.y =
		std::atan2(diff.x, diff.z);

	deviceRopeWorldTransform_.rotation_.x =
		-std::atan2(
			diff.y,
			(std::max)(lengthXZ, 0.0001f));

	deviceRopeWorldTransform_.UpdateMatarix();
}
