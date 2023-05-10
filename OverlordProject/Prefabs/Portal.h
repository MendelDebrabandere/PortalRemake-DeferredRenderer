#pragma once
class PortalMaterial;

enum class PortalType
{
	Orange = 0,
	Blue = 1
};

class Portal final : public GameObject
{
public:
	Portal(PortalType type, PortalMaterial* material, Portal* otherPortal);
	~Portal() override = default;

	Portal(const Portal& other) = delete;
	Portal(Portal&& other) noexcept = delete;
	Portal& operator=(const Portal& other) = delete;
	Portal& operator=(Portal&& other) noexcept = delete;

	const float* GetWorldViewProj() const;
	const float* GetWorld() const;

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	PortalType m_Type{};

	ModelComponent* m_pMesh{};

	PortalMaterial* m_pPortalMat{};

	Portal* m_OtherPortal{};

	float* m_pWorldVar{};
	float* m_pWorldViewProjVar{};
};
