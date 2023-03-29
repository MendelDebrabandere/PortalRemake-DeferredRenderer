#include "stdafx.h"
#include "BoneObject.h"

BoneObject::BoneObject(BaseMaterial* pMaterial, float length)
	: m_Length{ length }
	, m_pMaterial{ pMaterial }
{
}

void BoneObject::CalculateBindPose()
{
	// Calculate the inverse world matrix
	const XMMATRIX inverseWorldMatrix{ XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&GetTransform()->GetWorld())) };

	// Store the matrix in the bindpose variable
	XMStoreFloat4x4(&m_BindPose, inverseWorldMatrix);

	// Calculate the bind poses of the children
	for (BoneObject* pChild : GetChildren<BoneObject>())
	{
		pChild->CalculateBindPose();
	}
}

void BoneObject::Initialize(const SceneContext& /*sceneContext*/)
{
	//1. Create a GameObject(pEmpty)
	GameObject* pEmpty{ new GameObject() };

	//2. Add pEmpty as a child to the BoneObject
	this->AddChild(pEmpty);

	//3. Add a ModelComponent to pEmpty(pModel), use Bone.ovm
	ModelComponent* modelComp = new ModelComponent(L"Meshes/Bone.ovm");
	pEmpty->AddComponent<ModelComponent>(modelComp);

	//4. Assign the BoneObject's material (m_pMaterial) to pModel
	modelComp->SetMaterial(m_pMaterial);

	//5. Rotate pEmpty -90 degrees around the Y-axis
	pEmpty->GetTransform()->Rotate(0, -90, 0, true);

	//6. Uniformly scale pEmpty to match the BoneObject's length (mLength)
	pEmpty->GetTransform()->Scale(m_Length, m_Length, m_Length);
}

void BoneObject::AddBone(BoneObject* pBone)
{
	//1.
	//The incoming bone (pBone) must be translated along the X-axis so it is located next
	//to the parent bone. The length of the parent bone is defined by m_Length
	pBone->GetTransform()->Translate(m_Length, 0, 0);

	//2.
	//Add pBone as achild to the parent BoneObject (this). This creates the hierarchical
	//relation between both bone, pBone is now a child and 'follows' the transformation
	//of its parent.
	this->AddChild(pBone);
}

