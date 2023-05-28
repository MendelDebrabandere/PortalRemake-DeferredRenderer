#include "stdafx.h"
#include "PortalGun.h"

#include "Portal.h"
#include "Materials/Portal/PortalMaterial.h"
#include "Scenes/Exam/PortalScene.h"
#include "Prefabs/Character.h"


PortalGun::PortalGun(PortalScene* scene, Character* character)
	: m_pCharacter{character}
	, m_pScene{ scene }
{
}

void PortalGun::ShootGun(PortalType type)
{
	//RAYCAST
	constexpr float Xndc = 0;
	constexpr float Yndc = 0;

	const auto& vpInverse{ XMLoadFloat4x4(&m_pScene->GetSceneContext().pCamera->GetViewProjectionInverse()) };
	const XMVECTOR nearPoint = XMVector3TransformCoord(XMVECTOR{ Xndc, Yndc, 0, 0 }, vpInverse);
	const XMVECTOR farPoint = XMVector3TransformCoord(XMVECTOR{ Xndc, Yndc, 1, 0 }, vpInverse);

	XMFLOAT3 rayStartF;
	XMStoreFloat3(&rayStartF, nearPoint);
	XMFLOAT3 rayEndF;
	XMStoreFloat3(&rayEndF, farPoint);

	const PxVec3 rayStart{ rayStartF.x, rayStartF.y, rayStartF.z };
	const PxVec3 rayEnd{ rayEndF.x - rayStart.x, rayEndF.y - rayStart.y, rayEndF.z - rayStart.z };

	PxQueryFilterData filterData{};
	filterData.data.word0 = ~UINT(CollisionGroup::Group9);

	PxRaycastBuffer hit{};
	if (m_pScene->GetPhysxProxy()->Raycast(rayStart, rayEnd.getNormalized(), PX_MAX_F32, hit, PxHitFlag::eDEFAULT, filterData))
	{
		//Spawn new portal
		Portal* portal{};
		if (type == PortalType::Blue)
			portal = m_pBluePortal;
		else //orange
			portal = m_pOrangePortal;

		//Transform portal
		auto transform = portal->GetTransform();
		const XMFLOAT3 wallNormal = { hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };

		//Transform
		constexpr float wallGap{ 0.1f };
		const XMFLOAT3 position = { hit.block.position.x + wallNormal.x * wallGap, hit.block.position.y + wallNormal.y * wallGap, hit.block.position.z + wallNormal.z * wallGap };
		transform->Translate(position);

		//Rotate
		XMFLOAT3 projectedNormal = { wallNormal.x, 0, wallNormal.z };
		float projectedNormalLength = sqrtf(projectedNormal.x * projectedNormal.x + projectedNormal.z * projectedNormal.z);
		float angleY = atan2f(projectedNormal.x, projectedNormal.z);
		float angleX = atan2f(wallNormal.y, projectedNormalLength);
		transform->Rotate(angleX, angleY, 0, false);
	}
}

void PortalGun::Initialize(const SceneContext&)
{
	//Portal 1
	//***********
	m_pBluePortal = new Portal(PortalType::Blue, nullptr, m_pCharacter);
	m_pBluePortal->GetTransform()->Rotate(0, 0.f, 0);
	m_pBluePortal->GetTransform()->Translate(15, 2.5, 20);

	//Portal 2
	//***********
	m_pOrangePortal = new Portal(PortalType::Orange, m_pBluePortal, m_pCharacter);
	m_pOrangePortal->GetTransform()->Rotate(0, 0.f, 0);
	m_pOrangePortal->GetTransform()->Translate(5, 2.5, 5);

	m_pBluePortal->SetLinkedPortal(m_pOrangePortal);

	m_pScene->AddChild(m_pBluePortal);
	m_pScene->AddChild(m_pOrangePortal);
}

void PortalGun::Update(const SceneContext&)
{
	
}


std::pair<Portal*, Portal*> PortalGun::GetPortals() const
{
	return std::pair(m_pBluePortal, m_pOrangePortal);
}