#pragma once
class PortalGun;
class Portal;
//class PortalGun;
class Character;

class PortalScene final : public GameScene
{
public:
	PortalScene() : GameScene(L"PortalScene") {};
	~PortalScene() override = default;

	PortalScene(const PortalScene& other) = delete;
	PortalScene(PortalScene&& other) noexcept = delete;
	PortalScene& operator=(const PortalScene& other) = delete;
	PortalScene& operator=(PortalScene&& other) noexcept = delete;

	BaseMaterial* LoadMaterial(const std::wstring& name = L"", const XMFLOAT4& color = { 0,0,0,1 }, bool useTransparency = false);

protected:
	void Initialize() override;
	void Update() override;
	void PostDraw() override;
	void OnGUI() override;
	void OnSceneActivated() override;

private:
	// Helper functions
	void InitLevel();
	void InitCharacter(bool controlCamera, float mouseSens);

	GameObject* m_pFloor{ nullptr };

	GameObject* m_pWall1{ nullptr };
	GameObject* m_pWall2{ nullptr };

	Character* m_pCharacter{};

	PortalGun* m_pPortalGun{};

	Portal* m_pBluePortal{};
	Portal* m_pOrangePortal{};

	GameObject* m_pSprite{};

	bool m_ButtonIsTriggered{};

	FMOD::Channel* m_pChannelAudio{ nullptr };
};
