#include "stdafx.h"
#include "QuadRenderer.h"

#pragma region Rect
bool ::Quad::operator!=(const Quad& other) const
{
	return (x != other.x) || (y != other.y) || (width != other.width) || (height != other.height);
}

::Quad& ::Quad::operator=(const Quad& other)
{
	x = other.x;
	y = other.y;
	width = other.width;
	height = other.height;

	return *this;
}
#pragma endregion

#pragma region QuadMaterial
QuadMaterial::QuadMaterial():
	Material(L"Effects/Deferred/QuadEffect.fx")
{
}
#pragma endregion

QuadRenderer::~QuadRenderer()
{
	SafeRelease(m_pStaticVB)
	SafeRelease(m_pDynamicVB)
}

void QuadRenderer::Initialize()
{
	m_ScreenSize = { static_cast<float>(m_GameContext.windowWidth), static_cast<float>(m_GameContext.windowHeight) };
	m_HalfScreenSize = { m_ScreenSize.x / 2.f, m_ScreenSize.y / 2.f };

	//Vertices
	VertexPosTex vertices[4];
	vertices[0] = { {-1.f, 1.f, 0.f}, {0.f, 0.f} };
	vertices[1] = { {1.f, 1.f, 0.f}, {1.f, 0.f} };
	vertices[2] = { {-1.f, -1.f, 0.f}, {0.f, 1.f} };
	vertices[3] = { {1.f, -1.f, 0.f}, {1.f, 1.f} };

	//Dynamic VB
	D3D11_BUFFER_DESC vertexBuffDesc;
	vertexBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBuffDesc.ByteWidth = sizeof(VertexPosTex) * 4;
	vertexBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];

	HANDLE_ERROR(m_GameContext.d3dContext.pDevice->CreateBuffer(&vertexBuffDesc, &initData, &m_pDynamicVB))

	//Static VB
	vertexBuffDesc.CPUAccessFlags = 0;
	vertexBuffDesc.Usage = D3D11_USAGE_DEFAULT;

	HANDLE_ERROR(m_GameContext.d3dContext.pDevice->CreateBuffer(&vertexBuffDesc, &initData, &m_pStaticVB))

	//Quad Material
	m_pMaterial = MaterialManager::Get()->CreateMaterial<QuadMaterial>();
}

void QuadRenderer::Draw(ID3D11ShaderResourceView* pSRV, const Quad& dim, QuadMode mode)
{
	m_pMaterial->SetVariable_Texture(L"gTexture", pSRV);
	Draw(m_pMaterial, dim, mode);

	ID3D11ShaderResourceView* pEmptySRV[] = { nullptr };
	m_GameContext.d3dContext.pDeviceContext->PSSetShaderResources(0, 1, pEmptySRV);
}

void QuadRenderer::Draw(const BaseMaterial* pMaterial, const Quad& dim, QuadMode mode)
{
	const auto pDeviceContext = m_GameContext.d3dContext.pDeviceContext;

	if(mode == QuadMode::Custom && m_PreviousRect != dim)
	{
		m_PreviousRect = dim;

		//Screenspace > Clipspace
		float ndcX = ((dim.x - m_HalfScreenSize.x) / m_ScreenSize.x) * 2.0f;
		float ndcY = ((m_HalfScreenSize.y - dim.y) / m_ScreenSize.y) * 2.0f;
		const float ndcWidth = (dim.width / m_ScreenSize.x) * 2.0f;
		const float ndcHeight = (dim.height / m_ScreenSize.y) * 2.0f;

		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		pDeviceContext->Map(m_pDynamicVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); //Prepare for writing

		const auto pVertices = static_cast<VertexPosTex*>(mappedResource.pData);
		pVertices[0] = { { ndcX, ndcY, 0.f },{0.f, 0.f} };
		pVertices[1] = { { ndcX + ndcWidth, ndcY, 0.f },{1.f, 0.f} };
		pVertices[2] = { { ndcX, ndcY - ndcHeight, 0.f },{0.f, 1.f} };
		pVertices[3] = { { ndcX + ndcWidth, ndcY - ndcHeight, 0.f },{1.f, 1.f} };

		pDeviceContext->Unmap(m_pDynamicVB, 0);
	}

	const auto& techContext = pMaterial->GetTechniqueContext();

	//Set Pipeline
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->IASetInputLayout(techContext.pInputLayout);

	constexpr UINT offset = 0;
	constexpr UINT stride = sizeof(VertexPosTex);
	pDeviceContext->IASetVertexBuffers(0, 1, (mode == QuadMode::Full) ? &m_pStaticVB : &m_pDynamicVB, &stride, &offset);

	//Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	techContext.pTechnique->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		techContext.pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->Draw(4,0);
	}
}
