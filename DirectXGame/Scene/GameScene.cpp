#include "GameScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void GameScene::Initialize() {
	input_ = Input::GetInstance();

	// --------------------------------------------------------
	// GameScene用モデル
	// --------------------------------------------------------
	blockModel_ = Model::CreateFromOBJ("cube");
	switchModel_ = Model::CreateFromOBJ("cube");
	floorModel_ = Model::CreateFromOBJ("cube");
	wallModel_ = Model::CreateFromOBJ("cube");
	ropeModel_ = Model::CreateFromOBJ("cube");

	// --------------------------------------------------------
	// カメラ
	// --------------------------------------------------------
	camera_.Initialize();
	camera_.translation_ = {0.0f, 18.0f, -24.0f};
	camera_.rotation_ = {0.65f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	// --------------------------------------------------------
	// プレイヤー
	// 移動・描画・当たり判定はPlayerクラス側で管理する
	// --------------------------------------------------------
	player_.Initialize(kPlayerStartPosition_);

	// --------------------------------------------------------
	// 移動ブロックギミック
	// --------------------------------------------------------
	movableBlock_.Initialize(
		blockModel_,
		kBlockStartPosition_,
		{1.0f, 1.0f, 1.0f});

	// --------------------------------------------------------
	// スイッチ
	// --------------------------------------------------------
	InitializeTransform(
		switchWorldTransform_,
		kSwitchPosition_,
		kSwitchScale_);

	switchColor_.Initialize();
	switchColor_.SetColor({0.25f, 0.9f, 0.25f, 1.0f});

	switchAABB_ = Collision::MakeAABB(
		kSwitchPosition_,
		kSwitchScale_);

	// --------------------------------------------------------
	// 床
	// --------------------------------------------------------
	InitializeTransform(
		floorWorldTransform_,
		{0.0f, -0.15f, 1.0f},
		{8.0f, 0.10f, 11.0f});

	floorColor_.Initialize();
	floorColor_.SetColor({0.55f, 0.55f, 0.55f, 1.0f});

	// --------------------------------------------------------
	// 壁
	// 0:左 / 1:右 / 2:奥 / 3:手前
	// --------------------------------------------------------
	const Vector3 wallPositions[kWallCount] = {
		{-6.0f, 1.0f, 0.0f},
		{6.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 9.0f},
		{0.0f, 1.0f, -9.0f},
	};

	const Vector3 wallScales[kWallCount] = {
		{0.5f, 1.0f, 9.0f},
		{0.5f, 1.0f, 9.0f},
		{6.5f, 1.0f, 0.5f},
		{6.5f, 1.0f, 0.5f},
	};

	for (int i = 0; i < kWallCount; ++i) {
		InitializeTransform(
			wallWorldTransforms_[i],
			wallPositions[i],
			wallScales[i]);

		wallAABBs_[i] = Collision::MakeAABB(
			wallPositions[i],
			wallScales[i]);

		// ブロック側にも同じ壁判定を登録する
		movableBlock_.AddObstacle(wallAABBs_[i]);
	}

	wallColor_.Initialize();
	wallColor_.SetColor({0.25f, 0.25f, 0.3f, 1.0f});

	// --------------------------------------------------------
	// 糸
	// --------------------------------------------------------
	InitializeTransform(
		ropeWorldTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	ropeColor_.Initialize();
	ropeColor_.SetColor({1.0f, 0.9f, 0.2f, 1.0f});

	connectionState_ = ConnectionState::kIdle;
	ropeShootProgress_ = 0.0f;
}

bool GameScene::Update() {
	// --------------------------------------------------------
	// R : デモをリセット
	// --------------------------------------------------------
	if (input_->TriggerKey(DIK_R)) {
		ResetGame();
		return true;
	}

	// --------------------------------------------------------
	// E : 接続開始 / 接続解除
	// --------------------------------------------------------
	UpdateConnectionInput();

	// --------------------------------------------------------
	// 接続状態ごとの更新
	// --------------------------------------------------------
	switch (connectionState_) {
	case ConnectionState::kIdle: {
		// 未接続時は、プレイヤーだけを通常移動させる
		const Vector3 move = GetPlayerInputMove();
		MovePlayerWithCollision(move);
		break;
	}

	case ConnectionState::kShooting:
		// 糸が伸びている間は見せやすいようにプレイヤーを停止する
		UpdateRopeShot();
		break;

	case ConnectionState::kPulling:
		// 糸が命中した後、プレイヤーとギミックを互いに近づける
		UpdatePullTogether();
		break;

	case ConnectionState::kConnected:
		// 接続中は押す／引くで更新順を切り替える。
		// 引く時にブロックだけが先にプレイヤーへ入るのを防ぐ。
		UpdateConnectedMovement();
		break;
	}

	// --------------------------------------------------------
	// 接続中に離れすぎた場合は自動解除
	// --------------------------------------------------------
	if (connectionState_ == ConnectionState::kConnected) {
		const float distanceToBlock = Collision::Distance(
			player_.GetPosition(),
			movableBlock_.GetPosition());

		if (distanceToBlock > kDisconnectDistance) {
			CancelConnection();
		}
	}

	// --------------------------------------------------------
	// スイッチ判定
	// --------------------------------------------------------
	UpdateSwitch();

	// --------------------------------------------------------
	// 念のため毎フレーム最後にめり込みを解消する
	// --------------------------------------------------------
	ResolvePlayerGimmickOverlap();

	// --------------------------------------------------------
	// 糸の表示更新
	// --------------------------------------------------------
	UpdateRope();


	return true;
}

void GameScene::Draw() {
	if (blockModel_ == nullptr || switchModel_ == nullptr ||
		floorModel_ == nullptr || wallModel_ == nullptr ||
		ropeModel_ == nullptr) {
		return;
	}

	// 床
	floorModel_->Draw(
		floorWorldTransform_,
		camera_,
		&floorColor_);

	// 壁
	for (int i = 0; i < kWallCount; ++i) {
		wallModel_->Draw(
			wallWorldTransforms_[i],
			camera_,
			&wallColor_);
	}

	// スイッチ
	switchModel_->Draw(
		switchWorldTransform_,
		camera_,
		&switchColor_);

	// プレイヤー
	player_.Draw(camera_);

	// 移動ブロックギミック
	movableBlock_.Draw(camera_);

	// 射出中・引き寄せ中・接続中は糸を表示する
	if (connectionState_ != ConnectionState::kIdle) {
		ropeModel_->Draw(
			ropeWorldTransform_,
			camera_,
			&ropeColor_);
	}
}

void GameScene::Finalize() {
	player_.Finalize();

	delete blockModel_;
	delete switchModel_;
	delete floorModel_;
	delete wallModel_;
	delete ropeModel_;

	blockModel_ = nullptr;
	switchModel_ = nullptr;
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

	// translation_ / rotation_ / scale_からmatWorld_を作り直す
	worldTransform.UpdateMatarix();
}

Vector3 GameScene::GetPlayerInputMove() const {
	// 入力処理はPlayerクラスへ委譲する
	return player_.GetInputMove();
}

Vector3 GameScene::MovePlayerWithCollision(
    const Vector3& move,
    bool ignoreGimmickCollision) {

	// 通常は移動ブロックも障害物として渡す。
	// めり込み解消時など、壁だけ確認したい時はnullptrを渡す。
	Collision::AABB blockAABB = movableBlock_.GetAABB();
	const Collision::AABB* dynamicObstacle =
		ignoreGimmickCollision ? nullptr : &blockAABB;

	return player_.MoveWithCollision(
		move,
		wallAABBs_,
		kWallCount,
		dynamicObstacle);
}

bool GameScene::CanPlayerMoveTo(
    const Vector3& position,
    bool ignoreGimmickCollision) const {

	Collision::AABB blockAABB = movableBlock_.GetAABB();
	const Collision::AABB* dynamicObstacle =
		ignoreGimmickCollision ? nullptr : &blockAABB;

	return player_.CanMoveTo(
		position,
		wallAABBs_,
		kWallCount,
		dynamicObstacle);
}

Collision::AABB GameScene::GetPlayerAABBAt(
    const Vector3& position) const {

	return player_.GetAABBAt(position);
}

void GameScene::UpdateConnectedMovement() {
	const Vector3 desiredMove = GetPlayerInputMove();

	const float moveLengthSq =
		desiredMove.x * desiredMove.x +
		desiredMove.z * desiredMove.z;

	if (moveLengthSq <= 0.000001f) {
		return;
	}

	Vector3 diff = {
		movableBlock_.GetPosition().x - player_.GetPosition().x,
		0.0f,
		movableBlock_.GetPosition().z - player_.GetPosition().z,
	};

	const float distanceSq =
		diff.x * diff.x +
		diff.z * diff.z;

	if (distanceSq <= 0.000001f) {
		ResolvePlayerGimmickOverlap();
		return;
	}

	const float distance = std::sqrt(distanceSq);
	Vector3 direction = {
		diff.x / distance,
		0.0f,
		diff.z / distance,
	};

	// プレイヤーの入力が「プレイヤー→ブロック方向」に
	// どれだけ向いているかを求める。
	const float moveAmount =
		desiredMove.x * direction.x +
		desiredMove.z * direction.z;

	if (moveAmount > 0.0001f) {
		// ====================================================
		// 押す
		//
		// 先にブロックを逃がし、その後プレイヤーを動かす。
		// ブロックが壁で止まった場合はプレイヤー側の当たり判定で
		// ブロック内部へ入らないようにする。
		// ====================================================
		movableBlock_.MoveBy({
			direction.x * moveAmount,
			0.0f,
			direction.z * moveAmount,
		});

		MovePlayerWithCollision(desiredMove);

	} else if (moveAmount < -0.0001f) {
		// ====================================================
		// 引く
		//
		// 以前はブロックを先にプレイヤー側へ動かしていたため、
		// プレイヤーが壁などで動けない時にブロックだけが進み、
		// めり込む場合があった。
		//
		// 先にプレイヤーを動かし、「実際に動けた量」だけ
		// ブロックを追従させる。
		// ====================================================
		const Vector3 actualPlayerMove =
			MovePlayerWithCollision(desiredMove);

		// プレイヤー移動後の方向を取り直す
		diff = {
			movableBlock_.GetPosition().x - player_.GetPosition().x,
			0.0f,
			movableBlock_.GetPosition().z - player_.GetPosition().z,
		};

		const float newDistanceSq =
			diff.x * diff.x +
			diff.z * diff.z;

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
				movableBlock_.MoveBy({
					direction.x * actualMoveAmount,
					0.0f,
					direction.z * actualMoveAmount,
				});
			}
		}

	} else {
		// ブロックに対して横方向へ動いている時は
		// プレイヤーだけ通常の当たり判定付きで移動する。
		MovePlayerWithCollision(desiredMove);
	}

	ResolvePlayerGimmickOverlap();
}

void GameScene::ResolvePlayerGimmickOverlap() {
	Collision::AABB playerAABB =
		GetPlayerAABBAt(player_.GetPosition());

	Collision::AABB blockAABB =
		movableBlock_.GetAABB();

	if (!Collision::IsOverlap(playerAABB, blockAABB)) {
		return;
	}

	// --------------------------------------------------------
	// X/Zそれぞれのめり込み量を求め、
	// 小さい方の軸へ押し戻す。
	// --------------------------------------------------------
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
		// プレイヤーがブロックの左なら左へ、右なら右へ戻す
		const float sign =
			(player_.GetPosition().x <
			 movableBlock_.GetPosition().x)
			? -1.0f
			: 1.0f;

		correction.x =
			sign * (overlapX + kPlayerBlockSkin);

	} else {
		// プレイヤーがブロックの手前なら手前へ、奥なら奥へ戻す
		const float sign =
			(player_.GetPosition().z <
			 movableBlock_.GetPosition().z)
			? -1.0f
			: 1.0f;

		correction.z =
			sign * (overlapZ + kPlayerBlockSkin);
	}

	// --------------------------------------------------------
	// まずプレイヤー側を押し戻す。
	// 壁に邪魔されて押し戻せない時だけブロック側を逆へ逃がす。
	// --------------------------------------------------------
	const Vector3 correctedPlayerPosition = {
		player_.GetPosition().x + correction.x,
		player_.GetPosition().y,
		player_.GetPosition().z + correction.z,
	};

	if (CanPlayerMoveTo(correctedPlayerPosition, true)) {
		player_.SetPosition(correctedPlayerPosition);
		return;
	}

	// プレイヤーが壁際などで動けない場合
	// ブロックを反対方向へ移動して重なりを解消する。
	movableBlock_.MoveBy({
		-correction.x,
		0.0f,
		-correction.z,
	});
}

void GameScene::UpdateConnectionInput() {
	// 固定済みのギミックには接続しない
	if (movableBlock_.IsLocked()) {
		CancelConnection();
		return;
	}

	if (!input_->TriggerKey(DIK_E)) {
		return;
	}

	// 接続動作中または接続済みなら、Eでもう一度解除する
	if (connectionState_ != ConnectionState::kIdle) {
		CancelConnection();
		return;
	}

	const float distanceToBlock = Collision::Distance(
		player_.GetPosition(),
		movableBlock_.GetPosition());

	// 接続可能距離内なら糸の射出を開始する
	if (distanceToBlock <= kConnectDistance) {
		connectionState_ = ConnectionState::kShooting;
		ropeShootProgress_ = 0.0f;
		movableBlock_.SetConnected(false);
		movableBlock_.SetPullingVisual(false);
	}
}

void GameScene::UpdateRopeShot() {
	// --------------------------------------------------------
	// 糸の先端を0.0 → 1.0まで進める
	// 描画側ではこの値を使ってプレイヤーからギミックへ糸を伸ばす。
	// --------------------------------------------------------
	ropeShootProgress_ += kRopeShootProgressPerFrame;
	ropeShootProgress_ = (std::min)(ropeShootProgress_, 1.0f);

	// 糸の先端がギミックへ到達したら引き寄せフェーズへ移る
	if (ropeShootProgress_ >= 1.0f) {
		connectionState_ = ConnectionState::kPulling;
		movableBlock_.SetPullingVisual(true);
	}
}

void GameScene::UpdatePullTogether() {
	Vector3 playerPosition = player_.GetPosition();
	Vector3 blockPosition = movableBlock_.GetPosition();

	Vector3 diff = {
		blockPosition.x - playerPosition.x,
		0.0f,
		blockPosition.z - playerPosition.z,
	};

	float distanceXZ = std::sqrt(
		diff.x * diff.x + diff.z * diff.z);

	if (distanceXZ <= 0.0001f) {
		ResolvePlayerGimmickOverlap();

		movableBlock_.SetPullingVisual(false);
		movableBlock_.SetConnected(true);
		connectionState_ = ConnectionState::kConnected;
		return;
	}

	Vector3 direction = {
		diff.x / distanceXZ,
		0.0f,
		diff.z / distanceXZ,
	};

	// --------------------------------------------------------
	// AABBが完全に接触する位置より少しだけ手前を目標にする。
	//
	// ピッタリ0距離まで寄せると浮動小数点誤差で
	// 1フレームだけ重なってしまう場合があるため、
	// kPlayerBlockSkin分だけ隙間を残す。
	// --------------------------------------------------------
	const float targetDistance =
		CalculateContactDistanceXZ(direction) +
		kPlayerBlockSkin;

	float remainingDistance =
		distanceXZ - targetDistance;

	if (remainingDistance <= 0.005f) {
		ResolvePlayerGimmickOverlap();

		movableBlock_.SetPullingVisual(false);
		movableBlock_.SetConnected(true);
		connectionState_ = ConnectionState::kConnected;
		return;
	}

	// --------------------------------------------------------
	// まずギミックをプレイヤー側へ動かす。
	// 残り距離の半分までに制限することで、
	// プレイヤーが壁で止まっていても一気に貫通しない。
	// --------------------------------------------------------
	const float blockStep = (std::min)(
		kPullSpeedPerFrame,
		remainingDistance * 0.5f);

	movableBlock_.MoveBy({
		-direction.x * blockStep,
		0.0f,
		-direction.z * blockStep,
	});

	// --------------------------------------------------------
	// ギミック移動後の距離を再計算する
	// --------------------------------------------------------
	playerPosition = player_.GetPosition();
	blockPosition = movableBlock_.GetPosition();

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

	remainingDistance =
		distanceXZ - newTargetDistance;

	if (remainingDistance > 0.0f) {
		const float playerStep = (std::min)(
			kPullSpeedPerFrame,
			remainingDistance);

		// ギミック判定を無視しない。
		// 糸演出中でもプレイヤーがブロック内部へ入ることを防ぐ。
		MovePlayerWithCollision({
			direction.x * playerStep,
			0.0f,
			direction.z * playerStep,
		});
	}

	// 念のため重なりが発生していた場合は押し戻す
	ResolvePlayerGimmickOverlap();

	// --------------------------------------------------------
	// 接続完了判定
	// --------------------------------------------------------
	playerPosition = player_.GetPosition();
	blockPosition = movableBlock_.GetPosition();

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
		ResolvePlayerGimmickOverlap();

		movableBlock_.SetPullingVisual(false);
		movableBlock_.SetConnected(true);
		connectionState_ = ConnectionState::kConnected;
	}
}

float GameScene::CalculateContactDistanceXZ(
    const Vector3& direction) const {

	const Vector3& playerHalfSize = player_.GetHalfSize();
	const Vector3& blockHalfSize = movableBlock_.GetHalfSize();

	const float sumHalfX = playerHalfSize.x + blockHalfSize.x;
	const float sumHalfZ = playerHalfSize.z + blockHalfSize.z;

	const float absX = std::abs(direction.x);
	const float absZ = std::abs(direction.z);

	// --------------------------------------------------------
	// AABB同士が移動方向上で最初に接触する中心間距離を求める。
	// XまたはZのどちらか一方が境界に達した時点が接触となる。
	// --------------------------------------------------------
	float contactX = 100000.0f;
	float contactZ = 100000.0f;

	if (absX > 0.0001f) {
		contactX = sumHalfX / absX;
	}

	if (absZ > 0.0001f) {
		contactZ = sumHalfZ / absZ;
	}

	const float result = (std::min)(contactX, contactZ);

	// 方向がほぼ0の場合の保険
	if (result >= 99999.0f) {
		return sumHalfX;
	}

	return result;
}

void GameScene::CancelConnection() {
	connectionState_ = ConnectionState::kIdle;
	ropeShootProgress_ = 0.0f;
	movableBlock_.SetPullingVisual(false);
	movableBlock_.SetConnected(false);
}

void GameScene::UpdateSwitch() {
	// すでに固定済みなら判定不要
	if (movableBlock_.IsLocked()) {
		return;
	}

	// XZ平面でブロックとスイッチが重なったら成功
	if (Collision::IsOverlapXZ(
			movableBlock_.GetAABB(),
			switchAABB_)) {

		// スイッチ中央へ正確に配置し、その場に固定する
		movableBlock_.SnapAndLock({
			kSwitchPosition_.x,
			1.1f,
			kSwitchPosition_.z,
		});

		// 固定時は糸も消す
		connectionState_ = ConnectionState::kIdle;
		ropeShootProgress_ = 0.0f;
	}
}

void GameScene::UpdateRope() {
	if (connectionState_ == ConnectionState::kIdle) {
		return;
	}

	const Vector3 start = player_.GetPosition();
	const Vector3 target = movableBlock_.GetPosition();

	Vector3 end = target;

	if (connectionState_ == ConnectionState::kShooting) {
		// ----------------------------------------------------
		// EaseOutCubicで先端を少し勢いよく飛ばす
		// ----------------------------------------------------
		const float t = ropeShootProgress_;
		const float oneMinusT = 1.0f - t;
		const float easedT = 1.0f - oneMinusT * oneMinusT * oneMinusT;

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

	// 糸の中心は始点と終点の中間
	ropeWorldTransform_.translation_ = {
		(start.x + end.x) * 0.5f,
		(start.y + end.y) * 0.5f,
		(start.z + end.z) * 0.5f,
	};

	// cubeをZ方向へ細長く伸ばして糸として見せる
	ropeWorldTransform_.scale_ = {
		0.04f,
		0.04f,
		(std::max)(length * 0.5f, 0.001f),
	};

	// Z軸を始点→終点方向へ向ける
	ropeWorldTransform_.rotation_.y =
		std::atan2(diff.x, diff.z);

	// 高さの違いにも追従できるようにX回転も付ける
	ropeWorldTransform_.rotation_.x =
		-std::atan2(diff.y, (std::max)(lengthXZ, 0.0001f));

	ropeWorldTransform_.UpdateMatarix();
}

void GameScene::ResetGame() {
	player_.Reset(kPlayerStartPosition_);
	movableBlock_.Reset(kBlockStartPosition_);

	connectionState_ = ConnectionState::kIdle;
	ropeShootProgress_ = 0.0f;
}
