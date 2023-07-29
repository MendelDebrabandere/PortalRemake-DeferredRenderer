#pragma once
class BasicMaterial_Deferred : public Material<BasicMaterial_Deferred>
{
public:
	BasicMaterial_Deferred();
	~BasicMaterial_Deferred() override = default;

	BasicMaterial_Deferred(const BasicMaterial_Deferred& other) = delete;
	BasicMaterial_Deferred(BasicMaterial_Deferred&& other) noexcept = delete;
	BasicMaterial_Deferred& operator=(const BasicMaterial_Deferred& other) = delete;
	BasicMaterial_Deferred& operator=(BasicMaterial_Deferred&& other) noexcept = delete;

	void SetDiffuseMap(const std::wstring& assetFile);
	void SetDiffuseMap(TextureData* pTextureData);

	void SetNormalMap(const std::wstring& assetFile);
	void SetNormalMap(TextureData* pTextureData);

	void SetSpecularMap(const std::wstring& assetFile);
	void SetSpecularMap(TextureData* pTextureData);

	void UseTransparency(bool enable);

protected:
	void InitializeEffectVariables() override;
	void OnUpdateModelVariables(const SceneContext& /*sceneContext*/, const ModelComponent* /*pModel*/) const;
};

