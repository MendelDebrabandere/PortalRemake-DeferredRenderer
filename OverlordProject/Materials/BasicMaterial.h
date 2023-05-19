#pragma once
class BasicMaterial : public Material<BasicMaterial>
{
public:
	BasicMaterial();
	~BasicMaterial() override = default;

	BasicMaterial(const BasicMaterial& other) = delete;
	BasicMaterial(BasicMaterial&& other) noexcept = delete;
	BasicMaterial& operator=(const BasicMaterial& other) = delete;
	BasicMaterial& operator=(BasicMaterial&& other) noexcept = delete;

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

