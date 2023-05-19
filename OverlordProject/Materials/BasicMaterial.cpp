#include "stdafx.h"
#include "BasicMaterial.h"

BasicMaterial::BasicMaterial():
	Material(L"Effects/BasicEffect.fx")
{
}

void BasicMaterial::SetDiffuseMap(const std::wstring& assetFile)
{
	SetDiffuseMap(ContentManager::Load<TextureData>(assetFile));
}

void BasicMaterial::SetDiffuseMap(TextureData* pTextureData)
{
	SetVariable_Scalar(L"gUseDiffuseMap", pTextureData != nullptr);
	SetVariable_Texture(L"gDiffuseMap", pTextureData);
}

void BasicMaterial::SetNormalMap(const std::wstring& assetFile)
{
	SetNormalMap(ContentManager::Load<TextureData>(assetFile));

}

void BasicMaterial::SetNormalMap(TextureData* pTextureData)
{
	SetVariable_Scalar(L"gUseNormalMap", pTextureData != nullptr);
	SetVariable_Texture(L"gNormalMap", pTextureData);
}

void BasicMaterial::SetSpecularMap(const std::wstring& assetFile)
{
	SetSpecularMap(ContentManager::Load<TextureData>(assetFile));

}

void BasicMaterial::SetSpecularMap(TextureData* pTextureData)
{
	SetVariable_Scalar(L"gUseSpecularMap", pTextureData != nullptr);
	SetVariable_Texture(L"gSpecularMap", pTextureData);
}

void BasicMaterial::UseTransparency(bool enable)
{
	SetTechnique(enable ? L"Default_Alpha" : L"Default");
}

void BasicMaterial::InitializeEffectVariables()
{
}

void BasicMaterial::OnUpdateModelVariables(const SceneContext& sceneContext, const ModelComponent*) const
{
	SetVariable_Vector(L"gLightDirection", sceneContext.pLights->GetDirectionalLight().direction);
}
