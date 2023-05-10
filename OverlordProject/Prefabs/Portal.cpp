#include "stdafx.h"
#include "Portal.h"
#include "Materials/PortalMaterial.h"

Portal::Portal(PortalType type, PortalMaterial* material, Portal* otherPortal)
	: m_Type{ type }
	, m_pPortalMat{material}
	, m_OtherPortal{otherPortal}
{
}

const float* Portal::GetWorldViewProj() const
{
	return m_pWorldViewProjVar;
}

const float* Portal::GetWorld() const
{ 
	return m_pWorldVar;
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

void Portal::Update(const SceneContext& context)
{

	//auto& d3d11 = context.d3dContext;
	auto world = XMLoadFloat4x4(&GetTransform()->GetWorld());
	const auto viewProjection = XMLoadFloat4x4(&context.pCamera->GetViewProjection());

	m_pWorldVar = reinterpret_cast<float*>(&world);
	auto wvp = world * viewProjection;
	m_pWorldViewProjVar = reinterpret_cast<float*>(&wvp);
}