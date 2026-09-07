#include <KamataEngine.h>
#include <Windows.h>

#include "Demo/SwingDemoScene.h"

using namespace KamataEngine;

// ============================================================
// Windowsアプリのエントリーポイント
// ============================================================
int WINAPI WinMain(
    _In_ HINSTANCE,
    _In_opt_ HINSTANCE,
    _In_ LPSTR,
    _In_ int) {

	// エンジン初期化
	KamataEngine::Initialize(
		L"糸結び");

	DirectXCommon* dxCommon =
		DirectXCommon::GetInstance();

	// ギミック確認用シーン初期化
	SwingDemoScene scene;
	scene.Initialize();

	// ========================================================
	// メインループ
	// ========================================================
	while (true) {
		// ウィンドウ・入力更新
		if (KamataEngine::Update()) {
			break;
		}

		// ギミック確認用シーン更新
		if (!scene.Update()) {
			break;
		}

		// ----------------------------------------------------
		// 描画
		// ----------------------------------------------------
		dxCommon->PreDraw();

		Model::PreDraw(Model::CullingMode::kNone);
		scene.Draw();
		Model::PostDraw();

		dxCommon->PostDraw();
	}

	// 終了処理
	scene.Finalize();
	KamataEngine::Finalize();

	return 0;
}
