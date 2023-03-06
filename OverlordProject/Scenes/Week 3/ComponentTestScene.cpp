#include "stdafx.h"
#include "ComponentTestScene.h"

#include "Prefabs/SpherePrefab.h"


void ComponentTestScene::Initialize()
{
	auto& physx = PxGetPhysics();
	auto pBouncyMaterial = physx.createMaterial(0.5f, 0.5f, 1.f);

	//Ground plane
	GameSceneExt::CreatePhysXGroundPlane(*this, pBouncyMaterial);

	//Red sphere (GROUP 0) & (IGNORE GROUP 1 & 2)
	auto pSphereRed = new SpherePrefab(1, 10, XMFLOAT4{ Colors::Red });
	AddChild(pSphereRed);

	pSphereRed->GetTransform()->Translate(0, 30, 0);

	auto pRigidBody = pSphereRed->AddComponent(new RigidBodyComponent());
	pRigidBody->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);
	pRigidBody->SetCollisionGroup(CollisionGroup::Group0);
	pRigidBody->SetCollisionIgnoreGroups(CollisionGroup::Group1 | CollisionGroup::Group2);

	//Green sphere (GROUP 1)
	auto pSphereGreen = new SpherePrefab(1, 10, XMFLOAT4{ Colors::Green });
	AddChild(pSphereGreen);

	pSphereGreen->GetTransform()->Translate(0.5, 20, 0);

	pRigidBody = pSphereGreen->AddComponent(new RigidBodyComponent());
	pRigidBody->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);
	pRigidBody->SetCollisionGroup(CollisionGroup::Group1);
	pRigidBody->SetConstraint(RigidBodyConstraint::TransX | RigidBodyConstraint::TransZ, false);

	//Green sphere (GROUP 2)
	auto pSphereBlue = new SpherePrefab(1, 10, XMFLOAT4{ Colors::Blue });
	AddChild(pSphereBlue);

	pSphereBlue->GetTransform()->Translate(0, 10, 0);

	pRigidBody = pSphereBlue->AddComponent(new RigidBodyComponent());
	pRigidBody->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);
	pRigidBody->SetCollisionGroup(CollisionGroup::Group2);
}


