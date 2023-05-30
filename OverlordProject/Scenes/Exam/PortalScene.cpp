#include "stdafx.h"
#include "PortalScene.h"

#include "Materials/BasicMaterial_Deferred.h"
#include "Materials/ColorMaterial.h"
#include "Materials/DiffuseMaterial.h"
#include "Prefabs/Character.h"
#include "Prefabs/PortalGun.h"
#include "Prefabs/CubePrefab.h"
#include "Prefabs/Portal.h"
#include "Materials/Portal/PortalMaterial.h"
#include "Prefabs/SpherePrefab.h"

void PortalScene::Initialize()
{
	// Game settings
	//m_SceneContext.settings.showInfoOverlay = true;
	m_SceneContext.settings.drawPhysXDebug = true;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;

	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	InputManager::ForceMouseToCenter(true);
	InputManager::CursorVisible(false);

	InitLevel();
	InitCharacter(true, 10);

}

void PortalScene::Update()
{
	//Shooting gun
	if (InputManager::IsMouseButton(InputState::pressed, VK_RBUTTON))
	{
		m_pPortalGun->ShootGun(PortalType::Blue);
	}
	if (InputManager::IsMouseButton(InputState::pressed, VK_LBUTTON))
	{
		m_pPortalGun->ShootGun(PortalType::Orange);
	}

	//Update portal variables
	auto portalPair = m_pPortalGun->GetPortals();
	m_pBluePortal = portalPair.first;
	m_pOrangePortal = portalPair.second;
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

	// Wall3 creation
	auto wall3 = new CubePrefab(wallSize.x, wallSize.y, 1, wallColor);
	wall3->GetTransform()->Translate(-40,0, 40);
	wall3->GetTransform()->Rotate(80, wall2Rot, 0);
	auto wall3RB = wall3->AddComponent(new RigidBodyComponent(true));
	wall3RB->AddCollider(PxBoxGeometry{ wallSize.x / 2, wallSize.y / 2, 1 / 2.f }, *pDefaultMaterial);
	AddChild(wall3);

	//CHAIR
	DiffuseMaterial* pMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	//Set texture of the material
	pMaterial->SetDiffuseTexture(L"textures/Chair_Dark.dds");

	auto pChair = new GameObject();
	const auto component = new ModelComponent(L"Meshes/Chair.ovm");
	component->SetMaterial(pMaterial);
	pChair->AddComponent<ModelComponent>(component);
	pChair->GetTransform()->Translate(18, -3, 35);
	AddChild(pChair);

	//Cube
	auto cube = new CubePrefab(1, 1, 1, XMFLOAT4{ 0.1f,0.1f,0.1f,1 });
	cube->GetTransform()->Translate(18, 1, 25);
	AddChild(cube);

	////level
	//const auto model = new ModelComponent(L"Meshes/PortalLevel.ovm");
	//model->SetMaterial(pMaterial);
	//auto pLevel = new GameObject();
	//pLevel->AddComponent<ModelComponent>(model);
	//pLevel->GetComponent<TransformComponent>()->Translate(0.0f, 15.0f, 0.0f);
	//pLevel->GetComponent<TransformComponent>()->Rotate(0, 90, 0);
	//pLevel->GetComponent<TransformComponent>()->Scale(0.03f, 0.03f, 0.03f);
	////RigidBody
	//const auto pConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/PortalLevel.ovpc");
	//const auto convexGeometry{ PxConvexMeshGeometry{ pConvexMesh } };
	//pLevel->AddComponent(new RigidBodyComponent())->AddCollider(convexGeometry, *pDefaultMaterial);
	//AddChild(pLevel);
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

	// Add protalgun
	m_pPortalGun = new PortalGun(this, m_pCharacter);
	m_pCharacter->AddChild(m_pPortalGun);
}

void PortalScene::PostDraw()
{

	auto pMainCam = GetActiveCamera();

	//Draw blue portal
	SetActiveCamera(m_pBluePortal->GetCamera());
	m_pBluePortal->SetNearClipPlane();
	DrawPortal(m_pBluePortal->GetRenderTarget());
	m_pOrangePortal->GetScreenMat()->SetVariable_Texture(L"gTexture", m_pBluePortal->GetRenderTarget()->GetColorShaderResourceView());


	//Draw orange portal
	SetActiveCamera(m_pOrangePortal->GetCamera());
	m_pOrangePortal->SetNearClipPlane();
	DrawPortal(m_pOrangePortal->GetRenderTarget());
	m_pBluePortal->GetScreenMat()->SetVariable_Texture(L"gTexture", m_pOrangePortal->GetRenderTarget()->GetColorShaderResourceView());


	pMainCam->SetActive(true);
}

void PortalScene::OnGUI()
{
	m_pCharacter->DrawImGui();
}