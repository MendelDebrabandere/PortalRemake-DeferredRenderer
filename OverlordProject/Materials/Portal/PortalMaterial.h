#pragma once
class PortalMaterial final : public Material<PortalMaterial>
{
public:
	PortalMaterial();
	~PortalMaterial() override = default;
	PortalMaterial(const PortalMaterial& other) = delete;
	PortalMaterial(PortalMaterial&& other) noexcept = delete;
	PortalMaterial& operator=(const PortalMaterial& other) = delete;
	PortalMaterial& operator=(PortalMaterial&& other) noexcept = delete;

	void SetWorldViewProj(const XMFLOAT4X4& wvp);

protected:
	void InitializeEffectVariables() override;

private:
	TextureData* m_pDiffuseTexture{};
};

