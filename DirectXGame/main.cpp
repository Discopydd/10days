#include <KamataEngine.h>
#include <Windows.h>

#include "Scene/GameScene.h"

using namespace KamataEngine;

// ============================================================
// Windowsアプリのエントリーポイント
// ============================================================
int WINAPI WinMain(
	_In_ HINSTANCE,
	_In_opt_ HINSTANCE,
	_In_ LPSTR,
	_In_ int) {

	// ========================================================
	// エンジン初期化
	// ========================================================
	KamataEngine::Initialize(L"糸結び");

	DirectXCommon* dxCommon =
		DirectXCommon::GetInstance();

	Input* input =
		Input::GetInstance();

	WinApp* winApp =
		WinApp::GetInstance();

	// ========================================================
	// GameScene初期化
	// ========================================================
	GameScene* scene = new GameScene();
	scene->Initialize();

	// ========================================================
	// メインループ
	// ========================================================
	while (true) {

		// ----------------------------------------------------
		// ウィンドウ・入力更新
		// ----------------------------------------------------
		if (KamataEngine::Update()) {
			break;
		}

		// ----------------------------------------------------
		// ESCキーでフルスクリーン切り替え
		// ----------------------------------------------------
		if (input->TriggerKey(DIK_ESCAPE)) {

			// 現在のフルスクリーン状態を取得
			bool isFullscreen =
				winApp->IsFullscreen();

			// 現在と反対の状態へ切り替える
			winApp->SetFullscreen(
				!isFullscreen);
		}

		// ----------------------------------------------------
		// GameScene更新
		// ----------------------------------------------------
		if (!scene->Update()) {
			break;
		}

		// ====================================================
		// 描画
		// ====================================================
		dxCommon->PreDraw();

		// 現在は確認しやすいように両面描画
		Model::PreDraw(
			Model::CullingMode::kNone);

		scene->Draw();

		Model::PostDraw();

		dxCommon->PostDraw();
	}

	// ========================================================
	// 終了処理
	// ========================================================
	scene->Finalize();
	delete scene;
	scene = nullptr;

	KamataEngine::Finalize();

	return 0;
}