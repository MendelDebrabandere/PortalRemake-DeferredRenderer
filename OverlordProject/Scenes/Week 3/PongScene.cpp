#include "stdafx.h"
#include "PongScene.h"

#include "Prefabs/CubePrefab.h"
#include "Prefabs/SpherePrefab.h"

void PongScene::Initialize()
{


	//m_SceneContext.settings.showInfoOverlay = true;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;

	auto& physx = PxGetPhysics();
	auto pBouncyMaterial = physx.createMaterial(0.5f, 0.5f, 1.f);

	auto& sceneSettings = GetSceneSettings();
	sceneSettings.clearColor = XMFLOAT4{ Colors::Black };

	//auto* camera = GetActiveCamera();
	//camera->



	//PongLeft
	m_pPongLeft = new CubePrefab(pongRectSize.x, pongRectSize.y, pongRectSize.z, pongColor);
	m_pPongLeft->GetTransform()->Translate(-30, pongRectSize.y, 0);
	AddChild(m_pPongLeft);

	auto pongLeftRB = m_pPongLeft->AddComponent(new RigidBodyComponent(true));
	pongLeftRB->AddCollider(PxBoxGeometry{ pongRectSize.x / 2, pongRectSize.y/2, pongRectSize.z / 2 }, *pBouncyMaterial);
	pongLeftRB->SetDensity(staticDensity);


	//PongRight
	m_pPongRight = new CubePrefab(pongRectSize.x, pongRectSize.y, pongRectSize.z, pongColor);
	m_pPongRight->GetTransform()->Translate(30, pongRectSize.y, 0);
	AddChild(m_pPongRight);

	auto pongRightRB = m_pPongRight->AddComponent(new RigidBodyComponent(true));
	pongRightRB->AddCollider(PxBoxGeometry{ pongRectSize.x / 2, pongRectSize.y/2, pongRectSize.z / 2 }, *pBouncyMaterial);
	pongRightRB->SetDensity(staticDensity);


	//Ball
	m_pBall = new SpherePrefab(ballRadius, 10, ballColor);
	m_pBall->GetTransform()->Translate(0, ballRadius, 0);
	AddChild(m_pBall);

	auto ballRB = m_pBall->AddComponent(new RigidBodyComponent());
	ballRB->AddCollider(PxSphereGeometry{ ballRadius }, *pBouncyMaterial);
	ballRB->SetConstraint(RigidBodyConstraint::TransY, false);
	ballRB->SetConstraint(RigidBodyConstraint::RotX | RigidBodyConstraint::RotY | RigidBodyConstraint::RotZ, false);


	//Top Trigger
	auto pTopTrigger = new CubePrefab(0, 0, 0, pongColor);
	pTopTrigger->GetTransform()->Translate(0, 0, fieldSize.y);
	AddChild(pTopTrigger);

	auto pTopTriggerRB = pTopTrigger->AddComponent(new RigidBodyComponent(true));
	pTopTriggerRB->AddCollider(PxBoxGeometry{ fieldSize.x, 2.5f, 1}, *pBouncyMaterial);
	pTopTriggerRB->SetDensity(staticDensity);


	//Bottom Trigger
	auto pBottomTrigger = new CubePrefab(0, 0, 0, pongColor);
	pBottomTrigger->GetTransform()->Translate(0, 0, -fieldSize.y);
	AddChild(pBottomTrigger);

	auto pBottomTriggerRB = pBottomTrigger->AddComponent(new RigidBodyComponent(true));
	pBottomTriggerRB->AddCollider(PxBoxGeometry{ fieldSize.x, 2.5f, 1 }, *pBouncyMaterial);
	pBottomTriggerRB->SetDensity(staticDensity);


	//Left Trigger
	m_pLeftTrigger = new CubePrefab(0, 0, 0, pongColor);
	m_pLeftTrigger->GetTransform()->Translate(-fieldSize.x, 0, 0);
	AddChild(m_pLeftTrigger);

	auto pLeftTriggerRB = m_pLeftTrigger->AddComponent(new RigidBodyComponent(true));
	pLeftTriggerRB->AddCollider(PxBoxGeometry{ 1, 2.5f, fieldSize.y }, *pBouncyMaterial, true);


	//Right Trigger
	m_pRightTrigger = new CubePrefab(0, 0, 0, pongColor);
	m_pRightTrigger->GetTransform()->Translate(fieldSize.x, 0, 0);
	AddChild(m_pRightTrigger);

	auto pRightTriggerRB = m_pRightTrigger->AddComponent(new RigidBodyComponent(true));
	pRightTriggerRB->AddCollider(PxBoxGeometry{ 1, 2.5f, fieldSize.y }, *pBouncyMaterial, true);

}

void PongScene::Update()
{
	// Move ball when space is pressed
	if (m_IsBallMoving == false)
	{
		if (InputManager::IsKeyboardKey(InputState::pressed, VK_SPACE))
		{
			const float force{ 4000.f };
			const auto RBcomp = m_pBall->GetComponent<RigidBodyComponent>();
			if (RBcomp)
				RBcomp->AddForce(XMFLOAT3{ force,0,force });
			else
				std::cout << "No RBcomp found \n";
			m_IsBallMoving = true;
		}
	}

	//INPUT
	const auto rightPPos = m_pPongRight->GetTransform()->GetWorldPosition();
	const auto leftPPos = m_pPongLeft->GetTransform()->GetWorldPosition();
	const auto dTime = m_SceneContext.pGameTime->GetElapsed();
	const float moveSpeed{ 20.f };

	if (InputManager::IsKeyboardKey(InputState::down, VK_UP))
	{
		m_pPongRight->GetTransform()->Translate(rightPPos.x, rightPPos.y, rightPPos.z + moveSpeed * dTime);
	}
	if (InputManager::IsKeyboardKey(InputState::down, VK_DOWN))
	{
		m_pPongRight->GetTransform()->Translate(rightPPos.x, rightPPos.y, rightPPos.z - moveSpeed * dTime);
	}
	if (InputManager::IsKeyboardKey(InputState::down, 'R'))
	{
		m_pPongLeft->GetTransform()->Translate(leftPPos.x, leftPPos.y, leftPPos.z + moveSpeed * dTime);
	}
	if (InputManager::IsKeyboardKey(InputState::down, 'F'))
	{
		m_pPongLeft->GetTransform()->Translate(leftPPos.x, leftPPos.y, leftPPos.z - moveSpeed * dTime);
	}

	//Reset ball if it is out of bounds
	const auto ballXPos = m_pBall->GetTransform()->GetWorldPosition().x;

	if (ballXPos <= m_pLeftTrigger->GetTransform()->GetWorldPosition().x ||
		ballXPos >= m_pRightTrigger->GetTransform()->GetWorldPosition().x)
	{
		m_pBall->GetTransform()->Translate(0, 1, 0);
		m_IsBallMoving = false;
	}

	//Clamp peddles to playspace
	const auto leftPeddlePos = m_pPongLeft->GetTransform()->GetWorldPosition();
	const auto rightPeddlePos = m_pPongRight->GetTransform()->GetWorldPosition();

	if (leftPeddlePos.z > fieldSize.y)
		m_pPongLeft->GetTransform()->Translate(leftPeddlePos.x, leftPeddlePos.y, fieldSize.y);
	if (leftPeddlePos.z < -fieldSize.y)
		m_pPongLeft->GetTransform()->Translate(leftPeddlePos.x, leftPeddlePos.y, -fieldSize.y);

	if (rightPeddlePos.z > fieldSize.y)
		m_pPongRight->GetTransform()->Translate(rightPeddlePos.x, rightPeddlePos.y, fieldSize.y);
	if (rightPeddlePos.z < -fieldSize.y)
		m_pPongRight->GetTransform()->Translate(rightPeddlePos.x, rightPeddlePos.y, -fieldSize.y);

}

void PongScene::OnGUI()
{
	ImGui::Text("Pong Controls: \n\n");
	ImGui::Text("> Launch Ball: 'SPACE'\n");
	ImGui::Text("> Left Peddle: 'R'/'F'\n");
	ImGui::Text("> Right Peddle: 'UP'/'DOWN'\n");
}
