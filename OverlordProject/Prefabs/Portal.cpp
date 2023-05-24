#include "stdafx.h"
#include "Portal.h"
#include "Materials/Portal/PortalMaterial.h"
#include "Prefabs/Character.h"

Portal::Portal(PortalType type, PortalMaterial* material,Character* character)
	: m_Type{ type }
	, m_pPortalMat{ material }
	, m_pCharacter{ character }
{
}


void Portal::Initialize(const SceneContext& sceneContext)
{

	//Create portal mesh
	const auto component = new ModelComponent(L"Meshes/Portal.ovm");
	component->SetMaterial(m_pPortalMat);
	m_pMesh = AddComponent<ModelComponent>(component);
	m_pMesh->GetTransform()->Scale(0.03f);

	if (m_Type == PortalType::Blue)
		m_pPortalMat->MakeBlue();
	else if (m_Type == PortalType::Orange)
		m_pPortalMat->MakeOrange();

	m_pCameraObject = AddChild(new FixedCamera());
	m_pCameraComponent = m_pCameraObject->GetComponent<CameraComponent>();

	//im sorry ;( i had to use const cast
	SceneContext* SceneCtx = const_cast<SceneContext*>(&sceneContext);
	if (m_Type == PortalType::Blue)
		SceneCtx->pPortal1Camera = m_pCameraComponent;
	else if (m_Type == PortalType::Orange)
		SceneCtx->pPortal2Camera = m_pCameraComponent;
}

void Portal::Update(const SceneContext&/* context*/)
{
}