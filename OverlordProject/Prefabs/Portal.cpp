#include "stdafx.h"
#include "Portal.h"

Portal::Portal(PortalType type)
	:m_Type{ type }
{
}

void Portal::Initialize(const SceneContext& /*sceneContext*/)
{
	constexpr float halfPlaneHeight{ 3.5f };
	constexpr float halfPlaneWidth{ 1.5f };

	//Create Plane
	m_Plane = new MeshDrawComponent(2);

	XMFLOAT4 color{};
	if (m_Type == PortalType::Orange)
		color = XMFLOAT4(1.f, 0.7f, 0.2f, 1.f);
	else
		color = XMFLOAT4(0.2f, 0.2f, 1.f, 1.f);

	//FRONT
	m_Plane->AddQuad(
		VertexPosNormCol(XMFLOAT3(-halfPlaneWidth, halfPlaneHeight, 0), XMFLOAT3(0, 0, -1), color),
		VertexPosNormCol(XMFLOAT3(halfPlaneWidth, halfPlaneHeight, 0), XMFLOAT3(0, 0, -1), color),
		VertexPosNormCol(XMFLOAT3(halfPlaneWidth, -halfPlaneHeight, 0), XMFLOAT3(0, 0, -1), color),
		VertexPosNormCol(XMFLOAT3(-halfPlaneWidth, -halfPlaneHeight, 0), XMFLOAT3(0, 0, -1), color)
	);

	AddComponent(m_Plane);
}

void Portal::Update(const SceneContext&)
{
	
}