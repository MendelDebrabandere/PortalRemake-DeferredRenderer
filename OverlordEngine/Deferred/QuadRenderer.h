#pragma once

#pragma region Rect
struct Quad
{
	int x{};
	int y{};
	int width{};
	int height{};

	bool operator!=(const Quad& other) const;
	Quad& operator=(const Quad& other);
};
#pragma endregion

#pragma region QuadMaterial
class QuadMaterial : public Material<QuadMaterial>
{
public:
	QuadMaterial();
	~QuadMaterial() override = default;

	QuadMaterial(const QuadMaterial& other) = delete;
	QuadMaterial(QuadMaterial&& other) noexcept = delete;
	QuadMaterial& operator=(const QuadMaterial& other) = delete;
	QuadMaterial& operator=(QuadMaterial&& other) noexcept = delete;

protected:
	void InitializeEffectVariables() override {};
};
#pragma endregion

class QuadRenderer : public Singleton<QuadRenderer>
{
public:
	virtual ~QuadRenderer();
	QuadRenderer(const QuadRenderer& other) = delete;
	QuadRenderer(QuadRenderer&& other) noexcept = delete;
	QuadRenderer& operator=(const QuadRenderer& other) = delete;
	QuadRenderer& operator=(QuadRenderer&& other) noexcept = delete;

	void Draw(ID3D11ShaderResourceView* pSRV) { Draw(pSRV, {}, QuadMode::Full); }
	void Draw(ID3D11ShaderResourceView* pSRV, const Quad& dim) { Draw(pSRV, dim, QuadMode::Custom); }

	void Draw(const BaseMaterial* pMaterial) { Draw(pMaterial, {}, QuadMode::Full); }
	void Draw(const BaseMaterial* pMaterial, const Quad& dim) { Draw(pMaterial, dim, QuadMode::Custom); }

protected:
	void Initialize() override;

private:
	friend class Singleton<QuadRenderer>;
	QuadRenderer() = default;

	enum class QuadMode
	{
		Custom = 0,
		Full = 1
	};

	void Draw(ID3D11ShaderResourceView* pSRV, const Quad& dim, QuadMode mode);
	void Draw(const BaseMaterial* pMaterial, const Quad& dim, QuadMode mode);

	QuadMaterial* m_pMaterial{};
	ID3D11Buffer* m_pStaticVB{};
	ID3D11Buffer* m_pDynamicVB{};

	Quad m_PreviousRect{ -1,-1,-1,-1 };

	XMFLOAT2 m_ScreenSize{};
	XMFLOAT2 m_HalfScreenSize{};
};

