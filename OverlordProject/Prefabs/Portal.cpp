#include "stdafx.h"
#include "Portal.h"

#include "Materials/ColorMaterial.h"
#include "Materials/Portal/PortalMaterial.h"

Portal::Portal(PortalType type, Portal* pLinkedPortal)
	: m_Type{ type }
	, m_pLinkedPortal{ pLinkedPortal }
{
	if (type == PortalType::Blue)
	{
		m_Color = XMFLOAT4{ 0,0,1,1 };
	}
	else if (type == PortalType::Orange)
	{
		m_Color = XMFLOAT4{ 1,0.4f,0,1 };
	}
}

void Portal::Initialize(const SceneContext& sceneContext)
{
	//Render target
	m_pRenderTarget = new RenderTarget(sceneContext.d3dContext);
	RENDERTARGET_DESC rDesc{};
	rDesc.enableColorSRV = true;
	rDesc.enableColorBuffer = true;
	rDesc.enableDepthSRV = true;
	rDesc.width = static_cast<UINT>(sceneContext.windowWidth);
	rDesc.height = static_cast<UINT>(sceneContext.windowHeight);

	m_pRenderTarget->Create(rDesc);

	//Cameras
	m_pCameraComponent = new CameraComponent();
	m_pCameraComponent->SetActive(false);

	m_pCameraObject = new FreeCamera();
	m_pCameraObject->AddComponent(m_pCameraComponent);
	m_pCameraObject->SetTag(L"PortalCam");

	auto pCamMat = MaterialManager::Get()->CreateMaterial<ColorMaterial>();
	pCamMat->SetColor(m_Color);
	auto pModel = m_pCameraObject->AddComponent(new ModelComponent(L"Meshes/Sphere.ovm", false));
	pModel->SetMaterial(pCamMat);

	AddChild(m_pCameraObject);


	//Screen
	auto pScreenObject = AddChild(new GameObject());
	m_pScreenMat = MaterialManager::Get()->CreateMaterial<PortalMaterial>();
	m_pPortalModel = new ModelComponent(L"Meshes/portal.ovm");
	m_pPortalModel->SetMaterial(m_pScreenMat);
	pScreenObject->AddComponent(m_pPortalModel);

	m_pPortalModel->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	m_pPortalModel->GetTransform()->Rotate(0.f, 90.f, 0.f);
}

void Portal::Update(const SceneContext& /*sceneContext*/)
{
	auto position = m_pLinkedPortal->GetTransform()->GetWorldPosition();
	position.x -= 0.1f;
	position.y -= 0.1f;
	position.z -= 0.1f;
	m_pCameraComponent->GetTransform()->Translate(position);
	m_pCameraComponent->GetTransform()->Rotate(90, 0, 0);
}

void Portal::SetNearClipPlane()
{
}
