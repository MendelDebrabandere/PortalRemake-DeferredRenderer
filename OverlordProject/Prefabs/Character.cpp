#include "stdafx.h"
#include "Character.h"

#include "Materials/DiffuseMaterial.h"
#include "Materials/DiffuseMaterial_Skinned.h"

Character::Character(const CharacterDesc& characterDesc) :
	m_CharacterDesc{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime)
{}

void Character::SetTpCooldown(float val)
{
	m_TpCooldown = val;

	auto pFmod = SoundManager::Get()->GetSystem();

	FMOD::Sound* pSound{};
	pFmod->createStream("Resources/sound/Teleport.mp3", FMOD_DEFAULT, nullptr, &pSound);

	pFmod->playSound(pSound, nullptr, false, &m_pChannelTeleport);
	m_pChannelTeleport->setVolume(0.03f);
}

void Character::AddCameraRotation(float yaw, float pitch)
{
	m_TotalYaw += yaw;
	m_TotalPitch += pitch;
}

void Character::SetVelocity(const XMFLOAT3& v)
{
	m_TotalVelocity = v;

	// Update m_CurrentDirection
	m_CurrentDirection.x = m_TotalVelocity.x / m_MoveSpeed;
	m_CurrentDirection.z = m_TotalVelocity.z / m_MoveSpeed;
}

void Character::Initialize(const SceneContext& /*sceneContext*/)
{
	SetTag(L"Player");

	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CharacterDesc.controller));

	//Camera
	m_pCameraObject = AddChild(new FixedCamera());
	m_pCameraComponent = m_pCameraObject->GetComponent<CameraComponent>();
	//m_pCameraComponent->SetActive(true); //Uncomment to make this camera the active camera

	m_pCameraObject->GetTransform()->Translate(0.f, m_CharacterDesc.controller.height * .5f, 0.f);

	//for rendering E on the screen when a box is pickupable
	m_pFont = ContentManager::Load<SpriteFont>(L"SpriteFonts/Consolas_32.fnt");

	//Character Mesh
	//**************
	const auto pCharacterMesh = AddChild(new GameObject);
	const auto pModel = pCharacterMesh->AddComponent(new ModelComponent(L"Meshes/CharacterMesh.ovm"));

	const auto pSkinnedMaterialTorso = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	pSkinnedMaterialTorso->SetDiffuseTexture(L"Textures/character/chell_torso_diffuse.png");
	const auto pSkinnedMaterialLegs = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	pSkinnedMaterialLegs->SetDiffuseTexture(L"Textures/character/chell_legs_diffuse.png");
	const auto pSkinnedMaterialHead = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	pSkinnedMaterialHead->SetDiffuseTexture(L"Textures/character/chell_head_diffuse.png");
	const auto pSkinnedMaterialHair = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	pSkinnedMaterialHair->SetDiffuseTexture(L"Textures/character/chell_hair.png");
	const auto pSkinnedMaterialEye = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	pSkinnedMaterialEye->SetDiffuseTexture(L"Textures/character/eyeball_l.png");

	pModel->SetMaterial(pSkinnedMaterialHead, 0);
	pModel->SetMaterial(pSkinnedMaterialEye, 1);
	pModel->SetMaterial(pSkinnedMaterialEye, 2);
	pModel->SetMaterial(pSkinnedMaterialTorso, 3);
	pModel->SetMaterial(pSkinnedMaterialLegs, 4);
	pModel->SetMaterial(pSkinnedMaterialHair, 5);

	pCharacterMesh->GetTransform()->Translate(-0.3f, -1.9f, -1.f);
	pCharacterMesh->GetTransform()->Rotate(0, 180.f, 0.f);

	m_pAnimator = pModel->GetAnimator();
	if (m_pAnimator)
	{
		m_CurrClip = 0;
		m_pAnimator->SetAnimation(m_CurrClip);
		m_pAnimator->Play();
	}



}

void Character::Update(const SceneContext& sceneContext)
{
	if (m_pCameraComponent->IsActive())
	{
		//constexpr float epsilon{ 0.01f }; //Constant that can be used to compare if a float is near zero
		const float elapsedSec{ sceneContext.pGameTime->GetElapsed() };

		//***************
		//HANDLE INPUT
		
		//## Input Gathering (move)
		XMFLOAT2 move;
		//move.y should contain a 1 (Forward) or -1 (Backward) based on the active input (check corresponding actionId in m_CharacterDesc)
		move.y = sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveForward) ? 1.f : 0.f;
		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveBackward))
			move.y = -1.f;
		//Optional: if move.y is near zero (abs(move.y) < epsilon), you could use the ThumbStickPosition.y for movement

		//move.x should contain a 1 (Right) or -1 (Left) based on the active input (check corresponding actionId in m_CharacterDesc)
		move.x = sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveRight) ? 1.f : 0.f;
		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveLeft))
			move.x = -1.f;
		//Optional: if move.x is near zero (abs(move.x) < epsilon), you could use the Left ThumbStickPosition.x for movement

		//*************************************
		//ANIMATION
		if (m_pAnimator && m_JumpAnimTime <= 0.f)
		{
			int lastClip = m_CurrClip;
			if (move.y > 0.2f)
			{
				if (m_CurrClip != 1)
					m_CurrClip = 1;
			}
			else if (move.y < -0.2f)
			{
				if (m_CurrClip != 2)
					m_CurrClip = 2;
			}
			else if (move.x > 0.2f)
			{
				if (m_CurrClip != 4)
					m_CurrClip = 4;
			}
			else if (move.x < -0.2f)
			{
				if (m_CurrClip != 3)
					m_CurrClip = 3;
			}
			else
			{
				if (m_CurrClip != 0)
					m_CurrClip = 0;
			}

			if (lastClip != m_CurrClip)
				m_pAnimator->SetAnimation(m_CurrClip);
		}


		//## Input Gathering (look)
		XMFLOAT2 look{ 0.f, 0.f };
		//Only if the Left Mouse Button is Down >
		//if (InputManager::IsMouseButton(InputState::down, 1))
		//{
		constexpr float mouseSens{ 0.5f };
			// Store the MouseMovement in the local 'look' variable (cast is required)
			auto mouseMovement = InputManager::GetMouseMovement();
			look.x = static_cast<float>(mouseMovement.x) * mouseSens;
			look.y = static_cast<float>(mouseMovement.y) * mouseSens;
		//}
		//Optional: in case look.x AND look.y are near zero, you could use the Right ThumbStickPosition for look

		//************************
		//GATHERING TRANSFORM INFO

		//Retrieve the TransformComponent
		TransformComponent* pTransform{ GetTransform() };
		//Retrieve the forward & right vector (as XMVECTOR) from the TransformComponent
		const XMVECTOR forward{ XMLoadFloat3(&pTransform->GetForward()) };
		const XMVECTOR right{ XMLoadFloat3(&pTransform->GetRight()) };

		//***************
		//CAMERA ROTATION

		//Adjust the TotalYaw (m_TotalYaw) & TotalPitch (m_TotalPitch) based on the local 'look' variable
		//Make sure this calculated on a framerate independent way and uses CharacterDesc::rotationSpeed.
		const float rotationAmount{ elapsedSec * m_CharacterDesc.rotationSpeed };
		m_TotalYaw += look.x * rotationAmount;
		m_TotalPitch += look.y * rotationAmount;
		if (m_TotalPitch >= 90)
			m_TotalPitch = 90;
		else if (m_TotalPitch <= -90)
			m_TotalPitch = -90;
		//Rotate this character based on the TotalPitch (X) and TotalYaw (Y)
		GetTransform()->Rotate(0.0f, m_TotalYaw, 0.0f);
		sceneContext.pCamera->GetTransform()->Rotate(m_TotalPitch, 0.0f, 0.0f);

		//********
		//MOVEMENT

		//## Horizontal Velocity (Forward/Backward/Right/Left)
		//Calculate the current move acceleration for this frame (m_MoveAcceleration * ElapsedTime)
		const float acceleration{ m_MoveAcceleration * elapsedSec };
		//If the character is moving (= input is pressed)
		if (abs(move.x) > 0.0f || abs(move.y) > 0.0f)
		{
			//Calculate & Store the current direction (m_CurrentDirection) >> based on the forward/right vectors and the pressed input
			const XMVECTOR direction{ forward * move.y + right * move.x };
			XMStoreFloat3(&m_CurrentDirection, direction);
			//Increase the current MoveSpeed with the current Acceleration (m_MoveSpeed)
			m_MoveSpeed += acceleration;
			//Make sure the current MoveSpeed stays below the maximum MoveSpeed (CharacterDesc::maxMoveSpeed)
			m_MoveSpeed = std::min(m_MoveSpeed, m_CharacterDesc.maxMoveSpeed);
		}
		//Else (character is not moving, or stopped moving)
		else
		{
			//Decrease the current MoveSpeed with the current Acceleration (m_MoveSpeed)
			m_MoveSpeed -= acceleration;
			//Make sure the current MoveSpeed doesn't get smaller than zero
			m_MoveSpeed = std::max(m_MoveSpeed, 0.f);
		}

		//Now we can calculate the Horizontal Velocity which should be stored in m_TotalVelocity.xz
		//Calculate the horizontal velocity (m_CurrentDirection * MoveSpeed)
		//Set the x/z component of m_TotalVelocity (horizontal_velocity x/z)
		//It's important that you don't overwrite the y component of m_TotalVelocity (contains the vertical velocity)
		if (m_pControllerComponent->GetCollisionFlags() & PxControllerCollisionFlag::eCOLLISION_DOWN)
		{
			m_TotalVelocity.x = m_CurrentDirection.x * m_MoveSpeed;
			m_TotalVelocity.z = m_CurrentDirection.z * m_MoveSpeed;
			m_JumpAnimTime = 0.f;
		}

		//## Vertical Movement (Jump/Fall)
		//If the Controller Component is NOT grounded (= freefall)
		if (!(m_pControllerComponent->GetCollisionFlags() & PxControllerCollisionFlag::eCOLLISION_DOWN))
		{
			//Decrease the y component of m_TotalVelocity with a fraction (ElapsedTime) of the Fall Acceleration (m_FallAcceleration)
			m_TotalVelocity.y -= elapsedSec * m_FallAcceleration;
			//Make sure that the minimum speed stays above -CharacterDesc::maxFallSpeed (negative!)
			//m_TotalVelocity.y = std::max(m_TotalVelocity.y, -m_CharacterDesc.maxFallSpeed);
			//WHY, I WANT TO FALL AT INFIINITE SPEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEDS
			m_JumpAnimTime += elapsedSec;
			if (m_JumpAnimTime >= 0.55f && m_CurrClip != 6)
			{
				m_CurrClip = 6;
				m_pAnimator->SetAnimation(m_CurrClip);
			}
		}
		//Else If the jump action is triggered
		else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump))
		{
			//Set m_TotalVelocity.y equal to CharacterDesc::JumpSpeed
			m_TotalVelocity.y = m_CharacterDesc.JumpSpeed;
			m_CurrClip = 5;
			m_pAnimator->SetAnimation(m_CurrClip);
			m_JumpAnimTime = 0.1f;
			//Set the time because otherwise it gets set to idle next update
		}
		//Else (=Character is grounded, no input pressed)
		else
			//m_TotalVelocity.y is zero
			m_TotalVelocity.y = -0.1f;

		//************
		//DISPLACEMENT

		//The displacement required to move the Character Controller (ControllerComponent::Move) can be calculated using our TotalVelocity (m/s)
		//Calculate the displacement (m) for the current frame and move the ControllerComponent
		const XMFLOAT3 displacement
		{
			m_TotalVelocity.x * elapsedSec,
			m_TotalVelocity.y * elapsedSec,
			m_TotalVelocity.z * elapsedSec
		};
		m_pControllerComponent->Move(displacement);

		//The above is a simple implementation of Movement Dynamics, adjust the code to further improve the movement logic and behaviour.
		//Also, it can be usefull to use a seperate RayCast to check if the character is grounded (more responsive)

	}

	//TP cooldown
	if (m_TpCooldown >= 0.f)
	{
		m_TpCooldown -= sceneContext.pGameTime->GetElapsed();
	}

	//Box pickup/drop logic
	if (InputManager::IsKeyboardKey(InputState::pressed, 'E') && m_pHoldingCube)
	{
		m_pHoldingCube = nullptr;
	}
	else
	{
		//Get the object the camera is looking at
		GameObject* go = m_pCameraComponent->Pick(CollisionGroup::Group9);
		if (go)
		{
			auto tag = go->GetTag();
			if (tag == L"Cube")
			{
				//IT IS A CUBE
				auto xmPlayerPos = XMLoadFloat3(&GetTransform()->GetWorldPosition());
				auto xmCubeTransform = XMLoadFloat3(&go->GetTransform()->GetWorldPosition());

				XMVECTOR diff = XMVectorSubtract(xmPlayerPos, xmCubeTransform);

				float distance = XMVectorGetX(XMVector3Length(diff));

				if (distance <= 10.f) //BOX PICKUP RANGE
				{
					TextRenderer::Get()->DrawText(m_pFont, StringUtil::utf8_decode("E"), XMFLOAT2{ 633,390 }, XMFLOAT4{ 0,0,0,1 });

					if (InputManager::IsKeyboardKey(InputState::pressed, 'E'))
					{
						if (m_pHoldingCube == nullptr)
							m_pHoldingCube = go;
					}
				}
			}
		}
	}


	//holding cube logic
	if (m_pHoldingCube)
	{
		constexpr float objDist{ 6 };

		const XMFLOAT3& camPos = m_pCameraComponent->GetTransform()->GetWorldPosition();

		const XMFLOAT3 camFor = m_pCameraComponent->GetTransform()->GetForward();
		XMFLOAT3 objectPos{ camPos.x + camFor.x * objDist,
							camPos.y + camFor.y * objDist,
							camPos.z + camFor.z * objDist };

		m_pHoldingCube->GetTransform()->Translate(objectPos);

	}
}

void Character::DrawImGui()
{
	if (ImGui::CollapsingHeader("Character"))
	{
		ImGui::Text(std::format("Move Speed: {:0.1f} m/s", m_MoveSpeed).c_str());
		ImGui::Text(std::format("Fall Speed: {:0.1f} m/s", m_TotalVelocity.y).c_str());

		ImGui::Text(std::format("Move Acceleration: {:0.1f} m/s2", m_MoveAcceleration).c_str());
		ImGui::Text(std::format("Fall Acceleration: {:0.1f} m/s2", m_FallAcceleration).c_str());

		const float jumpMaxTime = m_CharacterDesc.JumpSpeed / m_FallAcceleration;
		const float jumpMaxHeight = (m_CharacterDesc.JumpSpeed * jumpMaxTime) - (0.5f * (m_FallAcceleration * powf(jumpMaxTime, 2)));
		ImGui::Text(std::format("Jump Height: {:0.1f} m", jumpMaxHeight).c_str());

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Move Speed (m/s)", &m_CharacterDesc.maxMoveSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Move Acceleration Time (s)", &m_CharacterDesc.moveAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_MoveAcceleration = m_CharacterDesc.maxMoveSpeed / m_CharacterDesc.moveAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Fall Speed (m/s)", &m_CharacterDesc.maxFallSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Fall Acceleration Time (s)", &m_CharacterDesc.fallAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_FallAcceleration = m_CharacterDesc.maxFallSpeed / m_CharacterDesc.fallAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		ImGui::DragFloat("Jump Speed", &m_CharacterDesc.JumpSpeed, 0.1f, 0.f, 0.f, "%.1f");
		ImGui::DragFloat("Rotation Speed (deg/s)", &m_CharacterDesc.rotationSpeed, 0.1f, 0.f, 0.f, "%.1f");

		bool isActive = m_pCameraComponent->IsActive();
		if(ImGui::Checkbox("Character Camera", &isActive))
		{
			m_pCameraComponent->SetActive(isActive);
		}
	}
}

void Character::SetCameraActive(bool newValue)
{
	m_pCameraComponent->SetActive(newValue);
}

void Character::SetCollisionGroup(CollisionGroup group)
{
	m_pControllerComponent->SetCollisionGroup(group);
}

void Character::SetCollisionIgnoreGroup(CollisionGroup group)
{
	m_pControllerComponent->SetCollisionIgnoreGroup(group);
}
