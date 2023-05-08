#include "stdafx.h"
#include "PortalGun.h"

#include "Portal.h"
#include "Materials/PortalMaterial.h"
#include "Scenes/Exam/PortalScene.h"


void PortalGun::ShootGun(PortalScene* scene, PortalType type)
{
	//RAYCAST
	constexpr float Xndc = 0;
	constexpr float Yndc = 0;

	const auto& vpInverse{ XMLoadFloat4x4(&scene->GetSceneContext().pCamera->GetViewProjectionInverse()) };
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
	if (scene->GetPhysxProxy()->Raycast(rayStart, rayEnd.getNormalized(), PX_MAX_F32, hit, PxHitFlag::eDEFAULT, filterData))
	{
		//delete old portal
		if (m_PortalPtrs[int(type)])
			scene->RemoveChild(m_PortalPtrs[int(type)]);

		//Spawn new portal
		auto portal = scene->AddChild(new Portal(type, m_PortalMaterialPtrs[int(type)]));
		auto transform = portal->GetTransform();

		//Transform
		const XMFLOAT3 position = { hit.block.position.x, hit.block.position.y, hit.block.position.z };
		transform->Translate(position);

		//Rotate
		const XMFLOAT3 wallNormal = { hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };
		XMFLOAT3 projectedNormal = { wallNormal.x, 0, wallNormal.z };
		float projectedNormalLength = sqrtf(projectedNormal.x * projectedNormal.x + projectedNormal.z * projectedNormal.z);
		float angleY = atan2f(projectedNormal.x, projectedNormal.z);
		float angleX = atan2f(wallNormal.y, projectedNormalLength);
		transform->Rotate(angleX, angleY, 0, false);

		//save new portal
		m_PortalPtrs[int(type)] = portal;
	}
}

void PortalGun::Initialize(const SceneContext&)
{
	m_PortalPtrs.resize(2);

	//Create new instance of a certain metrial
	m_PortalMaterialPtrs.resize(2);

	m_PortalMaterialPtrs[0] = MaterialManager::Get()->CreateMaterial<PortalMaterial>();
	m_PortalMaterialPtrs[1] = MaterialManager::Get()->CreateMaterial<PortalMaterial>();

}

void PortalGun::Update(const SceneContext&)
{
	
}
