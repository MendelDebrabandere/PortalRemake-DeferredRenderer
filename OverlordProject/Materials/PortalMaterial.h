#pragma once

class Portal;

class PortalMaterial final : public Material<PortalMaterial>
{
public:
	PortalMaterial();
	~PortalMaterial() override = default;

	PortalMaterial(const PortalMaterial& other) = delete;
	PortalMaterial(PortalMaterial&& other) noexcept = delete;
	PortalMaterial& operator=(const PortalMaterial& other) = delete;
	PortalMaterial& operator=(PortalMaterial&& other) noexcept = delete;

	void MakeOrange();
	void MakeBlue();

	void SetOtherPortal(Portal* otherPortal);

protected:
	void InitializeEffectVariables() override;

private:
};
