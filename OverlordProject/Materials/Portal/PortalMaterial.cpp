#include "stdafx.h"
#include "PortalMaterial.h"

#include "Graphics/PortalMapRenderer.h"
#include "Prefabs/Portal.h"


PortalMaterial::PortalMaterial()
	:Material(L"effects/portal/portalshader.fx")
{}


void PortalMaterial::MakeOrange()
{
	SetVariable_Vector(L"gColorDiffuse", XMFLOAT4{ 1.0f, 0.4f, 0.2f, 1.0f });
}

void PortalMaterial::MakeBlue()
{
	SetVariable_Vector(L"gColorDiffuse", XMFLOAT4{ 0.5f, 0.5f, 1.0f, 1.0f });
}

void PortalMaterial::SetOtherPortal(Portal* otherPortal)
{
	SetVariable_Matrix(L"gOtherPortalWorldViewProj", otherPortal->GetWorldViewProj());
	SetVariable_Matrix(L"gOtherPortalWorld", otherPortal->GetWorld());
}

void PortalMaterial::InitializeEffectVariables()
{
}


void PortalMaterial::OnUpdateModelVariables(const SceneContext& sceneContext, const ModelComponent* pModel) const
{
	/*
	 * TODO_W8
	 * Update The Shader Variables
	 * 1. Update the LightWVP > Used to Transform a vertex into Light clipping space
	 * 	LightWVP = model_world * light_viewprojection
	 * 	(light_viewprojection [LightVP] can be acquired from the ShadowMapRenderer)
	 *
	 * 2. Update the ShadowMap texture
	 *
	 * 3. Update the Light Direction (retrieve the direction from the LightManager > sceneContext)
	*/

	//Update Shadow Variables
	//const auto pShadowMapRenderer = ShadowMapRenderer::Get();
	//...

	// 1. Update the LightWVP > Used to Transform a vertex into Light clipping space
	// LightWVP = model_world * light_viewprojection
	// (light_viewprojection [LightVP] can be acquired from the ShadowMapRenderer)
	const auto pPortalMapRenderer = PortalMapRenderer::Get();
	auto modelWorldMatrix = pModel->GetTransform()->GetWorld();
	auto lightViewProjectionMatrix = pPortalMapRenderer->GetLightVP();
	XMFLOAT4X4 lightWVP;
	XMStoreFloat4x4(&lightWVP, XMMatrixMultiply(XMLoadFloat4x4(&modelWorldMatrix), XMLoadFloat4x4(&lightViewProjectionMatrix)));
	SetVariable_Matrix(L"gWorldViewProj_Light", lightWVP);

	// 2. Update the ShadowMap texture
	auto portalMapTexture = pPortalMapRenderer->GetShadowMap();
	SetVariable_Texture(L"gPortalMap", portalMapTexture);

	// 3. Update the Light Direction (retrieve the direction from the LightManager > sceneContext)
	auto lightDirection = sceneContext.pLights->GetDirectionalLight().direction;
	SetVariable_Vector(L"gLightDirection", lightDirection);
}
