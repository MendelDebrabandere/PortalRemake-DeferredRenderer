#include "stdafx.h"
#include "SpikyScene.h"

#include "Materials/SpikyMaterial.h"


void SpikyScene::Initialize()
{
	//m_SceneContext.settings.showInfoOverlay = true;
	//m_SceneContext.settings.drawPhysXDebug = true;
	//m_SceneContext.settings.drawGrid = true;
	m_SceneContext.settings.enableOnGUI = true;

	//Create new instance of a certain metrial
	m_pMaterial = MaterialManager::Get()->CreateMaterial<SpikyMaterial>();

	//Get materialID
	//const UINT materialID = pMaterial->GetMaterialId();

	//Retrieve an effect variable
	//uto pColorVariable = pMaterial->GetVariable(L"gColor");

	//Set variable (propper way to embed these calls in the class itself)
	//pMaterial->SetVariable_Vector(L"gColor", XMFLOAT4{ Colors::AliceBlue });

	const auto component = new ModelComponent(L"Meshes/OctaSphere.ovm");
	component->SetMaterial(m_pMaterial);

	m_pSphere = new GameObject();
	m_pSphere->AddComponent<ModelComponent>(component);
	m_pSphere->GetTransform()->Scale(15);

	AddChild(m_pSphere);

	//Get material reference
	//ColorMaterial* pMaterialRef = MaterialManager::Get()->GetMaterial<ColorMaterial>(materialID);

	//Remove Material (All remaining materials are automatically destroyed during shutdown)
	//MaterialManager::Get()->RemoveMaterial(materialID, true);

}

void SpikyScene::Update()
{
	const float deltaTime{ m_SceneContext.pGameTime->GetTotal() };
	const float rotationSpeed{ 25 * deltaTime };

	m_pSphere->GetTransform()->GetTransform()->Rotate(0, rotationSpeed, 0);
}

void SpikyScene::OnGUI()
{
	m_pMaterial->DrawImGui();
}
