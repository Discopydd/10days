#include "GameScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void GameScene::Initialize() {
	input_ = Input::GetInstance();

	// --------------------------------------------------------
	// GameScene用モデル
	// 同じcube.objを使うが、描画確認を分かりやすくするため
	// オブジェクトごとにModelを分けて生成する。
	// --------------------------------------------------------
	playerModel_ = Model::CreateSphere(12, 12);
	blockModel_ = Model::CreateFromOBJ("cube");
	switchModel_ = Model::CreateFromOBJ("cube");
	floorModel_ = Model::CreateFromOBJ("cube");
	wallModel_ = Model::CreateFromOBJ("cube");
	ropeModel_ = Model::CreateFromOBJ("cube");

	// --------------------------------------------------------
	// カメラ
	// --------------------------------------------------------
	camera_.Initialize();
	// 全体が確実に画面へ入るように、少し高い位置から見下ろす。
	// 前の設定ではプレイヤーやスイッチが画面外／重なりやすかったため、
	// 現在のGameSceneでは見やすさを優先している。
	camera_.translation_ = {0.0f, 18.0f, -24.0f};
	camera_.rotation_ = {0.65f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	// --------------------------------------------------------
	// プレイヤー
	// --------------------------------------------------------
	InitializeTransform(
		playerWorldTransform_,
		kPlayerStartPosition_,
		{0.6f, 0.6f, 0.6f});

	playerColor_.Initialize();
	playerColor_.SetColor({0.2f, 0.7f, 1.0f, 1.0f});

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
	InitializeTransform(
		wallWorldTransforms_[0],
		{-6.0f, 1.0f, 0.0f},
		{0.5f, 1.0f, 9.0f});

	InitializeTransform(
		wallWorldTransforms_[1],
		{6.0f, 1.0f, 0.0f},
		{0.5f, 1.0f, 9.0f});

	InitializeTransform(
		wallWorldTransforms_[2],
		{0.0f, 1.0f, 9.0f},
		{6.5f, 1.0f, 0.5f});

	InitializeTransform(
		wallWorldTransforms_[3],
		{0.0f, 1.0f, -9.0f},
		{6.5f, 1.0f, 0.5f});

	wallColor_.Initialize();
	wallColor_.SetColor({0.25f, 0.25f, 0.3f, 1.0f});

	// ブロックの当たり判定へ壁を登録する
	movableBlock_.AddObstacle(
		Collision::MakeAABB(
			{-6.0f, 1.0f, 0.0f},
			{0.5f, 1.0f, 9.0f}));

	movableBlock_.AddObstacle(
		Collision::MakeAABB(
			{6.0f, 1.0f, 0.0f},
			{0.5f, 1.0f, 9.0f}));

	movableBlock_.AddObstacle(
		Collision::MakeAABB(
			{0.0f, 1.0f, 9.0f},
			{6.5f, 1.0f, 0.5f}));

	movableBlock_.AddObstacle(
		Collision::MakeAABB(
			{0.0f, 1.0f, -9.0f},
			{6.5f, 1.0f, 0.5f}));

	// --------------------------------------------------------
	// 接続中の糸
	// --------------------------------------------------------
	InitializeTransform(
		ropeWorldTransform_,
		{0.0f, 1.0f, 0.0f},
		{0.04f, 0.04f, 1.0f});

	ropeColor_.Initialize();
	ropeColor_.SetColor({1.0f, 0.9f, 0.2f, 1.0f});
}

bool GameScene::Update() {
	// ESCで終了する
	if (input_->TriggerKey(DIK_ESCAPE)) {
		return false;
	}

	// RでGameSceneを最初からやり直す
	if (input_->TriggerKey(DIK_R)) {
		ResetGame();
	}

	// プレイヤーを動かし、このフレームの移動量を受け取る
	const Vector3 playerMoveDelta = UpdatePlayer();

	// Eキーによるブロック接続 / 解除
	UpdateConnection();

	// 接続中だけブロックを押す / 引く
	movableBlock_.Update(
		playerWorldTransform_.translation_,
		playerMoveDelta);

	// スイッチに置かれたらスナップして固定する
	UpdateSwitch();

	// 接続中の糸を更新する
	UpdateRope();

	playerWorldTransform_.UpdateMatarix();

	return true;
}

void GameScene::Draw() {
	// 必要なモデルが生成できていない場合は描画しない
	if (playerModel_ == nullptr || blockModel_ == nullptr ||
		switchModel_ == nullptr || floorModel_ == nullptr ||
		wallModel_ == nullptr || ropeModel_ == nullptr) {
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
	playerModel_->Draw(
		playerWorldTransform_,
		camera_,
		&playerColor_);

	// 移動ブロックギミック
	movableBlock_.Draw(camera_);

	// 接続中だけ糸を表示する
	if (movableBlock_.IsConnected()) {
		ropeModel_->Draw(
			ropeWorldTransform_,
			camera_,
			&ropeColor_);
	}
}

void GameScene::Finalize() {
	delete playerModel_;
	delete blockModel_;
	delete switchModel_;
	delete floorModel_;
	delete wallModel_;
	delete ropeModel_;

	playerModel_ = nullptr;
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

Vector3 GameScene::UpdatePlayer() {
	const Vector3 oldPosition =
		playerWorldTransform_.translation_;

	// WASDで簡易プレイヤーを移動する
	if (input_->PushKey(DIK_W)) {
		playerWorldTransform_.translation_.z += kPlayerSpeed;
	}
	if (input_->PushKey(DIK_S)) {
		playerWorldTransform_.translation_.z -= kPlayerSpeed;
	}
	if (input_->PushKey(DIK_A)) {
		playerWorldTransform_.translation_.x -= kPlayerSpeed;
	}
	if (input_->PushKey(DIK_D)) {
		playerWorldTransform_.translation_.x += kPlayerSpeed;
	}

	// 現在は簡易実装なので、プレイヤーだけはステージ外へ出ないよう簡易制限する
	playerWorldTransform_.translation_.x =
		std::clamp(playerWorldTransform_.translation_.x, -5.0f, 5.0f);

	playerWorldTransform_.translation_.z =
		std::clamp(playerWorldTransform_.translation_.z, -8.0f, 8.0f);

	// このフレームで実際に移動した量を返す
	return {
		playerWorldTransform_.translation_.x - oldPosition.x,
		playerWorldTransform_.translation_.y - oldPosition.y,
		playerWorldTransform_.translation_.z - oldPosition.z,
	};
}

void GameScene::UpdateConnection() {
	const float distanceToBlock =
		Collision::Distance(
			playerWorldTransform_.translation_,
			movableBlock_.GetPosition());

	// --------------------------------------------------------
	// E : 接続 / 解除
	//
	// ここは現在のGameScene用の簡易接続処理。
	// 本番では接続システム担当が対象判定後に
	// movableBlock_.SetConnected(true / false) を呼べばよい。
	// --------------------------------------------------------
	if (input_->TriggerKey(DIK_E) && !movableBlock_.IsLocked()) {
		if (movableBlock_.IsConnected()) {
			movableBlock_.SetConnected(false);
		} else if (distanceToBlock <= kConnectDistance) {
			movableBlock_.SetConnected(true);
		}
	}

	// 接続したまま離れすぎた場合は自動解除する
	if (movableBlock_.IsConnected() &&
		distanceToBlock > kDisconnectDistance) {

		movableBlock_.SetConnected(false);
	}
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
	}
}

void GameScene::UpdateRope() {
	if (!movableBlock_.IsConnected()) {
		return;
	}

	const Vector3& playerPosition =
		playerWorldTransform_.translation_;

	const Vector3& blockPosition =
		movableBlock_.GetPosition();

	const Vector3 diff = {
		blockPosition.x - playerPosition.x,
		blockPosition.y - playerPosition.y,
		blockPosition.z - playerPosition.z,
	};

	const float lengthXZ =
		std::sqrt(diff.x * diff.x + diff.z * diff.z);

	// 糸の中心はプレイヤーとブロックの中間
	ropeWorldTransform_.translation_ = {
		(playerPosition.x + blockPosition.x) * 0.5f,
		(playerPosition.y + blockPosition.y) * 0.5f,
		(playerPosition.z + blockPosition.z) * 0.5f,
	};

	// cubeをZ方向へ細長く伸ばして糸として見せる
	ropeWorldTransform_.scale_ = {
		0.04f,
		0.04f,
		lengthXZ * 0.5f,
	};

	// Z軸をプレイヤー→ブロック方向へ向ける
	ropeWorldTransform_.rotation_.y =
		std::atan2(diff.x, diff.z);

	ropeWorldTransform_.UpdateMatarix();
}

void GameScene::ResetGame() {
	playerWorldTransform_.translation_ =
		kPlayerStartPosition_;

	movableBlock_.Reset(kBlockStartPosition_);

	playerWorldTransform_.UpdateMatarix();
}
