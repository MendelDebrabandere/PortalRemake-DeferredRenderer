#include "stdafx.h"
#include "MainMenuScene.h"

void MainMenuScene::Initialize()
{
	// Game settings
	//m_SceneContext.settings.showInfoOverlay = true;
	m_SceneContext.settings.drawPhysXDebug = true;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;

	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	InputManager::ForceMouseToCenter(true);
	InputManager::CursorVisible(false);

}

void MainMenuScene::Update()
{
}
