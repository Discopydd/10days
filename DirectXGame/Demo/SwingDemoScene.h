#pragma once

#include <KamataEngine.h>

#include "../Gimmick/AnchorSwingGimmick.h"

// ============================================================
// アンカースウィングギミック確認用のデモシーン
//
// WASD : 通常移動 / スウィング方向への補助加速
// E    : アンカーへの接続 / 接続解除
// R    : デモを初期状態へ戻す
// ESC  : アプリを終了する
// ============================================================
class SwingDemoScene {
public:
	// モデル・カメラ・各オブジェクトを初期化する
	void Initialize();

	// 入力、プレイヤー物理、ロープ表示を毎フレーム更新する
	// falseを返したらアプリ終了
	bool Update();

	// 床・アンカー・プレイヤー・ロープを描画する
	void Draw();

	// 動的に生成したモデルを解放する
	void Finalize();

private:
	// WorldTransformを初期化する補助処理
	void InitializeTransform(
	    KamataEngine::WorldTransform& transform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

	// WASD入力からXZ平面上の移動速度を作る
	KamataEngine::Vector3 GetMoveVelocity() const;

	// 接続状態に応じてプレイヤーの速度と位置を更新する
	void UpdatePlayer();

	// Eキーによるアンカーへの接続 / 解除を更新する
	void UpdateConnection();

	// アンカーとプレイヤーを結ぶロープの位置・長さ・向きを更新する
	void UpdateRope();

	// Rキーでプレイヤーの位置と速度を初期状態へ戻す
	void ResetDemo();

private:
	KamataEngine::Input* input_ = nullptr;

	// 描画物ごとに使用するモデル
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* anchorModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;

	KamataEngine::Camera camera_;

	// 各描画物の位置・回転・大きさ
	KamataEngine::WorldTransform playerTransform_;
	KamataEngine::WorldTransform anchorTransform_;
	KamataEngine::WorldTransform floorTransform_;
	KamataEngine::WorldTransform ropeTransform_;

	// 各描画物の色
	KamataEngine::ObjectColor playerColor_;
	KamataEngine::ObjectColor anchorColor_;
	KamataEngine::ObjectColor floorColor_;
	KamataEngine::ObjectColor ropeColor_;

	// アンカーへの接続とスウィング物理を管理するギミック
	AnchorSwingGimmick anchorSwing_;

	// 接続解除後も保持し、慣性飛行へ引き継ぐプレイヤー速度
	KamataEngine::Vector3 playerVelocity_{};

	// 60FPS固定として扱う1フレームの時間
	static constexpr float kDeltaTime = 1.0f / 60.0f;
	// WASD入力から作る目標移動速度
	static constexpr float kMoveSpeed = 6.0f;
	// 未接続時にWASD入力で加える加速度
	static constexpr float kFreeMoveAcceleration = 12.0f;
	// 未接続時の水平速度上限
	static constexpr float kMaxFreeHorizontalSpeed = 12.0f;
	// 接続中・未接続中の両方で使用する重力加速度
	static constexpr float kGravity = 19.6f;
	// プレイヤー中心が床より下へ入らないための最低Y座標
	static constexpr float kFloorHeight = 0.6f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {-5.0f, 2.0f, 0.0f};
	const KamataEngine::Vector3 kAnchorPosition_ = {0.0f, 6.0f, 0.0f};
};
