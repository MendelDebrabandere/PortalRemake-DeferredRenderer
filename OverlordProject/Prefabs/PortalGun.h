#pragma once
class PortalMaterial;
class Portal;
enum class PortalType;
class PortalScene;
class Character;

class PortalGun final : public GameObject
{
public:
	PortalGun(PortalScene* scene, Character* character);
	~PortalGun() override = default;

	PortalGun(const PortalGun& other) = delete;
	PortalGun(PortalGun&& other) noexcept = delete;
	PortalGun& operator=(const PortalGun& other) = delete;
	PortalGun& operator=(PortalGun&& other) noexcept = delete;

	void ShootGun(PortalType);

	std::pair<Portal*, Portal*> GetPortals() const;

	//Only used at start for blue portal that is sitting on a black wall
	void SetBluePortalWall(PxShape* wall);
	void SetOrangePortalWall(PxShape* wall);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	Character* m_pCharacter;

	Portal* m_pBluePortal{};
	Portal* m_pOrangePortal{};

	PortalScene* m_pScene{};


	FMOD::Channel* m_pChannelOrangeGun{ nullptr };
	FMOD::Channel* m_pChannelBlueGun{ nullptr };
};
