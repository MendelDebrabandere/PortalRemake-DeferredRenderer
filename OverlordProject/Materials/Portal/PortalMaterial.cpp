#include "stdafx.h"
#include "PortalMaterial.h"


PortalMaterial::PortalMaterial()
	: Material<PortalMaterial>(L"Effects/Portal/PostPortal.fx")
{
}

void PortalMaterial::SetWorldViewProj(const XMFLOAT4X4& wvp)
{
	SetVariable_Matrix(L"gWorldViewProj", wvp);
}

void PortalMaterial::InitializeEffectVariables()
{
}