#pragma once

class PortalRenderer : public Singleton<PortalRenderer>
{
public:
	PortalRenderer(const PortalRenderer& other) = delete;
	PortalRenderer(PortalRenderer&& other) noexcept = delete;
	PortalRenderer& operator=(const PortalRenderer& other) = delete;
	PortalRenderer& operator=(PortalRenderer&& other) noexcept = delete;

	void Begin(const SceneContext&, CameraComponent* pCamera = nullptr, bool rt1 = true);
	void DrawMesh(const SceneContext&, MeshFilter*, const XMFLOAT4X4&, const std::vector<XMFLOAT4X4>& = {}) {};
	void End(const SceneContext&) const;

	void SetRenderTargets(RenderTarget* pPortal1RT, RenderTarget* pPortal2RT);

protected:
	void Initialize() override;

private:
	friend class Singleton<PortalRenderer>;
	PortalRenderer() = default;
	~PortalRenderer();

	//Rendertargets to render the 'portalmaps' to
	RenderTarget* m_pPortal1RenderTarget{ nullptr };
	RenderTarget* m_pPortal2RenderTarget{ nullptr };

	ID3DX11Effect* m_pPortalMapGenerator{ nullptr };

	static constexpr UINT m_VertexCount{ 4 };
	static ID3D11Buffer* m_pDefaultVertexBuffer;

	ID3D11InputLayout* m_pDefaultInputLayout{};
};

