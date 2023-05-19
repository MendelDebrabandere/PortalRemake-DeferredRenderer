#pragma once

#pragma region Directional_LightPass_Material
//LIGHTPASS_DIRECTIONAL
class DirectionalLightMaterial : public Material<DirectionalLightMaterial>
{
public:
	DirectionalLightMaterial():Material(L"Effects/Deferred/LightPass_Directional.fx"){}
	~DirectionalLightMaterial() override = default;
	DirectionalLightMaterial(const DirectionalLightMaterial& other) = delete;
	DirectionalLightMaterial(DirectionalLightMaterial&& other) noexcept = delete;
	DirectionalLightMaterial& operator=(const DirectionalLightMaterial& other) = delete;
	DirectionalLightMaterial& operator=(DirectionalLightMaterial&& other) noexcept = delete;

protected:
	void InitializeEffectVariables() override {}
};
#pragma endregion

#pragma region Volumetric_LightPass_Material
//LIGHTPASS_VOLUMETRIC
class VolumetricLightMaterial : public Material<VolumetricLightMaterial>
{
public:
	VolumetricLightMaterial():Material(L"Effects/Deferred/LightPass_Volumetric.fx"){}
	~VolumetricLightMaterial() override = default;
	VolumetricLightMaterial(const VolumetricLightMaterial& other) = delete;
	VolumetricLightMaterial(VolumetricLightMaterial&& other) noexcept = delete;
	VolumetricLightMaterial& operator=(const VolumetricLightMaterial& other) = delete;
	VolumetricLightMaterial& operator=(VolumetricLightMaterial&& other) noexcept = delete;

protected:
	void InitializeEffectVariables() override {}
};
#pragma endregion

#pragma region Helper
//HELPER
inline XMMATRIX MatrixAlignVectors(const XMFLOAT4& v1, const XMFLOAT4& v2)
{
	const XMVECTOR vec1 = XMLoadFloat4(&v1);
	const XMVECTOR vec2 = XMLoadFloat4(&v2);

	const float cos_theta = XMVectorGetX(XMVector3Dot(XMVector3Normalize(vec1), XMVector3Normalize(vec2)));
	const float angle = acos(cos_theta);

	if (angle == 0.f)
		return XMMatrixIdentity();

	const XMVECTOR axis = XMVector3Normalize(XMVector3Cross(vec1, vec2));

	return XMMatrixRotationAxis(axis, angle);
}
#pragma endregion

class DeferredLightRenderer final
{
public:
	DeferredLightRenderer() = default;
	~DeferredLightRenderer();
	DeferredLightRenderer(const DeferredLightRenderer& other) = delete;
	DeferredLightRenderer(DeferredLightRenderer&& other) noexcept = delete;
	DeferredLightRenderer& operator=(const DeferredLightRenderer& other) = delete;
	DeferredLightRenderer& operator=(DeferredLightRenderer&& other) noexcept = delete;

	void Initialize(const D3D11Context& d3dContext);

	void DirectionalLightPass(const SceneContext& sceneContext, ID3D11ShaderResourceView* const gbufferSRVs[]) const;
	void VolumetricLightPass(const SceneContext& sceneContext, ID3D11ShaderResourceView* const gbufferSRVs[], ID3D11RenderTargetView* pDefaultRTV) const;
	void CreateReadOnlyDSV(const D3D11Context& d3dContext, ID3D11Resource* pDepthResource, DXGI_FORMAT format);

private:

	//Read-Only DSV (Depth ReadOnly, Stencil Read/Write)
	ID3D11DepthStencilView* m_pReadOnlyDepthStencilView{};

	//Directional LightPass (Directional Light)
	DirectionalLightMaterial* m_pDirectionalLightMaterial{};

	//Volumetric LightPass (Point & Spot Lights)
	VolumetricLightMaterial* m_pVolumetricLightMaterial{};

	MeshFilter* m_pSphereMesh{}; //Point Lights
	ID3D11Buffer* m_pSphereVB{}, * m_pSphereIB{}; //Sphere Vertex/IndexBuffer

	MeshFilter* m_pConeMesh{}; //Spot Lights
	ID3D11Buffer* m_pConeVB{}, * m_pConeIB{}; //Cone Vertex/IndexBuffer

	UINT m_VertexStride{}; //Sphere & Cone VertexStride

	void DrawVolumetricLight(const SceneContext& sceneContext, const Light& light) const;
};

