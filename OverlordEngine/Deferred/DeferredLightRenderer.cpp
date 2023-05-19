#include "stdafx.h"
#include "DeferredLightRenderer.h"

DeferredLightRenderer::~DeferredLightRenderer()
{
	//Release Resources
	SafeRelease(m_pReadOnlyDepthStencilView);
}

void DeferredLightRenderer::Initialize(const D3D11Context& d3dContext)
{
	//Directional LightPass
	m_pDirectionalLightMaterial = MaterialManager::Get()->CreateMaterial<DirectionalLightMaterial>();

	//Volumetric LightPass
	m_pVolumetricLightMaterial = MaterialManager::Get()->CreateMaterial<VolumetricLightMaterial>();
	const auto inputLayoutID = m_pVolumetricLightMaterial->GetTechniqueContext().inputLayoutID;

	//Sphere Light Mesh
	m_pSphereMesh = ContentManager::Load<MeshFilter>(L"Meshes/Unitsphere.ovm");

	m_pSphereMesh->BuildVertexBuffer(d3dContext, m_pVolumetricLightMaterial);
	m_pSphereVB = m_pSphereMesh->GetVertexBufferData(inputLayoutID).pVertexBuffer;

	m_pSphereMesh->BuildIndexBuffer(d3dContext);
	m_pSphereIB = m_pSphereMesh->GetIndexBuffer();

	//Cone Light Mesh
	m_pConeMesh = ContentManager::Load<MeshFilter>(L"Meshes/Unitcone.ovm");

	m_pConeMesh->BuildVertexBuffer(d3dContext, m_pVolumetricLightMaterial);
	auto& vbData = m_pConeMesh->GetVertexBufferData(inputLayoutID);
	m_pConeVB = vbData.pVertexBuffer;
	m_VertexStride = vbData.VertexStride;

	m_pConeMesh->BuildIndexBuffer(d3dContext);
	m_pConeIB = m_pConeMesh->GetIndexBuffer();
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

void DeferredLightRenderer::VolumetricLightPass(const SceneContext& sceneContext, ID3D11ShaderResourceView* const gbufferSRVs[], ID3D11RenderTargetView* pDefaultRTV) const
{
	//Set DefaultRTV WITH Read-Only DSV (no write access to DepthBuffer, but still able to write to StencilBuffer)
	sceneContext.d3dContext.pDeviceContext->OMSetRenderTargets(1, &pDefaultRTV, m_pReadOnlyDepthStencilView);

	//Prepare Effect




	//Ambient SRV > Already on Main RenderTarget
	m_pVolumetricLightMaterial->SetVariable_Texture(L"gTextureDiffuse", gbufferSRVs[int(DeferredRenderer::eGBufferId::Diffuse)]);
	m_pVolumetricLightMaterial->SetVariable_Texture(L"gTextureSpecular", gbufferSRVs[int(DeferredRenderer::eGBufferId::Specular)]);
	m_pVolumetricLightMaterial->SetVariable_Texture(L"gTextureNormal", gbufferSRVs[int(DeferredRenderer::eGBufferId::Normal)]);
	m_pVolumetricLightMaterial->SetVariable_Texture(L"gTextureDepth", gbufferSRVs[int(DeferredRenderer::eGBufferId::Depth)]);

	m_pVolumetricLightMaterial->SetVariable_Matrix(L"gMatrixViewProjInv", sceneContext.pCamera->GetViewProjectionInverse());
	m_pVolumetricLightMaterial->SetVariable_Vector(L"gEyePos", sceneContext.pCamera->GetTransform()->GetWorldPosition());


	//Iterate Lights & Render Volumes
	for (auto& light : sceneContext.pLights->GetLights())
	{
		if (!light.isEnabled) continue;

		//Clear Stencil Buffer to 1
		sceneContext.d3dContext.pDeviceContext->ClearDepthStencilView(m_pReadOnlyDepthStencilView, D3D11_CLEAR_STENCIL, 0.f, 1);

		//Draw Next Light
		DrawVolumetricLight(sceneContext, light);
	}
}

void DeferredLightRenderer::DrawVolumetricLight(const SceneContext& sceneContext, const Light& light) const
{
	//Draw Light Volume & Apply Shading
	XMFLOAT3 s { light.range, light.range, light.range };
	XMMATRIX rot{ XMMatrixIdentity() };

	if (light.type == LightType::Spot)
	{
		s.x = s.y = tanf(XMConvertToRadians(light.spotLightAngle)) * light.range;
		rot = MatrixAlignVectors({ 0.f, 0.f, 1.f, 0.f }, light.direction);
	}

	//Calculate WVP
	const auto trans = XMMatrixTranslation(light.position.x, light.position.y, light.position.z);
	const auto scale = XMMatrixScaling(s.x, s.y, s.z);
	const auto world = scale * rot * trans;
	auto wvp = world * XMLoadFloat4x4(&sceneContext.pCamera->GetViewProjection());

	m_pVolumetricLightMaterial->SetVariable_Matrix(L"gWorldViewProjection", reinterpret_cast<float*>(&wvp));
	m_pVolumetricLightMaterial->SetVariable(L"gCurrentLight", &light, 0, sizeof(Light) - 4);

	const auto pDeviceContext = sceneContext.d3dContext.pDeviceContext;
	auto& techContext = m_pVolumetricLightMaterial->GetTechniqueContext();

	//Set inputLayout
	pDeviceContext->IASetInputLayout(techContext.pInputLayout);

	//Set VertexBuffer
	const UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, light.type == LightType::Point ? &m_pSphereVB : &m_pConeVB, &m_VertexStride, &offset);

	//Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(light.type == LightType::Point ? m_pSphereIB : m_pConeIB, DXGI_FORMAT_R32_UINT, 0);

	//Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	techContext.pTechnique->GetDesc(& techDesc);
	for (UINT p{0}; p < techDesc.Passes; ++p)
	{
		techContext.pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(light.type == LightType::Point ? m_pSphereMesh->GetIndexCount() : m_pConeMesh->GetIndexCount(), 0, 0);
	}
}

void DeferredLightRenderer::CreateReadOnlyDSV(const D3D11Context& d3dContext, ID3D11Resource* pDepthResource, DXGI_FORMAT format)
{
	//Create DSV with Read-Only Depth (m_pReadOnlyDepthStencilView)

	SafeRelease(m_pReadOnlyDepthStencilView);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
	descDSV.Format = format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	descDSV.Flags = D3D11_DSV_READ_ONLY_DEPTH; //DEPTH READ_ONLY! (STENCIL READ/WRITE)

	HANDLE_ERROR(d3dContext.pDevice->CreateDepthStencilView(pDepthResource, &descDSV, &m_pReadOnlyDepthStencilView));

}