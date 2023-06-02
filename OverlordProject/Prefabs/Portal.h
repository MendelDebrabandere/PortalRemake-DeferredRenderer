#pragma once
class PostPortalMaterial;
class PortalMaterial;
class Character;

enum class PortalType
{
	Orange = 0,
	Blue = 1
};


class Portal final : public GameObject
{
public:
	Portal(PortalType type, Portal* pLinkedPortal, Character* character);
	~Portal() override;

	Portal(const Portal& other) = delete;
	Portal(Portal&& other) noexcept = delete;
	Portal& operator=(const Portal& other) = delete;
	Portal& operator=(Portal&& other) noexcept = delete;

	void SetLinkedPortal(Portal* pPortal) { m_pLinkedPortal = pPortal; }

	PortalMaterial* GetScreenMat() const { return m_pScreenMat; }
	CameraComponent* GetCamera() const { return m_pCameraComponent; }
	RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }

	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

	void UpateNearClipPlane();

	void SetWall(PxShape* wall);

private:
	void DoCameraRotations(const SceneContext&);
	void DoCollisionLogic(const SceneContext&);
	void DoTeleportingLogic(const SceneContext&);
	void DoCubeLogic(const SceneContext&);

	Character* m_pCharacter{};
	bool m_CharacterEntered{ false };
	bool m_CharacterLeft{ false };

	Portal* m_pLinkedPortal{};

	GameObject* m_pCameraPivot{};
	GameObject* m_pCameraObject{};
	CameraComponent* m_pCameraComponent{};

	RenderTarget* m_pRenderTarget{};

	ModelComponent* m_pScreenModel{};
	PortalMaterial* m_pScreenMat{};
	PortalType m_Type{};

	XMFLOAT4 m_Color{};

	PxShape* m_pWall{};
	PxTransform m_WallPos{};

	GameObject* m_pCube{ nullptr };
};

