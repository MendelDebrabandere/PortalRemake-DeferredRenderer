#include "stdafx.h"
#include "DeferredLightRenderer.h"

DeferredLightRenderer::~DeferredLightRenderer()
{
	//Release Resources
}

void DeferredLightRenderer::Initialize(const D3D11Context& /*d3dContext*/)
{
	//Directional LightPass
	m_pDirectionalLightMaterial = MaterialManager::Get()->CreateMaterial<DirectionalLightMaterial>();

	//Volumetric LightPass
	//...

	//Sphere Light Mesh
	//...


	//Cone Light Mesh
	//...
}

void DeferredLightRenderer::DirectionalLightPass(const SceneContext& sceneContext, ID3D11ShaderResourceView* const gbufferSRVs[]) const
{
	//Retrieve Directional light
	const auto& light = sceneContext.pLights->GetDirectionalLight();

	if(light.isEnabled)
	{
		//Prepare Effect

		//Ambient SRV > Already on Main RenderTarget
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureDiffuse", gbufferSRVs[int(DeferredRenderer::eGBufferId::Diffuse)]);
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureSpecular", gbufferSRVs[int(DeferredRenderer::eGBufferId::Specular)]);
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureNormal", gbufferSRVs[int(DeferredRenderer::eGBufferId::Normal)]);
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureDepth", gbufferSRVs[int(DeferredRenderer::eGBufferId::Depth)]);

		m_pDirectionalLightMaterial->SetVariable_Matrix(L"gMatrixViewProjInv", sceneContext.pCamera->GetViewProjectionInverse());
		m_pDirectionalLightMaterial->SetVariable_Vector(L"gEyePos", sceneContext.pCamera->GetTransform()->GetWorldPosition());
		m_pDirectionalLightMaterial->SetVariable(L"gDirectionalLight", &light, 0, sizeof(Light) - 4);


		//Draw Effect (Full Screen Quad)
		QuadRenderer::Get()->Draw(m_pDirectionalLightMaterial);
	}
}

void DeferredLightRenderer::VolumetricLightPass(const SceneContext& /*sceneContext*/, ID3D11ShaderResourceView* const /*gbufferSRVs*/[], ID3D11RenderTargetView* /*pDefaultRTV*/) const
{
	//Set DefaultRTV WITH Read-Only DSV (no write access to DepthBuffer, but still able to write to StencilBuffer)
	//...

	//Prepare Effect

	//Ambient SRV > Already on Main RenderTarget
	//...

	//Iterate Lights & Render Volumes
	//...
}

void DeferredLightRenderer::DrawVolumetricLight(const SceneContext& /*sceneContext*/, const Light& /*light*/) const
{
	//Draw Light Volume & Apply Shading
}

void DeferredLightRenderer::CreateReadOnlyDSV(const D3D11Context& /*d3dContext*/, ID3D11Resource* /*pDepthResource*/, DXGI_FORMAT /*format*/)
{
	//Create DSV with Read-Only Depth (m_pReadOnlyDepthStencilView)
}