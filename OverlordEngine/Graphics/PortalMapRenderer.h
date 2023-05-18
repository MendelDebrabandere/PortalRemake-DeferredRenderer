#pragma once
class PortalMapMaterial;

class PortalMapRenderer : public Singleton<PortalMapRenderer>
{
public:
	PortalMapRenderer(const PortalMapRenderer& other) = delete;
	PortalMapRenderer(PortalMapRenderer&& other) noexcept = delete;
	PortalMapRenderer& operator=(const PortalMapRenderer& other) = delete;
	PortalMapRenderer& operator=(PortalMapRenderer&& other) noexcept = delete;

	void UpdateMeshFilter(const SceneContext& sceneContext, MeshFilter* pMeshFilter) const;

	void Begin(const SceneContext&);
	void DrawMesh(const SceneContext& sceneContext, MeshFilter* pMeshFilter, const XMFLOAT4X4& meshWorld, const std::vector<XMFLOAT4X4>& meshBones = {});
	void End(const SceneContext&) const;

	ID3D11ShaderResourceView* GetShadowMap() const;
	const XMFLOAT4X4& GetLightVP() const { return m_LightVP; }

	void Debug_DrawDepthSRV(const XMFLOAT2& position = { 0.f,0.f }, const XMFLOAT2& scale = { 1.f,1.f }, const XMFLOAT2& pivot = { 0.f,0.f }) const;

protected:
	void Initialize() override;

private:
	friend class Singleton<PortalMapRenderer>;
	PortalMapRenderer() = default;
	~PortalMapRenderer();

	//Rendertarget to render the 'shadowmap' to (depth-only)
	//Contains depth information for all rendered shadow-casting meshes from a light's perspective (usual the main directional light)
	RenderTarget* m_pPortalRenderTarget{ nullptr };

	//Light ViewProjection (perspective used to render ShadowMap)
	XMFLOAT4X4 m_LightVP{};


	PortalMapMaterial* m_pPortalMapGenerator{ nullptr };

	MaterialTechniqueContext m_GeneratorTechniqueContext;
};

