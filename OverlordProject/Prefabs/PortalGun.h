//#pragma once
//class PortalMaterial;
//class Portal;
//enum class PortalType;
//class PortalScene;
//class Character;
//
//class PortalGun final : public GameObject
//{
//public:
//	PortalGun(Character* character);
//	~PortalGun() override = default;
//
//	PortalGun(const PortalGun& other) = delete;
//	PortalGun(PortalGun&& other) noexcept = delete;
//	PortalGun& operator=(const PortalGun& other) = delete;
//	PortalGun& operator=(PortalGun&& other) noexcept = delete;
//
//	void ShootGun(PortalScene* , PortalType);
//
//protected:
//	void Initialize(const SceneContext&) override;
//	void Update(const SceneContext&) override;
//
//private:
//	Character* m_pCharacter;
//
//	std::vector<Portal*> m_PortalPtrs{};
//	std::vector<PortalMaterial*> m_PortalMaterialPtrs{};
//};
