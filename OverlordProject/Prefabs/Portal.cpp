#include "stdafx.h"
#include "Portal.h"

#include "Character.h"
#include "Materials/ColorMaterial.h"
#include "Materials/DiffuseMaterial.h"
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
		m_Color = XMFLOAT4{ 1,0.6f,0.2f,1 };
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

	m_pCameraPivot = AddChild(new GameObject());
	m_pCameraObject = m_pCameraPivot->AddChild(new GameObject());
	m_pCameraObject->AddComponent(m_pCameraComponent);
	m_pCameraObject->SetTag(L"PortalCam");

	////Camera arrow component for debugging
	//auto modelGO = m_pCameraObject->AddChild(new GameObject());
	//auto pModel = modelGO->AddComponent(new ModelComponent(L"Meshes/Arrow.ovm", false));
	//modelGO->GetTransform()->Translate(0.0f, 0.0f, -1.f);
	//pModel->SetMaterial(pColorMat);

	//Screen
	auto screenGO = AddChild(new GameObject());
	m_pScreenMat = MaterialManager::Get()->CreateMaterial<PortalMaterial>();
	m_pScreenModel = new ModelComponent(L"Meshes/portal.ovm");
	m_pScreenModel->SetMaterial(m_pScreenMat);
	screenGO->GetTransform()->Translate(0, 0, 0.f);
	screenGO->AddComponent(m_pScreenModel);

	//Frame
	auto pFrameModel = new ModelComponent(L"Meshes/portalframe.ovm");
	pFrameModel->SetMaterial(pColorMat);
	AddComponent(pFrameModel);

	//Overlap box
	auto& physx = PxGetPhysics();
	auto pDefaultMaterial = physx.createMaterial(0.5f, 0.5f, 0.5f);

	auto overlapRB = AddComponent(new RigidBodyComponent(true));
	overlapRB->AddCollider(PxBoxGeometry{ 0.7f, 0.7f, 1.f }, *pDefaultMaterial, true);
	overlapRB->GetPxRigidActor()->setName("PortalRB"); //important, because otherwise you can make new portals with gun on this RB

	auto function = [=](GameObject*, GameObject* pOtherObject, PxTriggerAction action)
	{
		if (pOtherObject == static_cast<GameObject*>(m_pCharacter) && m_pWall != nullptr)
		{
			if (action == PxTriggerAction::ENTER)
				m_CharacterEntered = true;
			if (action == PxTriggerAction::LEAVE)
				m_CharacterLeft = true;
		}
	};

	SetOnTriggerCallBack(function);

	//SECOND CUBE
	DiffuseMaterial* pCubeMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	//Set texture of the material
	pCubeMaterial->SetDiffuseTexture(L"textures/PortalCube.jpg");

	m_pCube = AddChild(new GameObject());
	m_pCube->SetTag(L"Cube");

	//Mesh
	const auto CubeMeshComponent = new ModelComponent(L"Meshes/PortalCube.ovm");
	CubeMeshComponent->SetMaterial(pCubeMaterial);
	m_pCube->AddComponent<ModelComponent>(CubeMeshComponent);

	//RigidBody
	const auto pConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/PortalCube.ovpc");
	const auto convexGeometry{ PxConvexMeshGeometry{ pConvexMesh } };
	auto cubeRB = m_pCube->AddComponent(new RigidBodyComponent(true));
	cubeRB->AddCollider(convexGeometry, *pDefaultMaterial);
	cubeRB->GetPxRigidActor()->setName("CubeRB");
}

void Portal::Update(const SceneContext& sceneContext)
{
	if (m_pLinkedPortal == nullptr)
		return;

	DoCameraRotations(sceneContext);

	DoCollisionLogic(sceneContext);

	DoTeleportingLogic(sceneContext);

	DoCubeLogic(sceneContext);
}

void Portal::DoCameraRotations(const SceneContext& sceneContext)
{
	// Get transforms
	auto otherPortalTransform = m_pLinkedPortal->GetTransform();
	auto playerCamTransform = sceneContext.pCamera->GetTransform();

	// Get the world positions and rotations as XMVECTOR's
	XMVECTOR otherPortalPos = XMLoadFloat3(&otherPortalTransform->GetWorldPosition());
	XMVECTOR playerCamPos = XMLoadFloat3(&playerCamTransform->GetWorldPosition());

	XMVECTOR otherPortalRot = XMLoadFloat4(&otherPortalTransform->GetWorldRotation());
	XMVECTOR playerCamRot = XMLoadFloat4(&playerCamTransform->GetWorldRotation());

	//TRANSLATIONS
	// Compute position of player relative to portal A
	XMVECTOR relativePos = playerCamPos - otherPortalPos;

	float distance;
	XMStoreFloat(&distance, XMVector3Length(relativePos));
	if (distance > 0.2f)
	{
		//convert to XMFLOAT
		XMFLOAT3 relativePosition;
		XMStoreFloat3(&relativePosition, relativePos);
		m_pCameraObject->GetTransform()->Translate(relativePosition);
	}


	//ROTATIONS
	// ROTATE OBJECT TO THE PLAYER CAMERA 
	// Convert quaternion to Euler angles (roll, pitch, yaw order)
	XMFLOAT4 q;
	XMStoreFloat4(&q, playerCamRot);
	float pitch = asin(2.f * (q.w * q.y - q.z * q.x));
	float roll = atan2(2.f * (q.w * q.x + q.y * q.z), 1 - 2.f * (q.x * q.x + q.y * q.y));
	float yaw = atan2(2.f * (q.w * q.z + q.x * q.y), 1 - 2.f * (q.y * q.y + q.z * q.z));

	if (yaw > XM_PI / 2.f || yaw < -XM_PI / 2.f)
	{
		roll += XM_PI;
		yaw += XM_PI;
		pitch = XM_PI - pitch;
	}

	m_pCameraObject->GetTransform()->Rotate(roll, pitch, yaw, false); // These values are radians so passing false

	// ROTATE PIVOT TO THE OTHER PORTAL
	XMStoreFloat4(&q, otherPortalRot);
	pitch = asin(2.f * (q.w * q.y - q.z * q.x));
	roll = atan2(2.f * (q.w * q.x + q.y * q.z), 1 - 2.f * (q.x * q.x + q.y * q.y));
	yaw = atan2(2.f * (q.w * q.z + q.x * q.y), 1 - 2.f * (q.y * q.y + q.z * q.z));

	if (yaw > XM_PI / 2.f || yaw < -XM_PI / 2.f)
	{
		roll += XM_PI;
		yaw += XM_PI;
		pitch = XM_PI - pitch;
	}

	m_pCameraPivot->GetTransform()->Rotate(-roll, XM_PI - pitch, -yaw, false); // These values are radians so passing false
}

void Portal::DoCollisionLogic(const SceneContext&)
{
	if (m_CharacterEntered)
	{
		m_pWall->setLocalPose(PxTransform(PxVec3{ -1000,-1000,-1000 }));
		m_CharacterEntered = false;
	}
	if (m_CharacterLeft)
	{
		m_pWall->setLocalPose(m_WallPos);
		m_CharacterLeft = false;
	}
}

void Portal::UpateNearClipPlane()
{
	if (m_pLinkedPortal == nullptr)
		return;

	const XMFLOAT3& portalPos = GetTransform()->GetWorldPosition();
	XMVECTOR xmPortalPos = XMLoadFloat3(&portalPos);

	const XMFLOAT3& portalNormal = GetTransform()->GetForward();
	XMVECTOR xmPortalNormal = XMLoadFloat3(&portalNormal);

	XMFLOAT3 inversPortalNormal{ -portalNormal.x ,-portalNormal.y ,-portalNormal.z };

	//Camera distance
	auto xmDot = XMVector3Dot(xmPortalPos, xmPortalNormal);
	float camDist{};
	XMStoreFloat(&camDist, xmDot);

	//Create and set clipping plane vector
	XMFLOAT4 clipPlane{ inversPortalNormal.x, inversPortalNormal.y, inversPortalNormal.z, camDist };
	m_pCameraComponent->SetOblique(true);
	m_pCameraComponent->SetClipPlane(clipPlane);
}

void Portal::SetWall(PxShape* wall)
{
	m_pWall = wall;
	m_WallPos = m_pWall->getLocalPose();
}

void Portal::DoTeleportingLogic(const SceneContext&)
{
	if (m_pCharacter->GetTpCooldown() >= 0.f)
		return;

	//Get Player data
	const XMFLOAT3& characterPos = m_pCharacter->GetTransform()->GetWorldPosition();
	XMVECTOR xmCharacterPos = XMLoadFloat3(&characterPos);

	//Get this data
	const XMFLOAT3& thisPortalPos = GetTransform()->GetPosition();
	XMVECTOR xmThisPortalPos = XMLoadFloat3(&thisPortalPos);
	const XMFLOAT3& thisPortalForward = GetTransform()->GetForward();
	XMVECTOR xmThisPortalForward = XMLoadFloat3(&thisPortalForward);
	const XMFLOAT4& thisPortalRot = GetTransform()->GetWorldRotation();
	XMVECTOR xmThisPortalRot = XMLoadFloat4(&thisPortalRot);

	//Get this data
	const XMFLOAT3& otherPortalPos = m_pLinkedPortal->GetTransform()->GetPosition();
	XMVECTOR xmOtherPortalPos = XMLoadFloat3(&otherPortalPos);
	const XMFLOAT3& otherPortalForward = m_pLinkedPortal->GetTransform()->GetForward();
	const XMFLOAT4& otherPortalRot = m_pLinkedPortal->GetTransform()->GetWorldRotation();
	XMVECTOR xmOtherPortalRot = XMLoadFloat4(&otherPortalRot);

	//Get player pos in portal space
	auto playerRelativePos = xmCharacterPos - xmThisPortalPos;

	//If this dotproduct is very small that means the player is on the plane of the portal.
	XMVECTOR dotProduct = XMVector3Dot(playerRelativePos, xmThisPortalForward);
	float dotProductValue = XMVectorGetX(dotProduct);

	if (abs(dotProductValue) <= 0.4f)
	{
		// Calculate the length (3D distance from player and portal)
		XMVECTOR length = XMVector3Length(playerRelativePos);
		float lengthValue = XMVectorGetX(length);
		if (lengthValue <= 3.f)
		{
			//TP to other portal
			m_pCharacter->SetTpCooldown(0.01f);

			//Calculate player relative pos to this portal
			XMVECTOR xmThisRelativePos = xmCharacterPos - xmThisPortalPos;
			xmThisRelativePos = XMVectorSetZ(xmThisRelativePos, 0.1f);
			XMFLOAT3 float3;
			XMStoreFloat3(&float3, xmThisRelativePos);
			float3.z = 0;
			xmThisRelativePos = XMLoadFloat3(&float3);
			XMVECTOR xmTeleportationPos = xmOtherPortalPos + xmThisRelativePos;

			XMFLOAT3 teleportationPos;
			XMStoreFloat3(&teleportationPos, xmTeleportationPos);
			m_pCharacter->GetTransform()->Translate(teleportationPos);

			//Calculate the rotation difference between the portals
			float thisPortalYaw = XM_PI + atan2f(thisPortalForward.z, thisPortalForward.x); // PI + because we need to take the opposite angle of the entrance (the back vector)
			float otherPortalYaw = atan2f(otherPortalForward.z, otherPortalForward.x);

			float portalYawDiff = otherPortalYaw - thisPortalYaw;

			m_pCharacter->AddCameraRotation(-portalYawDiff * 180.f / XM_PI, 0); // negative because in playercamere positive means rotation to the right, not left

			// i dont bother with the Pitch because that would look weird
			// the real game handles it really well but i wont bother

			//Rotate velocity
			XMFLOAT3 velocity = m_pCharacter->GetVelocity();
			XMVECTOR xmVelocity = XMLoadFloat3(&velocity);

			// Calculate the quaternion that brings quaternion1 to quaternion2
			XMVECTOR rotationQuaternion = XMQuaternionMultiply(XMQuaternionConjugate(xmThisPortalRot), xmOtherPortalRot);

			// Apply the rotation to the other vector
			XMVECTOR xmRotatedVector = XMVector3Rotate(xmVelocity, rotationQuaternion);

			XMFLOAT3 rotatedVector;
			XMStoreFloat3(&rotatedVector, xmRotatedVector);

			rotatedVector = XMFLOAT3{ -rotatedVector.x,
										rotatedVector.y,
										-rotatedVector.z };



			m_pCharacter->SetVelocity(rotatedVector);

		}
	}
}

void Portal::DoCubeLogic(const SceneContext&)
{
	auto playerCube = m_pCharacter->GetCube();
	if (playerCube == nullptr)
	{
		m_pCube->GetTransform()->Translate(-1000, -1000, -1000);
		return;
	}

	auto cubeModel = m_pCube->GetComponent<ModelComponent>();
	if (cubeModel == nullptr)
		return;

	//Get Player cube data
	const XMFLOAT3& cubePos = playerCube->GetTransform()->GetWorldPosition();
	XMVECTOR xmCubePos = XMLoadFloat3(&cubePos);

	//Get this data
	const XMFLOAT3& otherPortalPos = m_pLinkedPortal->GetTransform()->GetPosition();
	XMVECTOR xmOtherPortalPos = XMLoadFloat3(&otherPortalPos);

	//Get cube pos in otherportal space
	auto cubeRelativePos = xmCubePos - xmOtherPortalPos;

	// Calculate the length (3D distance from player and portal)
	XMVECTOR length = XMVector3Length(cubeRelativePos);
	float lengthValue = XMVectorGetX(length);

	//if the box is in range of portal
	if (lengthValue <= 5.8f)
	{
		//TRANSLATIONS
		// Compute position of player relative to portal A
		XMVECTOR relativePos = xmCubePos - xmOtherPortalPos;

		//Rotate with local position if the other portal
		const XMFLOAT4& rotQuat = m_pLinkedPortal->GetTransform()->GetRotation();
		auto xmRotQuat = XMLoadFloat4(&rotQuat);
		XMVECTOR q_conjugate = XMQuaternionConjugate(xmRotQuat);
		XMVECTOR rotatedVec = XMQuaternionMultiply(XMQuaternionMultiply(xmRotQuat, relativePos), q_conjugate);

		//convert to XMFLOAT
		XMFLOAT3 relativePosition;
		XMStoreFloat3(&relativePosition, rotatedVec);
		//Invert Z and X
		relativePosition.x = -relativePosition.x;
		relativePosition.z = -relativePosition.z;
		m_pCube->GetTransform()->Translate(relativePosition);


		//ROTATIONS
		const XMFLOAT4& cubeRot = playerCube->GetTransform()->GetRotation();
		XMVECTOR xmCubeRot = XMLoadFloat4(&cubeRot);

		//This portal Rot
		auto thisRotQuat = GetTransform()->GetRotation();
		auto xmThisRotQuat = XMLoadFloat4(&thisRotQuat);

		// Calculate the quaternion that brings quaternion1 to quaternion2
		XMVECTOR rotationQuaternion = XMQuaternionMultiply(xmThisRotQuat, xmRotQuat);

		// Apply the rotation to the other vector
		XMVECTOR xmRotatedQuat = XMQuaternionMultiply(rotationQuaternion, xmCubeRot);

		XMFLOAT4 rotatedQuat;
		XMStoreFloat4(&rotatedQuat, xmRotatedQuat);

		float yaw, pitch, roll;

		// roll (x-axis rotation)
		float sinr_cosp = +2.0f * (rotatedQuat.w * rotatedQuat.x + rotatedQuat.y * rotatedQuat.z);
		float cosr_cosp = +1.0f - 2.0f * (rotatedQuat.x * rotatedQuat.x + rotatedQuat.y * rotatedQuat.y);
		roll = atan2f(sinr_cosp, cosr_cosp);

		// pitch (y-axis rotation)
		float sinp = +2.0f * (rotatedQuat.w * rotatedQuat.y - rotatedQuat.z * rotatedQuat.x);
		if (fabs(sinp) >= 1)
			pitch = copysign(XM_PI / 2, sinp); // use 90 degrees if out of range
		else
			pitch = asinf(sinp);

		// yaw (z-axis rotation)
		float siny_cosp = +2.0f * (rotatedQuat.w * rotatedQuat.z + rotatedQuat.x * rotatedQuat.y);
		float cosy_cosp = +1.0f - 2.0f * (rotatedQuat.y * rotatedQuat.y + rotatedQuat.z * rotatedQuat.z);
		yaw = atan2f(siny_cosp, cosy_cosp);
		

		m_pCube->GetTransform()->Rotate(pitch, yaw, roll, false); // These values are radians so passing false
	}
	else
		m_pCube->GetTransform()->Translate(-1000, -1000, -1000);
}
