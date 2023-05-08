#include "stdafx.h"
#include "PortalMaterial.h"


PortalMaterial::PortalMaterial()
	:Material<PortalMaterial>(L"effects/portalshader.fx")
{}


void PortalMaterial::MakeOrange()
{
	SetVariable_Vector(L"gColorDiffuse", XMFLOAT4{ 1.0f, 0.4f, 0.2f, 1.0f });
}

void PortalMaterial::MakeBlue()
{
	SetVariable_Vector(L"gColorDiffuse", XMFLOAT4{ 0.5f, 0.5f, 1.0f, 1.0f });
}

void PortalMaterial::InitializeEffectVariables()
{
	SetVariable_Vector(L"gLightDirection", XMFLOAT3{ 0.577f, 0.577f, -0.577f });
	//SetVariable_Vector(L"gColorDiffuse", XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f });
}
