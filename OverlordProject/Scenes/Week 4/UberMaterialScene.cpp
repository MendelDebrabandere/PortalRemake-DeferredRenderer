#include "stdafx.h"
#include "UberMaterialScene.h"
#include "Materials/UberMaterial.h"


void UberMaterialScene::Initialize()
{
	//m_SceneContext.settings.showInfoOverlay = true;
	//m_SceneContext.settings.drawPhysXDebug = true;
	//m_SceneContext.settings.drawGrid = true;
	m_SceneContext.settings.enableOnGUI = true;

	//Create new instance of a certain metrial
	m_pUberMaterial = MaterialManager::Get()->CreateMaterial<UberMaterial>();

	const auto component = new ModelComponent(L"Meshes/sphere.ovm");
	component->SetMaterial(m_pUberMaterial);

	m_pSphere = new GameObject();
	m_pSphere->AddComponent<ModelComponent>(component);
	m_pSphere->GetTransform()->Scale(15);

	AddChild(m_pSphere);

}
