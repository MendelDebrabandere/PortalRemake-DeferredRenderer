#pragma once
class PortalMapMaterial final : public Material<PortalMapMaterial>
{
public:
	PortalMapMaterial();
	~PortalMapMaterial() override = default;
	PortalMapMaterial(const PortalMapMaterial& other) = delete;
	PortalMapMaterial(PortalMapMaterial&& other) noexcept = delete;
	PortalMapMaterial& operator=(const PortalMapMaterial& other) = delete;
	PortalMapMaterial& operator=(PortalMapMaterial&& other) noexcept = delete;

protected:
	void InitializeEffectVariables() override;
};

