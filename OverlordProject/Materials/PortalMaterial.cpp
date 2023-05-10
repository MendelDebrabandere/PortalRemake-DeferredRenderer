#include "stdafx.h"
#include "PortalMaterial.h"
#include "Prefabs/Portal.h"


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

void PortalMaterial::SetOtherPortal(Portal* otherPortal)
{
	SetVariable_Matrix(L"gOtherPortalWorldViewProj", otherPortal->GetWorldViewProj());
	SetVariable_Matrix(L"gOtherPortalWorld", otherPortal->GetWorld());
}

void PortalMaterial::InitializeEffectVariables()
{
}
