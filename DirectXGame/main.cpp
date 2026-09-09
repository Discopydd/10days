#include <KamataEngine.h>
#include <Windows.h>

#include "Scene/SceneManager.h"

using namespace KamataEngine;

int WINAPI WinMain(
	_In_ HINSTANCE,
	_In_opt_ HINSTANCE,
	_In_ LPSTR,
	_In_ int) {

	KamataEngine::Initialize(L"4044_糸結び");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Input* input = Input::GetInstance();
	WinApp* winApp = WinApp::GetInstance();

	SceneManager* sceneManager = new SceneManager();
	sceneManager->Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		if (input->TriggerKey(DIK_ESCAPE)) {
			winApp->SetFullscreen(!winApp->IsFullscreen());
		}

		if (!sceneManager->Update()) {
			break;
		}

		dxCommon->PreDraw();

		Sprite::PreDraw();
		sceneManager->DrawBackground();
		Sprite::PostDraw();

		Model::PreDraw(Model::CullingMode::kNone);
		sceneManager->Draw();
		Model::PostDraw();

		Sprite::PreDraw();
		sceneManager->DrawSprite();
		Sprite::PostDraw();

		dxCommon->PostDraw();
	}

	sceneManager->Finalize();
	delete sceneManager;
	sceneManager = nullptr;

	KamataEngine::Finalize();

	return 0;
}
