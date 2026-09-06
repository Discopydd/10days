#pragma once

#include <KamataEngine.h>

#include "../Common/Collision.h"
#include "../Gimmick/MovableBlockGimmick.h"

// ============================================================
// ゲーム本編を管理するGameScene
//
// main.cppへゲーム処理を詰め込まず、
// プレイヤー・ステージ・ギミックなどをこのクラスで管理する。
// 現段階では移動ブロックギミックを確認できる構成にしている。
// ============================================================
class GameScene {
public:
	// リソース・オブジェクトの初期化
	void Initialize();

	// 毎フレーム更新
	// falseを返したらアプリ終了
	bool Update();

	// 3Dオブジェクト描画
	void Draw();

	// 終了処理
	void Finalize();

private:
	// WorldTransformを初期化する補助処理
	void InitializeTransform(
	    KamataEngine::WorldTransform& worldTransform,
	    const KamataEngine::Vector3& position,
	    const KamataEngine::Vector3& scale);

	// プレイヤーを更新する
	KamataEngine::Vector3 UpdatePlayer();

	// Eキーによる接続 / 解除を更新する
	void UpdateConnection();

	// スイッチへ置かれたか判定する
	void UpdateSwitch();

	// 接続中の糸表示を更新する
	void UpdateRope();

	// RキーでGameScene全体を初期状態へ戻す
	void ResetGame();

private:
	KamataEngine::Input* input_ = nullptr;
	// 描画確認を確実にするため、オブジェクトごとにモデルを分ける
	// 必要に応じて後から共有モデルへ変更できる
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* switchModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::Model* wallModel_ = nullptr;
	KamataEngine::Model* ropeModel_ = nullptr;

	KamataEngine::Camera camera_;

	// --------------------------------------------------------
	// プレイヤー
	// --------------------------------------------------------
	KamataEngine::WorldTransform playerWorldTransform_;
	KamataEngine::ObjectColor playerColor_;

	// --------------------------------------------------------
	// 移動ブロックギミック
	// --------------------------------------------------------
	MovableBlockGimmick movableBlock_;

	// --------------------------------------------------------
	// スイッチ
	// --------------------------------------------------------
	KamataEngine::WorldTransform switchWorldTransform_;
	KamataEngine::ObjectColor switchColor_;
	Collision::AABB switchAABB_{};

	// --------------------------------------------------------
	// 床
	// --------------------------------------------------------
	KamataEngine::WorldTransform floorWorldTransform_;
	KamataEngine::ObjectColor floorColor_;

	// --------------------------------------------------------
	// 壁4枚
	// WorldTransformはコピー不可なので配列としてデフォルト構築する
	// --------------------------------------------------------
	static constexpr int kWallCount = 4;
	KamataEngine::WorldTransform wallWorldTransforms_[kWallCount];
	KamataEngine::ObjectColor wallColor_;

	// --------------------------------------------------------
	// 接続中の糸
	// 後で共通の接続システムへ置き換え可能
	// --------------------------------------------------------
	KamataEngine::WorldTransform ropeWorldTransform_;
	KamataEngine::ObjectColor ropeColor_;

	// --------------------------------------------------------
	// GameScene用定数
	// --------------------------------------------------------
	static constexpr float kPlayerSpeed = 0.10f;
	static constexpr float kConnectDistance = 5.0f;
	static constexpr float kDisconnectDistance = 7.0f;

	const KamataEngine::Vector3 kPlayerStartPosition_ = {-4.0f, 0.6f, 0.0f};
	const KamataEngine::Vector3 kBlockStartPosition_ = {0.0f, 1.0f, 0.0f};
	const KamataEngine::Vector3 kSwitchPosition_ = {4.0f, 0.10f, 0.0f};
	const KamataEngine::Vector3 kSwitchScale_ = {1.6f, 0.10f, 1.6f};
};
