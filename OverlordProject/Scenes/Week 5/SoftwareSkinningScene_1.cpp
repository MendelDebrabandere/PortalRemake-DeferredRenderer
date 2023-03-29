#include "stdafx.h"
#include "SoftwareSkinningScene_1.h"
#include "prefabs/BoneObject.h"
#include "Materials/ColorMaterial.h"

void SoftwareSkinningScene_1::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;


	//1. Create a ColorMaterial that will be used for both BoneObjects
	ColorMaterial* pMaterial{ MaterialManager::Get()->CreateMaterial<ColorMaterial>() };
	pMaterial->SetColor(XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.f });

	//2. Create a new GameObject which will act as the root for our Bone Hierarchy (pRoot)
	GameObject* pRoot{ new GameObject() };
	AddChild(pRoot);

	constexpr float boneSize{ 15.f };

	//3. Initialize the first bone (m_pBone0), use the premade ColorMaterial and a length of
	//   15.f. This bone is then added to the pRoot as a child.
	m_pBone0 = new BoneObject(pMaterial, boneSize);
	pRoot->AddChild(m_pBone0);

	//4. Initialize the second bone (m_pBone1), same parameters as m_pBone0. This bone
	//   is added as a child to m_pBone0 BUT use the AddBone function instead of the
	//   AddChild function! (The AddBone function will add this bone as a child and
	//   translate it to the end of the parent bone)
	m_pBone1 = new BoneObject(pMaterial, boneSize);
	m_pBone0->AddBone(m_pBone1);

	//5. Make sure to add pRoot to the scenegraph using AddChild

}

void SoftwareSkinningScene_1::Update()
{
	//The update function takes care of animating both bones. The datamember
	//m_BoneRotation fluctuates between -45 and 45 degrees at a speed of 45 degrees per
	//second. Use m_RotationSign (must be -1 of 1) to keep track of the update rate
	//(increment or decrement the rotation). Both bones are rotated along the Z-Axis.
	//m_pBone0 > Rotate(0, 0, m_BoneRotation)
	//m_pBone1 > Rotate(0, 0, -m_BoneRotation * 2.f)

	if(m_ManualMode)
	{
		m_pBone0->GetTransform()->Rotate(0, 0, 180 * sinf(m_BoneRotation0));
		m_pBone1->GetTransform()->Rotate(0, 0, 180 * sinf(m_BoneRotation1));
	}
	else
	{
		constexpr float rotationSpeed{ 1 };
		m_BoneRotation = m_SceneContext.pGameTime->GetTotal() * rotationSpeed;

		m_pBone0->GetTransform()->Rotate(0, 0, 45 * sinf(m_BoneRotation));
		m_pBone1->GetTransform()->Rotate(0, 0, 45 * -2 * sinf(m_BoneRotation));
	}

}

void SoftwareSkinningScene_1::OnGUI()
{
	//Add some ImGui widget so you can manually adjust the rotations of both bones. Also
	//add a checkbox to toggle between manual and automatic mode. You can of course
	//add some extra datamembers to the class to achieve this behaviour.

	ImGui::Checkbox("Manual mode", &m_ManualMode);
	ImGui::SliderFloat("First bone angle", &m_BoneRotation0, -3.14f / 2, 3.14f / 2);
	ImGui::SliderFloat("Second bone angle", &m_BoneRotation1, -3.14f / 2, 3.14f / 2);
}