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
	modelGO->GetTransform()->Translate(0.0f, 0.0f, -1.f);
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
	// Get transforms
	auto otherPortalTransform = m_pLinkedPortal->GetTransform();
	auto playerCamTransform = sceneContext.pCamera->GetTransform();

	// Get the world positions and rotations as XMVECTOR's
	XMVECTOR portalPos = XMLoadFloat3(&otherPortalTransform->GetWorldPosition());
	XMVECTOR camPos = XMLoadFloat3(&playerCamTransform->GetWorldPosition());

	XMVECTOR camRot = XMLoadFloat4(&playerCamTransform->GetWorldRotation());

	// Compute position of player relative to portal A
	XMVECTOR relativePos = camPos - portalPos;

	XMFLOAT3 pos{};
	XMStoreFloat3(&pos, relativePos);

	// Convert quaternion to Euler angles (roll, pitch, yaw order)
	XMFLOAT4 q;
	XMStoreFloat4(&q, camRot);
	float pitch = asin(2.f * (q.w * q.y - q.z * q.x));
	float roll = atan2(2.f * (q.w * q.x + q.y * q.z), 1 - 2.f * (q.x * q.x + q.y * q.y));
	float yaw = atan2(2.f * (q.w * q.z + q.x * q.y), 1 - 2.f * (q.y * q.y + q.z * q.z));

	XMFLOAT3 eulerRotation{ roll, pitch, yaw };

	// Set the relative position and rotation of Portal B Camera
	m_pCameraObject->GetTransform()->Rotate(eulerRotation.x, eulerRotation.y, eulerRotation.z, false); // These values are radians so passing true
	m_pCameraObject->GetTransform()->Translate(pos);

}

void Portal::SetNearClipPlane()
{
	const XMFLOAT3& portalPos = GetTransform()->GetWorldPosition();
	XMVECTOR xmPortalPos = XMLoadFloat3(&portalPos);

	const XMFLOAT3& portalNormal = GetTransform()->GetForward();
	XMVECTOR xmPortalNormal = XMLoadFloat3(&portalNormal);
	XMVECTOR xmInvPortalNormal = XMVectorScale(xmPortalNormal, -1); //rotate 180 degrees

	//Camera distance
	auto xmDot = XMVector3Dot(xmPortalPos, xmInvPortalNormal);
	float camDist{};
	XMStoreFloat(&camDist, xmDot);

	//Don't use oblique clip plane if very close to portal as it seems this can cause some visual artifacts
	float nearClipLimit = 0.3f;
	if (abs(camDist) > nearClipLimit)
	{
		//Create and set clipping plane vector
		XMFLOAT4 clipPlane{ portalNormal.x, portalNormal.y, portalNormal.z, camDist };
		m_pCameraComponent->SetOblique(true);
		m_pCameraComponent->SetClipPlane(clipPlane);
	}
	else
	{
		m_pCameraComponent->SetOblique(false);
	}
}
