#include "stdafx.h"
#include "MainMenuScene.h"

#include "PortalScene.h"
#include "Prefabs/CubePrefab.h"

void MainMenuScene::Initialize()
{
	// Game settings
	//m_SceneContext.settings.showInfoOverlay = true;
	m_SceneContext.settings.drawPhysXDebug = true;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;

	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	//Camera
	auto pCameraObject = new FixedCamera();
	AddChild(pCameraObject);
	pCameraObject->GetTransform()->Translate(0, 0, -40);
	auto pCameraComp = new CameraComponent();
	pCameraObject->AddComponent(pCameraComp);
	SetActiveCamera(pCameraComp);


	// Menu GUI
	m_pMenuSprite = new GameObject();
	m_pMenuSprite->AddComponent(new SpriteComponent(L"Textures/MainMenu.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	AddChild(m_pMenuSprite);

	m_pMenuSprite->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight / 2.f, .9f);

	//COMMENT THE GUI TO SEE THE ACTUAL BUTTON OBJECTS

	// Menu buttons
	const auto pMaterial = PxGetPhysics().createMaterial(.5f, .5f, .5f);

	auto pStartButton = AddChild(new CubePrefab(XMFLOAT3{ 11, 5, 1 }, XMFLOAT4{ Colors::Orange }));
	pStartButton->GetTransform()->Translate(0, -1, 0);
	auto startRB = pStartButton->AddComponent(new RigidBodyComponent(true));
	startRB->AddCollider(PxBoxGeometry{ 5, 2, 0.5f }, *pMaterial, false);
	pStartButton->SetTag(L"StartButton");

	auto pQuitButton = AddChild(new CubePrefab(XMFLOAT3{ 11, 5, 1 }, XMFLOAT4{ Colors::Blue }));
	pQuitButton->GetTransform()->Translate(0, -9, 0);
	auto pColQuit = pQuitButton->AddComponent(new RigidBodyComponent(true));
	pColQuit->AddCollider(PxBoxGeometry{ 5, 2, 0.5f }, *pMaterial, true);
	pQuitButton->SetTag(L"QuitButton");

}

void MainMenuScene::Update()
{
	if (InputManager::IsMouseButton(InputState::pressed, VK_LBUTTON))
	{
		if (const auto pPickedObject = m_SceneContext.pCamera->Pick())
		{
			if (pPickedObject->GetTag() == L"StartButton")
			{
				SceneManager::Get()->SetActiveGameScene(L"PortalScene");
			}

			else if (pPickedObject->GetTag() == L"QuitButton")
			{
				PostQuitMessage(0);
			}
		}
	}
}

void MainMenuScene::OnSceneActivated()
{
	//Input
	InputManager::ForceMouseToCenter(false);
	InputManager::CursorVisible(true);

	//Reset portal scene
	auto portalScene = SceneManager::Get()->GetSceneByName(L"PortalScene");
	if (portalScene)
		SceneManager::Get()->RemoveGameScene(portalScene, true);

	SceneManager::Get()->AddGameScene(new PortalScene());
}
