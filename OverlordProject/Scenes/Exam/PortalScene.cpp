#include "stdafx.h"
#include "PortalScene.h"

#include "Materials/DiffuseMaterial.h"
#include "Prefabs/Character.h"
#include "Prefabs/PortalGun.h"
#include "Prefabs/CubePrefab.h"
#include "Prefabs/Portal.h"
#include "Materials/Portal/PortalMaterial.h"

void PortalScene::Initialize()
{
	// Game settings
	//m_SceneContext.settings.showInfoOverlay = true;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = false;

	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	//in this order because the level uses the portalgun object in init of no portal walls for start game
	InitCharacter(true, 10);
	InitLevel();


	//Sprite on screen
	m_pSprite = new GameObject();
	m_pSprite->AddComponent(new SpriteComponent(L"Textures/PortalCrosshair.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	AddChild(m_pSprite);

	m_pSprite->GetTransform()->Translate(1280 / 2.f, 720 / 2.f, 1.);
	m_pSprite->GetTransform()->Scale(1.f, 1.f, 1.f);

}

void PortalScene::Update()
{
	//Shooting gun
	if (InputManager::IsMouseButton(InputState::pressed, VK_LBUTTON))
	{
		m_pPortalGun->ShootGun(PortalType::Blue);
	}
	if (InputManager::IsMouseButton(InputState::pressed, VK_RBUTTON))
	{
		m_pPortalGun->ShootGun(PortalType::Orange);
	}

	//Update portal variables
	auto portalPair = m_pPortalGun->GetPortals();
	m_pBluePortal = portalPair.first;
	m_pOrangePortal = portalPair.second;


	if (InputManager::IsKeyboardKey(InputState::pressed, 27)) // ESC BUTTON = 27
	{
		SceneManager::Get()->SetActiveGameScene(L"MainMenuScene");
	}
}

void PortalScene::InitLevel()
{
	// Default physics material
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);

	//Cube
	{
		DiffuseMaterial* pCubeMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
		//Set texture of the material
		pCubeMaterial->SetDiffuseTexture(L"textures/PortalCube.jpg");

		auto pCube = AddChild(new GameObject());
		pCube->SetTag(L"Cube");

		//Mesh
		const auto CubeMeshComponent = new ModelComponent(L"Meshes/PortalCube.ovm");
		CubeMeshComponent->SetMaterial(pCubeMaterial);
		pCube->AddComponent<ModelComponent>(CubeMeshComponent);

		//RigidBody
		const auto pConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/PortalCube.ovpc");
		const auto convexGeometry{ PxConvexMeshGeometry{ pConvexMesh } };
		auto cubeRB = pCube->AddComponent(new RigidBodyComponent(false));
		cubeRB->AddCollider(convexGeometry, *pDefaultMaterial);
		cubeRB->GetPxRigidActor()->setName("CubeRB");
		pCube->GetTransform()->Translate(60, 40, 90.f);
	}

	//Door
	{
		DiffuseMaterial* pDoorMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
		//Set texture of the material
		pDoorMaterial->SetDiffuseTexture(L"textures/portal_door.jpeg");

		auto pDoor = AddChild(new GameObject());

		//Mesh
		const auto doorMeshComponent = new ModelComponent(L"Meshes/door.ovm");
		doorMeshComponent->SetMaterial(pDoorMaterial);
		pDoor->AddComponent<ModelComponent>(doorMeshComponent);

		//RigidBody
		const auto pDoorConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/door.ovpc");
		const auto doorConvexGeometry{ PxConvexMeshGeometry{ pDoorConvexMesh } };
		auto doorRB = pDoor->AddComponent(new RigidBodyComponent(true));
		doorRB->AddCollider(doorConvexGeometry, *pDefaultMaterial);
		doorRB->GetPxRigidActor()->setName("DoorRB");
		pDoor->GetTransform()->Translate(72.f, 0, 51.f);
		pDoor->GetTransform()->Rotate(0, 90, 0);

		doorRB->AddCollider(PxBoxGeometry{ 5.f, 8.f, 2.6f }, *pDefaultMaterial, true);

		auto doorFunction = [=](GameObject*, GameObject* pOtherObject, PxTriggerAction action)
		{
			//IF THE BUTTON IS PRESSED
			if (action == PxTriggerAction::ENTER && m_ButtonIsTriggered)
			{
				if (pOtherObject->GetTag() == L"Player")
				{
					//GO TO MAIN MENU
					SceneManager::Get()->SetActiveGameScene(L"MainMenuScene");
				}
			}
		};

		pDoor->SetOnTriggerCallBack(doorFunction);
	}

	//Button
	{
		DiffuseMaterial* pButtonMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
		//Set texture of the material
		pButtonMaterial->SetDiffuseTexture(L"textures/portal_button.jpeg");

		auto pButton = AddChild(new GameObject());

		//Mesh
		const auto buttonMeshComponent = new ModelComponent(L"Meshes/button.ovm");
		buttonMeshComponent->SetMaterial(pButtonMaterial);
		pButton->AddComponent<ModelComponent>(buttonMeshComponent);

		//RigidBody
		const auto pButtonConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/button.ovpc");
		const auto buttonConvexGeometry{ PxConvexMeshGeometry{ pButtonConvexMesh } };
		auto buttonRB = pButton->AddComponent(new RigidBodyComponent(true));
		buttonRB->AddCollider(buttonConvexGeometry, *pDefaultMaterial);
		buttonRB->GetPxRigidActor()->setName("ButtonRB");
		pButton->GetTransform()->Translate(62, 0, 15.f);
		pButton->GetTransform()->Rotate(0, 0, 0);
		buttonRB->AddCollider(PxBoxGeometry{ 2.3f, 2.2f, 2.3f }, *pDefaultMaterial, true);


		auto buttonFunction = [=](GameObject*, GameObject* pOtherObject, PxTriggerAction action)
		{
			auto pFmod = SoundManager::Get()->GetSystem();
			FMOD::Sound* pSound{};

			//IF IT IS THE CUBE
			auto rb = pOtherObject->GetComponent<RigidBodyComponent>();
			if (rb)
			{
				auto name = rb->GetPxRigidActor()->getName();
				if (name)
				{
					if (std::string(name) == "CubeRB")
					{
						if (action == PxTriggerAction::ENTER)
						{
							m_ButtonIsTriggered = true;
							pFmod->createStream("Resources/sound/ButtonPressed.mp3", FMOD_DEFAULT, nullptr, &pSound);
						}
						if (action == PxTriggerAction::LEAVE)
						{
							m_ButtonIsTriggered = false;
							pFmod->createStream("Resources/sound/ButtonReleased.mp3", FMOD_DEFAULT, nullptr, &pSound);
						}
					}
				}
			}

			if (pSound)
			{
				pFmod->playSound(pSound, nullptr, false, &m_pChannelAudio);
				m_pChannelAudio->setVolume(0.2f);
			}
		};

		pButton->SetOnTriggerCallBack(buttonFunction);
	}

	//level
	{
		DiffuseMaterial* pLevelMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
		//Set texture of the material
		pLevelMaterial->SetDiffuseTexture(L"textures/WallTextureWhite.jpg");

		const auto levelModel = new ModelComponent(L"Meshes/PortalLevel.ovm");
		levelModel->SetMaterial(pLevelMaterial);
		auto pLevel = new GameObject();
		pLevel->AddComponent<ModelComponent>(levelModel);
		pLevel->GetComponent<TransformComponent>()->Rotate(0, -90, 0);
		AddChild(pLevel);
		//RigidBody
		//
		//I want to just add a RB but for some reason the .ovpc doesnt work, so i will have to place them all manualy :(
		//A lot of ugly hard coded values :(
		//
		//const auto pLevelConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/PortalLevel.ovpc");
		//const auto pLevelConvexGeometry{ PxConvexMeshGeometry{ pLevelConvexMesh } };
		auto pLevelRB = pLevel->AddComponent(new RigidBodyComponent(true)); /*->AddCollider(pLevelConvexGeometry, *pDefaultMaterial);*/
		pLevelRB->AddCollider(PxBoxGeometry{ 14.8f, 10 ,40 }, *pDefaultMaterial, false, PxTransform(10, -10.f, -35)); // Start floor
		//pLevelRB->AddCollider(PxBoxGeometry{ 10.f, 2, 40 }, *pDefaultMaterial, false, PxTransform(30, -11.7f, -35)); // Drop down floor
		pLevelRB->AddCollider(PxBoxGeometry{ 35.f, 10, 40 }, *pDefaultMaterial, false, PxTransform(74.5f, -10.f, -35)); // Other side floor

		pLevelRB->AddCollider(PxBoxGeometry{ 2, 32, 40 }, *pDefaultMaterial, false, PxTransform(3.f, 20.f, -35)); // Back wall
		pLevelRB->AddCollider(PxBoxGeometry{ 2, 32, 40 }, *pDefaultMaterial, false, PxTransform(105.9f, 20.f, -35)); // Front wall
		UINT colliderID = pLevelRB->AddCollider(PxBoxGeometry{ 60, 32, 2 }, *pDefaultMaterial, false, PxTransform(55.f, 20.f, -3)); // Left wall
		pLevelRB->AddCollider(PxBoxGeometry{ 60, 32, 2 }, *pDefaultMaterial, false, PxTransform(55.f, 20.f, -73.8f)); // Right wall

		pLevelRB->AddCollider(PxBoxGeometry{ 8.69f, 14.7f, 30 }, *pDefaultMaterial, false, PxTransform(68.08f, 10.1f, -54.7f)); // Middle Wall Bottom part
		pLevelRB->AddCollider(PxBoxGeometry{ 8.69f, 6.3f, 30 }, *pDefaultMaterial, false, PxTransform(68.08f, 45.9f, -54.7f)); // Middle Wall Top part
		pLevelRB->AddCollider(PxBoxGeometry{ 8.69f, 7.5f, 12.4f }, *pDefaultMaterial, false, PxTransform(68.08f, 32.f, -37.1f)); // Middle Wall Left part
		pLevelRB->AddCollider(PxBoxGeometry{ 8.69f, 7.5f, 4.f }, *pDefaultMaterial, false, PxTransform(68.08f, 32.f, -68.4f)); // Middle Wall Right part

		pLevelRB->AddCollider(PxBoxGeometry{ 14.f, 14.7f, 20 }, *pDefaultMaterial, false, PxTransform(90.7f, 10.1f, -64.5f)); // Back elevated part

		//Set to left wall
		m_pPortalGun->SetOrangePortalWall(pLevelRB->GetCollider(colliderID).GetShape());

	}

	//No portal walls
	{
		DiffuseMaterial* pWallMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
		//Set texture of the material
		pWallMaterial->SetDiffuseTexture(L"textures/NoPortal.png");

		auto pWalls = AddChild(new GameObject());
		pWalls->SetTag(L"Walls");

		//Mesh
		const auto wallMeshComponent = new ModelComponent(L"Meshes/NoPortalWalls.ovm");
		wallMeshComponent->SetMaterial(pWallMaterial);
		pWalls->AddComponent<ModelComponent>(wallMeshComponent);

		//RigidBody
		const auto pConvexMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/NoPortalWalls.ovpt");
		const auto convexGeometry{ PxTriangleMeshGeometry{ pConvexMesh } };
		auto wallRB = pWalls->AddComponent(new RigidBodyComponent(true));
		UINT colliderID = wallRB->AddCollider(convexGeometry, *pDefaultMaterial);
		wallRB->GetPxRigidActor()->setName("WallsRB");
		pWalls->GetTransform()->Rotate(0, -90, 0);

		m_pPortalGun->SetBluePortalWall(wallRB->GetCollider(colliderID).GetShape());
	}

	//Glass Cage
	{
		DiffuseMaterial* pCubeMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
		//Set texture of the material
		pCubeMaterial->SetDiffuseTexture(L"textures/Glass.png");
		pCubeMaterial->SetTechnique(L"TransparencyTech");

		auto pCube = AddChild(new GameObject());
		pCube->SetTag(L"Glass");

		//Mesh
		const auto CubeMeshComponent = new ModelComponent(L"Meshes/GlassCage.ovm");
		CubeMeshComponent->SetMaterial(pCubeMaterial);
		pCube->AddComponent<ModelComponent>(CubeMeshComponent);

		//RigidBody
		const auto pConvexMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/GlassCage.ovpt");
		const auto convexGeometry{ PxTriangleMeshGeometry{ pConvexMesh } };
		auto cubeRB = pCube->AddComponent(new RigidBodyComponent(true));
		cubeRB->AddCollider(convexGeometry, *pDefaultMaterial);
		cubeRB->GetPxRigidActor()->setName("GlassRB");
		pCube->GetTransform()->Rotate(0, -90, 0);
		pCube->GetTransform()->Translate(0, 1.2f, 0);
	}
}

void PortalScene::InitCharacter(bool controlCamera, float mouseSens)
{
	// Default physics material
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);

	enum InputIds
	{
		CharacterMoveLeft,
		CharacterMoveRight,
		CharacterMoveForward,
		CharacterMoveBackward,
		CharacterJump
	};

	// Character creation
	CharacterDesc characterDesc{ pDefaultMaterial };
	characterDesc.actionId_MoveForward = CharacterMoveForward;
	characterDesc.actionId_MoveBackward = CharacterMoveBackward;
	characterDesc.actionId_MoveLeft = CharacterMoveLeft;
	characterDesc.actionId_MoveRight = CharacterMoveRight;
	characterDesc.actionId_Jump = CharacterJump;
	characterDesc.rotationSpeed = mouseSens;
	characterDesc.JumpSpeed = 18;
	characterDesc.controller.height = 2.7f;

	m_pCharacter = AddChild(new Character(characterDesc));
	m_pCharacter->GetTransform()->Translate(25.f, -7.3f, 25.f);

	// Character input
	auto inputAction = InputAction(CharacterMoveLeft, InputState::down, 'A');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveRight, InputState::down, 'D');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveForward, InputState::down, 'W');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveBackward, InputState::down, 'S');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterJump, InputState::pressed, VK_SPACE, -1, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);

	// Control camera
	m_pCharacter->SetCameraActive(controlCamera);

	m_pCharacter->SetCollisionGroup(CollisionGroup::Group9);

	// Add protalgun
	m_pPortalGun = new PortalGun(this, m_pCharacter);
	m_pCharacter->AddChild(m_pPortalGun);
}

void PortalScene::PostDraw()
{

	auto pMainCam = GetActiveCamera();

	//Draw blue portal
	SetActiveCamera(m_pBluePortal->GetCamera());
	m_pBluePortal->UpateNearClipPlane();
	DrawPortal(m_pBluePortal->GetRenderTarget());
	m_pOrangePortal->GetScreenMat()->SetVariable_Texture(L"gTexture", m_pBluePortal->GetRenderTarget()->GetColorShaderResourceView());


	//Draw orange portal
	SetActiveCamera(m_pOrangePortal->GetCamera());
	m_pOrangePortal->UpateNearClipPlane();
	DrawPortal(m_pOrangePortal->GetRenderTarget());
	m_pBluePortal->GetScreenMat()->SetVariable_Texture(L"gTexture", m_pOrangePortal->GetRenderTarget()->GetColorShaderResourceView());


	pMainCam->SetActive(true);
}

void PortalScene::OnGUI()
{
	m_pCharacter->DrawImGui();
}

void PortalScene::OnSceneActivated()
{
	//INPUT
	InputManager::ForceMouseToCenter(true);
	InputManager::CursorVisible(false);
}
