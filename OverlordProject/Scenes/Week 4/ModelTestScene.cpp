#include "stdafx.h"
#include "ModelTestScene.h"

#include "Materials/DiffuseMaterial.h"


void ModelTestScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;

	//Create new instance of a certain metrial
	DiffuseMaterial* pMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	//Set texture of the material
	pMaterial->SetDiffuseTexture(L"textures/Chair_Dark.dds");

	//GROUNDPLANE
	GameSceneExt::CreatePhysXGroundPlane(*this);

	//CHAIR
	const auto component = new ModelComponent(L"Meshes/Chair.ovm");
	component->SetMaterial(pMaterial);
	m_pChair = new GameObject();
	m_pChair->AddComponent<ModelComponent>(component);

	const auto rbComp = new RigidBodyComponent();
	rbComp->AddCollider(L"Meshes/Chair.ovpt",)
	m_pChair->AddComponent<RigidBodyComponent>(rbComp);

	AddChild(m_pChair);

}


