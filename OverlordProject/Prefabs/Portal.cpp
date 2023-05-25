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

Portal::~Portal()
{
	SafeDelete(m_pRenderTarget);
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

	//Camera
	m_pCameraComponent = new CameraComponent();
	m_pCameraComponent->SetActive(false);

	m_pCameraObject = new FreeCamera();
	m_pCameraObject->AddComponent(m_pCameraComponent);
	m_pCameraObject->SetTag(L"PortalCam");

	auto pColorMat = MaterialManager::Get()->CreateMaterial<ColorMaterial>();
	pColorMat->SetColor(m_Color);
	auto pModel = m_pCameraObject->AddComponent(new ModelComponent(L"Meshes/Arrow.ovm", false));
	//pModel->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	pModel->SetMaterial(pColorMat);

	AddChild(m_pCameraObject);


	//Screen
	m_pScreenMat = MaterialManager::Get()->CreateMaterial<PortalMaterial>();
	m_pPortalModel = new ModelComponent(L"Meshes/portal.ovm");
	m_pPortalModel->SetMaterial(m_pScreenMat);
	AddComponent(m_pPortalModel);
	m_pPortalModel->GetTransform()->Scale(0.1f, 0.1f, 0.1f);

	//Frame
	auto pFrameModel = new ModelComponent(L"Meshes/portalframe.ovm");
	pFrameModel->SetMaterial(pColorMat);
	AddComponent(pFrameModel);
	pFrameModel->GetTransform()->Scale(0.1f, 0.1f, 0.1f);

}

void Portal::Update(const SceneContext& /*sceneContext*/)
{
	auto desiredPos = m_pLinkedPortal->GetTransform()->GetWorldPosition();
	desiredPos.x -= 0.1f;
	desiredPos.y -= 0.1f;
	desiredPos.z -= 0.1f;
	auto parentTranslate = GetTransform()->GetWorldPosition();

	auto localTranslate = XMFLOAT3{ desiredPos.x - parentTranslate.x,
									desiredPos.y - parentTranslate.y,
									desiredPos.z - parentTranslate.z };

	localTranslate = XMFLOAT3{ localTranslate.x * 10,
									localTranslate.y * 10,
									localTranslate.z * 10 };

	m_pCameraComponent->GetTransform()->Translate(localTranslate);
	m_pCameraComponent->GetTransform()->Rotate(0, 0, 0);
}

void Portal::SetNearClipPlane()
{
}
