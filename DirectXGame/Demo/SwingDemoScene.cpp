#include "SwingDemoScene.h"

#include <cmath>

using namespace KamataEngine;

void SwingDemoScene::Initialize() {
	input_ = Input::GetInstance();

	// --------------------------------------------------------
	// デモに必要なモデルを生成する
	// --------------------------------------------------------
	// 現在は外部モデルに依存しないよう、プレイヤーを球体で生成する。
	playerModel_ = Model::CreateSphere(12, 12);
	anchorModel_ = Model::CreateSphere(12, 12);
	floorModel_ = Model::CreateFromOBJ("cube");
	ropeModel_ = Model::CreateFromOBJ("cube");

	// --------------------------------------------------------
	// アンカーとスウィング範囲が見える位置へカメラを配置する
	// --------------------------------------------------------
	camera_.Initialize();
	camera_.translation_ = {0.0f, 7.0f, -22.0f};
	camera_.rotation_ = {0.08f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	// --------------------------------------------------------
	// プレイヤー・アンカー・床・ロープのTransformを初期化する
	// ロープは後からZ方向へ伸ばして2点間を結ぶ
	// --------------------------------------------------------
	InitializeTransform(
		playerTransform_, kPlayerStartPosition_, {0.6f, 0.6f, 0.6f});
	InitializeTransform(
		anchorTransform_, kAnchorPosition_, {0.45f, 0.45f, 0.45f});
	InitializeTransform(
		floorTransform_, {0.0f, -0.15f, 0.0f}, {12.0f, 0.15f, 6.0f});
	InitializeTransform(
		ropeTransform_, {}, {0.04f, 0.04f, 1.0f});

	// オブジェクトを見分けやすい色に設定する
	playerColor_.Initialize();
	playerColor_.SetColor({0.2f, 0.7f, 1.0f, 1.0f});
	anchorColor_.Initialize();
	anchorColor_.SetColor({1.0f, 0.25f, 0.2f, 1.0f});
	floorColor_.Initialize();
	floorColor_.SetColor({0.35f, 0.38f, 0.42f, 1.0f});
	ropeColor_.Initialize();
	ropeColor_.SetColor({1.0f, 0.9f, 0.2f, 1.0f});

	// --------------------------------------------------------
	// スウィング物理の調整値を設定する
	// --------------------------------------------------------
	AnchorSwingGimmick::Settings settings{};
	settings.connectDistance = 8.0f; // アンカーへ接続できる最大距離
	settings.ropeLength = 7.0f;      // 接続中の最大ロープ長
	settings.gravity = kGravity;
	settings.swingAssist = 5.0f;     // WASDによる接線方向の補助加速
	settings.maxSpeed = 25.0f;       // スウィング中の速度上限
	anchorSwing_.Initialize(settings);

	ResetDemo();
}

bool SwingDemoScene::Update() {
	// ESCキーでデモを終了する
	if (input_->TriggerKey(DIK_ESCAPE)) {
		return false;
	}

	// Rキーで位置・速度・接続状態をリセットする
	if (input_->TriggerKey(DIK_R)) {
		ResetDemo();
	}

	// 接続判定を先に行い、同じフレームから物理へ反映する
	UpdateConnection();
	UpdatePlayer();
	UpdateRope();

	// 更新後のプレイヤー座標をGPUへ転送する
	playerTransform_.UpdateMatarix();
	return true;
}

void SwingDemoScene::Draw() {
	// モデル生成に失敗している場合は描画しない
	if (playerModel_ == nullptr || anchorModel_ == nullptr ||
	    floorModel_ == nullptr || ropeModel_ == nullptr) {
		return;
	}

	floorModel_->Draw(floorTransform_, camera_, &floorColor_);
	anchorModel_->Draw(anchorTransform_, camera_, &anchorColor_);
	playerModel_->Draw(playerTransform_, camera_, &playerColor_);

	// ロープはアンカーへ接続している間だけ表示する
	if (anchorSwing_.IsConnected()) {
		ropeModel_->Draw(ropeTransform_, camera_, &ropeColor_);
	}
}

void SwingDemoScene::Finalize() {
	delete playerModel_;
	delete anchorModel_;
	delete floorModel_;
	delete ropeModel_;

	playerModel_ = nullptr;
	anchorModel_ = nullptr;
	floorModel_ = nullptr;
	ropeModel_ = nullptr;
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

Vector3 SwingDemoScene::GetMoveVelocity() const {
	Vector3 move{};

	// WASD入力をXZ平面上の方向ベクトルとしてまとめる
	if (input_->PushKey(DIK_A)) {
		move.x -= 1.0f;
	}
	if (input_->PushKey(DIK_D)) {
		move.x += 1.0f;
	}
	if (input_->PushKey(DIK_W)) {
		move.z += 1.0f;
	}
	if (input_->PushKey(DIK_S)) {
		move.z -= 1.0f;
	}

	// 斜め入力でも速度が速くならないよう正規化する
	const float length = std::sqrt(move.x * move.x + move.z * move.z);
	if (length > 0.0f) {
		move.x = move.x / length * kMoveSpeed;
		move.z = move.z / length * kMoveSpeed;
	}

	return move;
}

void SwingDemoScene::UpdatePlayer() {
	const Vector3 moveVelocity = GetMoveVelocity();

	if (anchorSwing_.IsConnected()) {
		// 接続中はギミック側で重力・補助加速・ロープ距離制限を適用する
		anchorSwing_.Update(
			playerTransform_.translation_,
			playerVelocity_,
			moveVelocity,
			kDeltaTime);
	} else {
		// ----------------------------------------------------
		// 未接続中の自由移動
		//
		// 水平速度を自動減衰させないため、接続解除時に持っていた
		// スウィング速度がそのまま慣性飛行へ引き継がれる。
		// ----------------------------------------------------
		const float moveLength = std::sqrt(
			moveVelocity.x * moveVelocity.x +
			moveVelocity.z * moveVelocity.z);

		// WASD入力がある場合だけ、その方向へ追加加速する
		if (moveLength > 0.0f) {
			playerVelocity_.x +=
				moveVelocity.x / moveLength * kFreeMoveAcceleration * kDeltaTime;
			playerVelocity_.z +=
				moveVelocity.z / moveLength * kFreeMoveAcceleration * kDeltaTime;
		}

		// 入力を続けても水平速度が際限なく増えないよう制限する
		const float horizontalSpeed = std::sqrt(
			playerVelocity_.x * playerVelocity_.x +
			playerVelocity_.z * playerVelocity_.z);
		if (horizontalSpeed > kMaxFreeHorizontalSpeed) {
			const float scale = kMaxFreeHorizontalSpeed / horizontalSpeed;
			playerVelocity_.x *= scale;
			playerVelocity_.z *= scale;
		}

		// 未接続中も重力を加え、現在速度で位置を進める
		playerVelocity_.y -= kGravity * kDeltaTime;
		playerTransform_.translation_.x += playerVelocity_.x * kDeltaTime;
		playerTransform_.translation_.y += playerVelocity_.y * kDeltaTime;
		playerTransform_.translation_.z += playerVelocity_.z * kDeltaTime;
	}

	// --------------------------------------------------------
	// デモ用の簡易床判定
	// 床より下へ入った場合はY座標を戻し、落下速度だけを止める
	// --------------------------------------------------------
	if (playerTransform_.translation_.y < kFloorHeight) {
		playerTransform_.translation_.y = kFloorHeight;
		if (playerVelocity_.y < 0.0f) {
			playerVelocity_.y = 0.0f;
		}
	}
}

void SwingDemoScene::UpdateConnection() {
	// Eキーを押すたびに接続 / 解除を切り替える
	// 接続時の距離判定はAnchorSwingGimmick側で行う
	if (input_->TriggerKey(DIK_E)) {
		anchorSwing_.ToggleConnection(
			kAnchorPosition_,
			playerTransform_.translation_);
	}
}

void SwingDemoScene::UpdateRope() {
	if (!anchorSwing_.IsConnected()) {
		return;
	}

	const Vector3& player = playerTransform_.translation_;
	const Vector3& anchor = anchorSwing_.GetAnchorPosition();
	const Vector3 difference = {
		player.x - anchor.x,
		player.y - anchor.y,
		player.z - anchor.z,
	};

	const float horizontalLength = std::sqrt(
		difference.x * difference.x + difference.z * difference.z);
	const float ropeLength = std::sqrt(
		horizontalLength * horizontalLength + difference.y * difference.y);

	// ロープの中心をプレイヤーとアンカーの中間へ配置する
	ropeTransform_.translation_ = {
		(player.x + anchor.x) * 0.5f,
		(player.y + anchor.y) * 0.5f,
		(player.z + anchor.z) * 0.5f,
	};

	// cubeモデルをZ方向へ伸ばしてロープの長さに合わせる
	ropeTransform_.scale_ = {0.04f, 0.04f, ropeLength * 0.5f};

	// Z軸がアンカーからプレイヤーの方向を向くようXY角度を設定する
	ropeTransform_.rotation_.x =
		-std::atan2(difference.y, horizontalLength);
	ropeTransform_.rotation_.y =
		std::atan2(difference.x, difference.z);
	ropeTransform_.UpdateMatarix();
}

void SwingDemoScene::ResetDemo() {
	// 接続を解除し、位置と速度を完全に初期状態へ戻す
	anchorSwing_.Disconnect();
	playerTransform_.translation_ = kPlayerStartPosition_;
	playerVelocity_ = {};
	playerTransform_.UpdateMatarix();
}
