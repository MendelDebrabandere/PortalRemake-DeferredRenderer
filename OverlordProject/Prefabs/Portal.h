#pragma once
class Character;
class PortalMaterial;

enum class PortalType
{
	Orange = 0,
	Blue = 1
};

class Portal final : public GameObject
{
public:
	Portal(PortalType type, PortalMaterial* material, Character* character);
	~Portal() override = default;

	Portal(const Portal& other) = delete;
	Portal(Portal&& other) noexcept = delete;
	Portal& operator=(const Portal& other) = delete;
	Portal& operator=(Portal&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	Character* m_pCharacter{};

	FixedCamera* m_pCameraObject{};
	CameraComponent* m_pCameraComponent{ nullptr };

	PortalType m_Type{};

	ModelComponent* m_pMesh{};

	PortalMaterial* m_pPortalMat{};
};
