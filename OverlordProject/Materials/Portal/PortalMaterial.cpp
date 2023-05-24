#include "stdafx.h"
#include "PortalMaterial.h"

PortalMaterial::PortalMaterial()
	:Material(L"effects/portal/tempportal.fx")
{}


void PortalMaterial::MakeOrange()
{
	SetVariable_Vector(L"gPortalColor", XMFLOAT4{ 1.0f, 0.4f, 0.2f, 1.0f });
}

void PortalMaterial::MakeBlue()
{
	SetVariable_Vector(L"gPortalColor", XMFLOAT4{ 0.2f, 0.2f, 0.8f, 1.0f });
}

void PortalMaterial::InitializeEffectVariables()
{
}
