#pragma once
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

protected:
	void Initialize() override;
	void Update() override;
	//void Draw() override;
	void PostDraw() override;
	void OnGUI() override;

private:
	// Helper functions
	void InitLevel();
	void InitCharacter(bool controlCamera, float mouseSens);

	GameObject* m_pFloor{ nullptr };

	GameObject* m_pWall1{ nullptr };
	GameObject* m_pWall2{ nullptr };

	Character* m_pCharacter{};

	//PortalGun* m_pPortalGun{};

	//bool m_DrawPortalMap{ true };
	//float m_PortalMapScale{ 0.3f };

	Portal* m_pBluePortal{};
	Portal* m_pOrangePortal{};

};
