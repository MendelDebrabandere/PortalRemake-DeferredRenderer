#include "stdafx.h"
#include "PortalRenderer.h"

ID3D11Buffer* PortalRenderer::m_pDefaultVertexBuffer = {};

PortalRenderer::~PortalRenderer()
{
	if (m_pPortal1RenderTarget)
		SafeDelete(m_pPortal1RenderTarget)
	if (m_pPortal2RenderTarget)
		SafeDelete(m_pPortal2RenderTarget)

	SafeRelease(m_pDefaultInputLayout);
	SafeRelease(m_pDefaultVertexBuffer);
}

void PortalRenderer::Initialize()
{
	//load and save .fx
	m_pPortalMapGenerator = ContentManager::Load<ID3DX11Effect>(L"Effects/Portal/PortalRenderer.fx");

	const VertexPosTex vertices[m_VertexCount]
	{
		{{-1.f,-1.f,0.f},{0.f,1.f}},
		{{-1.f,1.f,0.f},{0.f,0.f}},
		{{1.f,-1.f,0.f},{1.f,1.f}},
		{{1.f,1.f,0.f},{1.f,0.f}}
	};

	//Create VertexBuffer
	D3D11_BUFFER_DESC buffDesc{};
	buffDesc.ByteWidth = sizeof(VertexPosTex) * m_VertexCount;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.CPUAccessFlags = 0;
	buffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = &vertices[0];

	HANDLE_ERROR(m_GameContext.d3dContext.pDevice->CreateBuffer(&buffDesc, &initData, &m_pDefaultVertexBuffer));




	//Create Default InputLayout if nullptr
	if (!m_pDefaultInputLayout)
	{
		//Assuming the InputLayout for all PostProcessing Effects is the same...
		EffectHelper::BuildInputLayout(m_GameContext.d3dContext.pDevice, m_pPortalMapGenerator->GetTechniqueByIndex(0), &m_pDefaultInputLayout);
	}
}

void PortalRenderer::Begin(const SceneContext& /*sceneContext*/, CameraComponent* /*pCamera*/, bool /*rt1*/)
{
	const auto pSourceSRV = m_GameContext.pGame->GetRenderTarget()->GetColorShaderResourceView();
	m_pPortalMapGenerator->GetVariableByName("gTexture")->AsShaderResource()->SetResource(pSourceSRV);
}

void PortalRenderer::End(const SceneContext&) const
{
	//1. Reset the Main Game RenderTarget back to default (OverlordGame::SetRenderTarget)
	m_GameContext.pGame->SetRenderTarget(nullptr);
}


void PortalRenderer::SetRenderTargets(RenderTarget* pPortal1RT, RenderTarget* pPortal2RT)
{
	m_pPortal1RenderTarget = pPortal1RT;
	m_pPortal2RenderTarget = pPortal2RT;
}
