#pragma once
class Portal;
enum class PortalType;
class PortalScene;

class PortalGun final : public GameObject
{
public:
	PortalGun() = default;
	~PortalGun() override = default;

	PortalGun(const PortalGun& other) = delete;
	PortalGun(PortalGun&& other) noexcept = delete;
	PortalGun& operator=(const PortalGun& other) = delete;
	PortalGun& operator=(PortalGun&& other) noexcept = delete;

	void ShootGun(PortalScene* , PortalType);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	std::vector<Portal*> m_pPortals{};

};
