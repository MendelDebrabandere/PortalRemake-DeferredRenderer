#include "stdafx.h"
#include "PortalScene.h"

#include "Prefabs/Character.h"
#include "Prefabs/PortalGun.h"
#include "Prefabs/CubePrefab.h"
#include "Prefabs/Portal.h"

void PortalScene::Initialize()
{
	// Game settings
	//m_SceneContext.settings.showInfoOverlay = true;
	m_SceneContext.settings.drawPhysXDebug = true;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;

	InputManager::ForceMouseToCenter(m_MouseGone);
	InputManager::CursorVisible(!m_MouseGone);


	InitLevel();
	InitCharacter(true, 10);
}

void PortalScene::Update()
{
	if (InputManager::IsMouseButton(InputState::pressed, VK_RBUTTON))
	{
		m_pPortalGun->ShootGun(this, PortalType::Blue);
	}
	if (InputManager::IsMouseButton(InputState::pressed, VK_LBUTTON))
	{
		m_pPortalGun->ShootGun(this, PortalType::Orange);
	}
}

void PortalScene::OnGUI()
{
	m_pCharacter->DrawImGui();
}

void PortalScene::InitLevel()
{
	// Default physics material
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);


	// Floor settings
	constexpr XMFLOAT4	floorColor{ 0.3f,0.3f,0.3f,1.f };
	constexpr XMFLOAT2	floorSize{ 100,100 };

	// Walls settings
	constexpr XMFLOAT4	wallColor{ 0.8f,0.8f,0.8f,1.f };
	constexpr XMFLOAT2	wallSize{ 30,10 };
	constexpr XMFLOAT2	wall1Pos{ 10,10 };
	constexpr float		wall1Rot{ 45 };
	constexpr XMFLOAT2	wall2Pos{ -10,-10 };
	constexpr float		wall2Rot{ 45 };



	// Floor creation
	m_pFloor = new CubePrefab(floorSize.x, 1, floorSize.y, floorColor);
	m_pFloor->GetTransform()->Translate(0, -0.5F, 0);
	auto floorRB = m_pFloor->AddComponent(new RigidBodyComponent(true));
	floorRB->AddCollider(PxBoxGeometry{ floorSize.x / 2, 1 / 2.f, floorSize.y / 2 }, *pDefaultMaterial);
	AddChild(m_pFloor);

	// Wall1 creation
	m_pWall1 = new CubePrefab(wallSize.x, wallSize.y, 1, wallColor);
	m_pWall1->GetTransform()->Translate(wall1Pos.x, wallSize.y / 2, wall1Pos.y);
	m_pWall1->GetTransform()->Rotate(0, wall1Rot, 0);
	auto wall1RB = m_pWall1->AddComponent(new RigidBodyComponent(true));
	wall1RB->AddCollider(PxBoxGeometry{ wallSize.x / 2, wallSize.y / 2, 1 / 2.f }, *pDefaultMaterial);
	AddChild(m_pWall1);

	// Wall2 creation
	m_pWall2 = new CubePrefab(wallSize.x, wallSize.y, 1, wallColor);
	m_pWall2->GetTransform()->Translate(wall2Pos.x, wallSize.y / 2, wall2Pos.y);
	m_pWall2->GetTransform()->Rotate(0, wall2Rot, 0);
	auto wall2RB = m_pWall2->AddComponent(new RigidBodyComponent(true));
	wall2RB->AddCollider(PxBoxGeometry{ wallSize.x / 2, wallSize.y / 2, 1 / 2.f }, *pDefaultMaterial);
	AddChild(m_pWall2);
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

	m_pCharacter = AddChild(new Character(characterDesc));
	m_pCharacter->GetTransform()->Translate(0.f, 5.f, 0.f);

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

	//Character collision group
	m_pCharacter->SetCollisionGroup(CollisionGroup::Group9);

	// Add protalgun
	m_pPortalGun = new PortalGun();
	m_pCharacter->AddChild(m_pPortalGun);
}


