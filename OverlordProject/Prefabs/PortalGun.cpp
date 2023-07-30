#include "stdafx.h"
#include "PortalGun.h"

#include "Portal.h"
#include "Materials/DiffuseMaterial.h"
#include "Scenes/PortalRemake/PortalScene.h"
#include "Prefabs/Character.h"


PortalGun::PortalGun(PortalScene* scene, Character* character)
	: m_pCharacter{character}
	, m_pScene{ scene }
{
}

void PortalGun::ShootGun(PortalType type)
{
	//Spawn particle
	SpawnParticle(type);

	//RAYCAST
	//We do a custom raycast and not just cameracomponent.pick
	//because we need the normal and location of te hit as well
	constexpr float Xndc = 0;
	constexpr float Yndc = 0;

	const auto& vpInverse{ XMLoadFloat4x4(&m_pScene->GetSceneContext().pCamera->GetViewProjectionInverse()) };
	const XMVECTOR nearPoint = XMVector3TransformCoord(XMVECTOR{ Xndc, Yndc, 0, 0 }, vpInverse);
	const XMVECTOR farPoint = XMVector3TransformCoord(XMVECTOR{ Xndc, Yndc, 1, 0 }, vpInverse);

	XMFLOAT3 rayStartF;
	XMStoreFloat3(&rayStartF, nearPoint);
	XMFLOAT3 rayEndF;
	XMStoreFloat3(&rayEndF, farPoint);

	PxVec3 rayStart{ rayStartF.x, rayStartF.y, rayStartF.z };
	const PxVec3 rayEnd{ rayEndF.x - rayStart.x, rayEndF.y - rayStart.y, rayEndF.z - rayStart.z };
	rayStart += rayEnd.getNormalized() * 2;

	PxQueryFilterData filterData{};
	filterData.data.word0 = ~UINT(CollisionGroup::Group9);

	PxRaycastBuffer hit{};
	if (m_pScene->GetPhysxProxy()->Raycast(rayStart, rayEnd.getNormalized(), PX_MAX_F32, hit, PxHitFlag::eDEFAULT, filterData))
	{
		//Anything that shouldnt have portals on them (interactible objects) have a name
		auto RBName = hit.block.actor->getName();
		if (RBName)
			return;

		//Spawn new portal
		Portal* portal{};
		if (type == PortalType::Blue)
		{
			auto pFmod = SoundManager::Get()->GetSystem();

			FMOD::Sound* pSound{};
			pFmod->createStream("Resources/sound/BluePortal.mp3", FMOD_DEFAULT, nullptr, &pSound);

			pFmod->playSound(pSound, nullptr, false, &m_pChannelBlueGun);
			m_pChannelBlueGun->setVolume(0.1f);

			portal = m_pBluePortal;
		}
		else //orange
		{
			auto pFmod = SoundManager::Get()->GetSystem();

			FMOD::Sound* pSound{};
			pFmod->createStream("Resources/sound/OrangePortal.mp3", FMOD_DEFAULT, nullptr, &pSound);

			pFmod->playSound(pSound, nullptr, false, &m_pChannelOrangeGun);
			m_pChannelOrangeGun->setVolume(0.1f);

			portal = m_pOrangePortal;
		}

		//Transform portal
		auto transform = portal->GetTransform();
		const XMFLOAT3 wallNormal = { hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };

		//Transform
		constexpr float wallGap{ 0.0f };
		const XMFLOAT3 position = { hit.block.position.x + wallNormal.x * wallGap, hit.block.position.y + wallNormal.y * wallGap, hit.block.position.z + wallNormal.z * wallGap };
		transform->Translate(position);

		//Rotate
		XMFLOAT3 projectedNormal = { wallNormal.x, 0, wallNormal.z };
		float projectedNormalLength = sqrtf(projectedNormal.x * projectedNormal.x + projectedNormal.z * projectedNormal.z);
		float angleY = atan2f(projectedNormal.x, projectedNormal.z);
		float angleX = atan2f(wallNormal.y, projectedNormalLength);
		constexpr float PI = 3.141592653589793238462643383279502884197f;
		transform->Rotate(angleX, PI + angleY, 0, false);

		//Set wall
		portal->SetWall(hit.block.shape);
	}
}

void PortalGun::Initialize(const SceneContext&)
{
	//Portal 1
	//***********
	m_pBluePortal = new Portal(PortalType::Blue, nullptr, m_pCharacter);
	m_pBluePortal->GetTransform()->Rotate(0, 0, 0);
	m_pBluePortal->GetTransform()->Translate(25.f, -7.3f, 39.2f);

	//Portal 2
	//***********
	m_pOrangePortal = new Portal(PortalType::Orange, m_pBluePortal, m_pCharacter);
	m_pOrangePortal->GetTransform()->Rotate(0, -90.f, 0);
	m_pOrangePortal->GetTransform()->Translate(5.f, -7.3f, 35.f);

	m_pBluePortal->SetLinkedPortal(m_pOrangePortal);

	m_pScene->AddChild(m_pBluePortal);
	m_pScene->AddChild(m_pOrangePortal);

	//Gun mesh
	m_pGunMesh = m_pCharacter->GetCamera()->AddChild(new GameObject);
	const auto pGunModel = m_pGunMesh->AddComponent(new ModelComponent(L"Meshes/PortalGun.ovm"));

	const auto pMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	pMaterial->SetDiffuseTexture(L"Textures/PortalGun.png");

	pGunModel->SetMaterial(pMaterial, 0);

	m_pGunMesh->GetTransform()->Translate(0.3f, -0.25f, 0.3f);
	m_pGunMesh->GetTransform()->Rotate(90, 0, 180);
	m_pGunMesh->GetTransform()->Scale(0.5f, 0.5f, 0.5f);
}

void PortalGun::Update(const SceneContext& sceneContext)
{
	if (m_pEmitterObj)
	{
		particleTimer += sceneContext.pGameTime->GetElapsed();
		if (particleTimer >= 0.1f)
		{
			particleTimer = 0.f;
			RemoveChild(m_pEmitterObj, true);
			m_pEmitterObj = nullptr;
		}
	}
}

void PortalGun::SpawnParticle(PortalType type)
{
	//Particle System for shooting gun
	ParticleEmitterSettings settings{};
	settings.velocity = { 0.f,0.f,0.f };
	settings.minSize = 0.6f;
	settings.maxSize = 0.6f;
	settings.minEnergy = 0.1f;
	settings.maxEnergy = 0.1f;
	settings.minScale = 1.f;
	settings.maxScale = 1.f;
	settings.minEmitterRadius = .0f;
	settings.maxEmitterRadius = .0f;
	settings.color = { 1.f,1.f,1.f, .6f };

	m_pEmitterObj = AddChild(new GameObject());
	if (type == PortalType::Blue)
		m_pEmitter = m_pEmitterObj->AddComponent(new ParticleEmitterComponent(L"Textures/BlueEffect.png", settings, 1));
	else
		m_pEmitter = m_pEmitterObj->AddComponent(new ParticleEmitterComponent(L"Textures/OrangeEffect.png", settings, 1));

	//SET THE POS OF THE EMITTER
	XMFLOAT3 startVec = { 0,-1,0 };

	XMFLOAT4 rot = m_pGunMesh->GetTransform()->GetWorldRotation();
	XMFLOAT3 pos = m_pGunMesh->GetTransform()->GetWorldPosition();

	//rotate unit vector with rotation quaternion
	XMVECTOR xmStartVec = XMLoadFloat3(&startVec);
	XMVECTOR xmRot = XMLoadFloat4(&rot);

	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(xmRot);

	XMVECTOR rotatedVec = XMVector3Transform(xmStartVec, rotationMatrix);

	XMFLOAT3 result;
	XMStoreFloat3(&result, rotatedVec);

	//Adding offset to position
	constexpr float dist{ 0.7f };

	pos = XMFLOAT3{ pos.x + result.x * dist,
					pos.y + result.y * dist,
					pos.z + result.z * dist };

	m_pEmitterObj->GetTransform()->Translate(pos);
}


std::pair<Portal*, Portal*> PortalGun::GetPortals() const
{
	return std::pair(m_pBluePortal, m_pOrangePortal);
}

void PortalGun::SetBluePortalWall(PxShape* wall)
{
	m_pBluePortal->SetWall(wall);
}

void PortalGun::SetOrangePortalWall(PxShape* wall)
{
	m_pOrangePortal->SetWall(wall);
}
