#pragma once
class DeferredLightRenderer;

class DeferredRenderer : public Singleton<DeferredRenderer>
{
public:
	~DeferredRenderer();
	DeferredRenderer(const DeferredRenderer& other) = delete;
	DeferredRenderer(DeferredRenderer&& other) noexcept = delete;
	DeferredRenderer& operator=(const DeferredRenderer& other) = delete;
	DeferredRenderer& operator=(DeferredRenderer&& other) noexcept = delete;

	enum class eGBufferId
	{
		LightAccumulation = 0, //Ambient > Stored on Default RT
		Diffuse = 1,
		Specular = 2,
		Normal = 3,
		Depth = 4
	};

	void Begin(const SceneContext& sceneContext) const;
	void End(const SceneContext& sceneContext) const;

	void DrawImGui();

protected:
	void Initialize() override;

private:
	friend class Singleton<DeferredRenderer>;
	DeferredRenderer() = default;

	//GBUFFER
	static constexpr int RT_COUNT{ 4 }; //GBuffer RenderTarget Count 
	static constexpr int SRV_COUNT{ RT_COUNT + 1 }; //GBuffer ShaderResourceView Count 

	RenderTarget* CreateRenderTarget(UINT width, UINT height, DXGI_FORMAT format) const; //Helper to construct RenderTargets
	RenderTarget* m_GBuffer[RT_COUNT]{}; //GBuffer RenderTargets ( Ambient+Depth | Diffuse | Specular | Normal )

	ID3D11RenderTargetView* m_pDefaultRenderTargetView{}; //Main Game RenderTargetView (RTV)
	ID3D11DepthStencilView* m_pDefaultDepthStencilView{}; //Main Game DepthStencelView (DSV)

	ID3D11RenderTargetView* m_RenderTargetViews[RT_COUNT]{}; //References to RenderTargetViews
	ID3D11ShaderResourceView* m_ShaderResourceViews[SRV_COUNT]{}; //References to ShaderResourceViews ( Ambient | Diffuse | Specular | Normal | Depth)

	//LightPass
	DeferredLightRenderer* m_pLightPassRenderer{}; //Helper Class for Deferred Lighting (Directional + Volumetric)

	//Debugging
	void Debug_DrawGBuffer() const;
	bool m_DrawImGui{ false };
	int m_VizRTVid{ -1 };
};

