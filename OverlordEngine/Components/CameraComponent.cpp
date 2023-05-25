#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent() :
	m_FarPlane(2500.0f),
	m_NearPlane(0.1f),
	m_FOV(XM_PIDIV4),
	m_Size(25.0f),
	m_PerspectiveProjection(true)
{
	XMStoreFloat4x4(&m_Projection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, XMMatrixIdentity());
}

void CameraComponent::Update(const SceneContext& sceneContext)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	XMMATRIX projection{};

	if (m_PerspectiveProjection)
	{
		projection = XMMatrixPerspectiveFovLH(m_FOV, sceneContext.aspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		const float viewWidth = (m_Size > 0) ? m_Size * sceneContext.aspectRatio : sceneContext.windowWidth;
		const float viewHeight = (m_Size > 0) ? m_Size : sceneContext.windowHeight;
		projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	const XMVECTOR worldPosition = XMLoadFloat3(&GetTransform()->GetWorldPosition());
	const XMVECTOR lookAt = XMLoadFloat3(&GetTransform()->GetForward());
	const XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

	const XMMATRIX view = XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
	const XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
	const XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

	if (m_UseOblique)
	{
		//Get clip plane
		auto clipPlane = m_ClippingPlane;
		auto xmClipPlane = DirectX::XMLoadFloat4(&clipPlane);

		//Transform our clip plane with the inverse transpose of the view 
		auto xmViewInvTransposed = DirectX::XMMatrixTranspose(viewInv);
		xmClipPlane = DirectX::XMPlaneTransform(xmClipPlane, xmViewInvTransposed);
		DirectX::XMStoreFloat4(&clipPlane, xmClipPlane);

		//Offset for near plane floating point errors
		clipPlane.w += (0.15f * clipPlane.w);
		xmClipPlane = DirectX::XMLoadFloat4(&clipPlane);

		//Now we calculate the clip-space corner point opposite of the clipping plane
		DirectX::XMFLOAT4 q{};
		q.x = MathHelper::GetSign(clipPlane.x);
		q.y = MathHelper::GetSign(clipPlane.y);
		q.z = 1.f;
		q.w = 1.f;

		//Load in projection matrix
		DirectX::XMFLOAT4X4 matrix{};
		DirectX::XMStoreFloat4x4(&matrix, projection);

		//Transform Q into camera space (can be achieved through projection matrix)
		q.x = q.x / matrix._11;
		q.y = q.y / matrix._22;
		q.z = 1.0f;
		q.w = (1.0f - matrix._33) / matrix._43;

		auto xmQ = DirectX::XMLoadFloat4(&q);
		auto xmDot = DirectX::XMVector4Dot(xmClipPlane, xmQ);
		float dot{};
		DirectX::XMStoreFloat(&dot, xmDot);
		float a = 1.f / dot;

		auto xmM3 = DirectX::XMVectorScale(xmClipPlane, a);
		DirectX::XMFLOAT4 m3{};
		DirectX::XMStoreFloat4(&m3, xmM3);

		//Final matrix composition, replace the third column
		matrix._13 = m3.x;
		matrix._23 = m3.y;
		matrix._33 = m3.z;
		matrix._43 = m3.w;

		//Projection matrix done, load back into projection
		projection = DirectX::XMLoadFloat4x4(&matrix);
	}

	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
}

void CameraComponent::SetActive(bool active)
{
	if (m_IsActive == active) return;

	const auto pGameObject = GetGameObject();
	ASSERT_IF(!pGameObject, L"Failed to set active camera. Parent game object is null");

	if (!pGameObject) return; //help the compiler... (C6011)
	const auto pScene = pGameObject->GetScene();
	ASSERT_IF(!pScene, L"Failed to set active camera. Parent game scene is null");

	m_IsActive = active;
	pScene->SetActiveCamera(active?this:nullptr); //Switch to default camera if active==false
}

GameObject* CameraComponent::Pick(CollisionGroup ignoreGroups) const
{
	//TODO_W7(L"Implement Picking Logic")
	const auto& pScene = GetGameObject()->GetScene();

	const POINT& mousePos = InputManager::GetMousePosition();
	const float halfWidth{ pScene->GetSceneContext().windowWidth / 2.0f };
	const float halfHeight{ pScene->GetSceneContext().windowHeight / 2.0f };

	const float Xndc = (mousePos.x - halfWidth) / halfWidth;
	const float Yndc = (halfHeight - mousePos.y) / halfHeight;

	const auto& vpInverse{ XMLoadFloat4x4(&GetViewProjectionInverse()) };
	const XMVECTOR nearPoint = XMVector3TransformCoord(XMVECTOR{ Xndc, Yndc, 0, 0 }, vpInverse);
	const XMVECTOR farPoint = XMVector3TransformCoord(XMVECTOR{ Xndc, Yndc, 1, 0 }, vpInverse);

	XMFLOAT3 rayStartF;
	XMStoreFloat3(&rayStartF, nearPoint);
	XMFLOAT3 rayEndF;
	XMStoreFloat3(&rayEndF, farPoint);

	const PxVec3 rayStart{ rayStartF.x, rayStartF.y, rayStartF.z };
	const PxVec3 rayEnd{ rayEndF.x - rayStart.x, rayEndF.y - rayStart.y, rayEndF.z - rayStart.z };

	PxQueryFilterData filterData{};
	filterData.data.word0 = ~UINT(ignoreGroups);

	PxRaycastBuffer hit{};
	if (pScene->GetPhysxProxy()->Raycast(rayStart, rayEnd.getNormalized(), PX_MAX_F32, hit, PxHitFlag::eDEFAULT, filterData))
	{
		BaseComponent* pComponent{ static_cast<BaseComponent*>(hit.block.actor->userData) };
		return pComponent->GetGameObject();
	}

	return nullptr;
}