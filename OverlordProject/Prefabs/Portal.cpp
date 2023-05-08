#include "stdafx.h"
#include "Portal.h"
#include "Materials/PortalMaterial.h"

Portal::Portal(PortalType type, PortalMaterial* material)
	: m_Type{ type }
	, m_pPortalMat{material}
{
}

void Portal::Initialize(const SceneContext& /*sceneContext*/)
{

	//Create portal mesh
	const auto component = new ModelComponent(L"Meshes/Portal.ovm");
	component->SetMaterial(m_pPortalMat);
	m_pMesh = AddComponent<ModelComponent>(component);
	m_pMesh->GetTransform()->Scale(0.03f);

	if (m_Type == PortalType::Blue)
		m_pPortalMat->MakeBlue();
	if (m_Type == PortalType::Orange)
		m_pPortalMat->MakeOrange();

}

void Portal::Update(const SceneContext&)
{
	
}