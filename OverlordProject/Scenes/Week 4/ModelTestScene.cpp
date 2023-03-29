#include "stdafx.h"
#include "ModelTestScene.h"

#include "Materials/DiffuseMaterial.h"


void ModelTestScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;

	auto pPhysMat{ PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f) };

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
	m_pChair->GetComponent<TransformComponent>()->Translate(0.0f, 15.0f, 0.0f);
	m_pChair->GetComponent<TransformComponent>()->Rotate(45, 45, 45);
	//RigidBody
	const auto pConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/Chair.ovpc");
	const auto convexGeometry{ PxConvexMeshGeometry{ pConvexMesh } };
	m_pChair->AddComponent(new RigidBodyComponent())->AddCollider(convexGeometry, *pPhysMat);
	AddChild(m_pChair);

}


