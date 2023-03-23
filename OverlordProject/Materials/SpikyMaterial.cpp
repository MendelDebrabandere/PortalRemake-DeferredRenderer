#include "stdafx.h"
#include "SpikyMaterial.h"

SpikyMaterial::SpikyMaterial()
	:Material<SpikyMaterial>(L"effects/SpikyShader.fx")
{}

void SpikyMaterial::InitializeEffectVariables()
{
}
