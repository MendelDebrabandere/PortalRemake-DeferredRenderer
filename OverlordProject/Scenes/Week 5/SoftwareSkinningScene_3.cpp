#include "stdafx.h"
#include "SoftwareSkinningScene_3.h"

#include "prefabs/BoneObject.h"
#include "Materials/ColorMaterial.h"

void SoftwareSkinningScene_3::Initialize()
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


	///////////
	m_pBone0->AddComponent(new MeshDrawComponent());
	m_pBone0->CalculateBindPose();

	GameObject* pBoxDrawer{ new GameObject() };
	m_pMeshDrawer = pBoxDrawer->AddComponent(new MeshDrawComponent(24, true));
	AddChild(pBoxDrawer);
	InitializeVertices(boneSize);
}

void SoftwareSkinningScene_3::Update()
{

	// ROTATE BONES
	if (m_ManualMode)
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


	// calculate the transformed vertex for every vertex in our SkinnedVertex vector
	const auto bone0Transform = XMLoadFloat4x4(&m_pBone0->GetBindPose()) * XMLoadFloat4x4(&m_pBone0->GetTransform()->GetWorld());
	const auto bone1Transform = XMLoadFloat4x4(&m_pBone1->GetBindPose()) * XMLoadFloat4x4(&m_pBone1->GetTransform()->GetWorld());

	for (int idx{}; idx < m_SkinnedVertices.size(); idx += 1)
	{
		VertexSoftwareSkinned& skinnedVertex{ m_SkinnedVertices[idx] };

		//const XMMATRIX& curBoneTransform{ idx / 24 == 0 ? bone0Transform : bone1Transform };
		const XMMATRIX& curBoneTransform{ bone0Transform * skinnedVertex.blendWeight0 + bone1Transform * skinnedVertex.blendWeight1 };

		const XMVECTOR transformedVertex{ XMVector3TransformCoord(XMLoadFloat3(&skinnedVertex.originalVertex.Position), curBoneTransform) };
		XMStoreFloat3(&skinnedVertex.transformedVertex.Position, transformedVertex);
	}


	// Update meshDrawer
	m_pMeshDrawer->RemoveTriangles();
	for (int idx{}; idx < m_SkinnedVertices.size(); idx += 4)
	{
		QuadPosNormCol quad{ m_SkinnedVertices[idx].transformedVertex,
							m_SkinnedVertices[idx + 1].transformedVertex ,
							m_SkinnedVertices[idx + 2].transformedVertex,
							m_SkinnedVertices[idx + 3].transformedVertex };
		m_pMeshDrawer->AddQuad(quad);
	}
	m_pMeshDrawer->UpdateBuffer();
}

void SoftwareSkinningScene_3::OnGUI()
{
	//Add some ImGui widget so you can manually adjust the rotations of both bones. Also
	//add a checkbox to toggle between manual and automatic mode. You can of course
	//add some extra datamembers to the class to achieve this behaviour.

	ImGui::Checkbox("Manual mode", &m_ManualMode);
	ImGui::SliderFloat("First bone angle", &m_BoneRotation0, -3.14f / 2, 3.14f / 2);
	ImGui::SliderFloat("Second bone angle", &m_BoneRotation1, -3.14f / 2, 3.14f / 2);
}




void SoftwareSkinningScene_3::InitializeVertices(float length)
{
	constexpr float leftVertW{ 1 };
	constexpr float middleVertW{ 0.5f };
	constexpr float rightVertW{ 0 };

	auto pos = XMFLOAT3(length / 2.f, 0.f, 0.f);
	const auto offset = XMFLOAT3(length / 2.f, 2.5f, 2.5f);
	auto col = XMFLOAT4(1.f, 0.f, 0.f, 0.5f);
#pragma region BOX 1
	//FRONT
	XMFLOAT3 norm = { 0, 0, -1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, leftVertW);
	//BACK
	norm = { 0, 0, 1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	//TOP
	norm = { 0, 1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, leftVertW);
	//BOTTOM
	norm = { 0, -1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, leftVertW);
	//LEFT
	norm = { -1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, leftVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, leftVertW);
	//RIGHT
	norm = { 1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
#pragma endregion

	col = { 0.f, 1.f, 0.f, 0.5f };
	pos = { 22.5f, 0.f, 0.f };
#pragma region BOX 2
	//FRONT
	norm = { 0, 0, -1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	//BACK
	norm = { 0, 0, 1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, rightVertW);
	//TOP
	norm = { 0, 1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	//BOTTOM
	norm = { 0, -1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	//LEFT
	norm = { -1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, middleVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, middleVertW);
	//RIGHT
	norm = { 1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, rightVertW);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, rightVertW);
#pragma endregion
}