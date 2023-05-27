#include "stdafx.h"
#include "Portal.h"

#include "Character.h"
#include "Materials/ColorMaterial.h"
#include "Materials/Portal/PortalMaterial.h"

Portal::Portal(PortalType type, Portal* pLinkedPortal, Character* character)
	: m_Type{ type }
	, m_pLinkedPortal{ pLinkedPortal }
	, m_pCharacter{ character }
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

	//Color material
	auto pColorMat = MaterialManager::Get()->CreateMaterial<ColorMaterial>();
	pColorMat->SetColor(m_Color);

	//Camera
	m_pCameraComponent = new CameraComponent();
	m_pCameraComponent->SetActive(false);

	m_pCameraObject = new GameObject();
	m_pCameraObject->AddComponent(m_pCameraComponent);
	m_pCameraObject->SetTag(L"PortalCam");

	auto modelGO = m_pCameraObject->AddChild(new GameObject());
	auto pModel = modelGO->AddComponent(new ModelComponent(L"Meshes/Arrow.ovm", false));
	modelGO->GetTransform()->Translate(0.0f, 0.0f, -7.f);
	pModel->SetMaterial(pColorMat);

	AddChild(m_pCameraObject);


	//Screen
	m_pScreenMat = MaterialManager::Get()->CreateMaterial<PortalMaterial>();
	m_pPortalModel = new ModelComponent(L"Meshes/portal.ovm");
	m_pPortalModel->SetMaterial(m_pScreenMat);
	AddComponent(m_pPortalModel);

	//Frame
	auto pFrameModel = new ModelComponent(L"Meshes/portalframe.ovm");
	pFrameModel->SetMaterial(pColorMat);
	AddComponent(pFrameModel);

}

void Portal::Update(const SceneContext& sceneContext)
{
	////SET POS TO CENTER OF OTHER PORTAL
	//auto otherPortalCenterWorld = m_pLinkedPortal->GetTransform()->GetWorldPosition();
	////otherPortalCenterWorld.x -= 0.1f;
	////otherPortalCenterWorld.y -= 0.1f;
	////otherPortalCenterWorld.z -= 0.1f;
	//auto parentTranslateWorld = GetTransform()->GetWorldPosition();

	//auto otherPortalLocal = XMFLOAT3{ otherPortalCenterWorld.x - parentTranslateWorld.x,
	//								otherPortalCenterWorld.y - parentTranslateWorld.y,
	//								otherPortalCenterWorld.z - parentTranslateWorld.z };

	//otherPortalLocal = XMFLOAT3{ otherPortalLocal.x * 10,
	//								otherPortalLocal.y * 10,
	//								otherPortalLocal.z * 10 + -5 };


	////auto playerTransform = m_pCharacter->GetTransform()->GetWorldPosition();

	//m_pCameraObject->GetTransform()->Translate(otherPortalLocal);
	//m_pCameraObject->GetTransform()->Rotate(0, 180, 0);


	auto portalT = GetTransform();
	auto linkedPortalT = m_pLinkedPortal->GetTransform();
	auto camT = sceneContext.pCamera->GetTransform();

	XMVECTOR relativeRotCam = XMLoadFloat4(&camT->GetWorldRotation());
	XMVECTOR relativeRotIn = XMLoadFloat4(&portalT->GetWorldRotation());
	XMVECTOR relativeRotOut = XMLoadFloat4(&linkedPortalT->GetWorldRotation());

	XMVECTOR relativePosCam = XMLoadFloat3(&camT->GetWorldPosition());
	XMVECTOR relativePosIn = XMLoadFloat3(&portalT->GetWorldPosition());
	XMVECTOR relativePosOut = XMLoadFloat3(&linkedPortalT->GetWorldPosition());



	//Rotate Camera according to main camera in portal space
	auto relativeRot = XMQuaternionInverse(relativeRotOut) * relativeRotCam;

	//Calculate rotation offset
	XMMATRIX yRotDiff{};
	if (static_cast<int>(std::roundf(m_pLinkedPortal->GetPortalDir())) % 180 == 0)
	{
		yRotDiff = XMMatrixRotationAxis(XMLoadFloat3(&portalT->GetUp()), XMConvertToRadians(180.f + m_pLinkedPortal->GetPortalDir()));
	}
	else
	{
		yRotDiff = XMMatrixRotationAxis(XMLoadFloat3(&portalT->GetUp()), XMConvertToRadians(m_pLinkedPortal->GetPortalDir()));
	}
	relativeRot = XMVector4Transform(relativeRot, yRotDiff);
	m_pCameraObject->GetTransform()->Rotate(relativeRotIn * relativeRot);

	//Translate Camera according to main camera in portal space
	XMVECTOR relativePos = (relativePosCam - relativePosOut);
	//Calculate rotation offset

	XMMATRIX yRotDiff2{};
	if (static_cast<int>(std::roundf(m_pLinkedPortal->GetPortalDir())) % 180 == 0)
	{
		yRotDiff2 = XMMatrixRotationAxis(XMLoadFloat3(&portalT->GetUp()), XMConvertToRadians(180.f + m_pLinkedPortal->GetPortalDir()));
	}
	else
	{
		yRotDiff2 = XMMatrixRotationAxis(XMLoadFloat3(&portalT->GetUp()), XMConvertToRadians(m_pLinkedPortal->GetPortalDir()));
	}

	relativePos = XMVector4Transform(relativePos, yRotDiff2);

	XMFLOAT3 pos{};
	XMStoreFloat3(&pos, relativePos);

	m_pCameraObject->GetTransform()->Translate(relativePos);
}

void Portal::SetNearClipPlane()
{
	const DirectX::XMFLOAT3& portalPos = GetTransform()->GetWorldPosition();
	DirectX::XMVECTOR xmPortalPos = DirectX::XMLoadFloat3(&portalPos);

	const DirectX::XMFLOAT3& portalNormal = GetTransform()->GetForward();
	DirectX::XMVECTOR xmPortalNormal = DirectX::XMLoadFloat3(&portalNormal);
	DirectX::XMVECTOR xmInvPortalNormal = DirectX::XMVectorScale(xmPortalNormal, -1); //rotate 180 degrees

	//Camera distance
	auto xmDot = DirectX::XMVector3Dot(xmPortalPos, xmInvPortalNormal);
	float camDist{};
	DirectX::XMStoreFloat(&camDist, xmDot);

	//Don't use oblique clip plane if very close to portal as it seems this can cause some visual artifacts
	float nearClipLimit = 0.3f;
	if (abs(camDist) > nearClipLimit)
	{
		//Create and set clipping plane vector
		DirectX::XMFLOAT4 clipPlane{ portalNormal.x, portalNormal.y, portalNormal.z, camDist };
		m_pCameraComponent->SetOblique(true);
		m_pCameraComponent->SetClipPlane(clipPlane);
	}
	else
	{
		m_pCameraComponent->SetOblique(false);
	}
}
