#pragma once
class DeferredRenderingScene : public GameScene
{
public:
	DeferredRenderingScene() :GameScene(L"DeferredRenderingScene") {}
	~DeferredRenderingScene() override = default;
	DeferredRenderingScene(const DeferredRenderingScene& other) = delete;
	DeferredRenderingScene(DeferredRenderingScene&& other) noexcept = delete;
	DeferredRenderingScene& operator=(const DeferredRenderingScene& other) = delete;
	DeferredRenderingScene& operator=(DeferredRenderingScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void OnGUI() override;
	void Update() override;

	void LoadSponzaMesh(const std::wstring& meshName, const std::wstring& specularMap = L"", const std::wstring& normalMap = L"", bool useTransparency = false) const;
	bool GetSponzaTexture(const std::wstring& baseName, const std::wstring& suffix, const std::wstring& overrideName, std::wstring& result) const;

	GameObject* m_pSponza{};
	bool m_FlashLightMode{ false };
};

