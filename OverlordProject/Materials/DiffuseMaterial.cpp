#include "stdafx.h"
#include "DiffuseMaterial.h"

DiffuseMaterial::DiffuseMaterial()
	:Material<DiffuseMaterial>(L"effects/PosNormTex3d.fx")
{}

void DiffuseMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	if (m_pDiffuseTexture) delete m_pDiffuseTexture;
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);

	SetVariable_Texture(L"gDiffuseMap", m_pDiffuseTexture);
}

void DiffuseMaterial::InitializeEffectVariables()
{
		//const auto effectDiffuse = m_pEffect->GetVariableByName("gDiffuseMap");
}
