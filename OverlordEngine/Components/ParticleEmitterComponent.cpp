#include "stdafx.h"
#include "ParticleEmitterComponent.h"
#include "Misc/ParticleMaterial.h"

ParticleMaterial* ParticleEmitterComponent::m_pParticleMaterial{};

ParticleEmitterComponent::ParticleEmitterComponent(const std::wstring& assetFile, const ParticleEmitterSettings& emitterSettings, UINT particleCount):
	m_ParticlesArray(new Particle[particleCount]),
	m_ParticleCount(particleCount), //How big is our particle buffer?
	m_MaxParticles(particleCount), //How many particles to draw (max == particleCount)
	m_AssetFile(assetFile),
	m_EmitterSettings(emitterSettings)
{
	m_enablePostDraw = true; //This enables the PostDraw function for the component
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	//TODO_W9(L"Implement Destructor")
	delete m_ParticlesArray;
	m_pVertexBuffer->Release();
}

void ParticleEmitterComponent::Initialize(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement Initialize")
	if (!m_pParticleMaterial)
		m_pParticleMaterial = MaterialManager::Get()->CreateMaterial<ParticleMaterial>();

	CreateVertexBuffer(sceneContext);

	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);
}

void ParticleEmitterComponent::CreateVertexBuffer(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement CreateVertexBuffer")
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();

	// set up the description of the buffer
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VertexParticle) * m_ParticleCount; 
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;

	// create the buffer
	HRESULT hr = sceneContext.d3dContext.pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer);
	if (FAILED(hr))
	{
		std::cout << "Failed to create vertex bugger for particle emitter component\n";
	}

}

void ParticleEmitterComponent::Update(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement Update")
	// 1. Calculate particleInterval
	float particleInterval = (m_EmitterSettings.maxEnergy - m_EmitterSettings.minEnergy) / m_ParticleCount;

	// 2. Increase m_LastParticleInit by elapsed time
	m_LastParticleSpawn += sceneContext.pGameTime->GetElapsed();

	// 3a. Set m_ActiveParticles to zero
	m_ActiveParticles = 0;

	// 3b. Map vertexbuffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	sceneContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

	// 3c. Cast pData to VertexParticle* pBuffer
	VertexParticle* pBuffer = static_cast<VertexParticle*>(mappedSubresource.pData);

	// 3e. Iterate the Particle Array
	for (UINT i = 0; i < m_ParticleCount; ++i)
	{
		// 3f1. If the particle is active, update it
		if (m_ParticlesArray[i].isActive)
		{
			UpdateParticle(m_ParticlesArray[i], sceneContext.pGameTime->GetElapsed());
		}

		// 3f2. If the particle is not active, spawn it if possible
		if (!m_ParticlesArray[i].isActive && m_LastParticleSpawn >= particleInterval)
		{
			SpawnParticle(m_ParticlesArray[i]);
		}

		// 3f3. If the particle is active (after potential update/spawn), add it to the vertex buffer
		if (m_ParticlesArray[i].isActive)
		{
			pBuffer[m_ActiveParticles] = m_ParticlesArray[i].vertexInfo;
			++m_ActiveParticles;
		}
	}

	// 3g. Unmap the vertex buffer
	sceneContext.d3dContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);

}

void ParticleEmitterComponent::UpdateParticle(Particle& p, float elapsedTime) const
{
	//TODO_W9(L"Implement UpdateParticle")
	// 1. Check if the particle is active, if not, return
	if (!p.isActive)
	{
		return;
	}

	// 2. Subtract the elapsedTime from the particle’s currentEnergy.
	p.currentEnergy -= elapsedTime;

	// 2a. If currentEnergy is smaller than ZERO, deactivate the particle and return
	if (p.currentEnergy < 0)
	{
		p.isActive = false;
		return;
	}

	// Create a local variable, called ‘lifePercent of type float, this is the percentual particle lifetime. 
	float lifePercent = p.currentEnergy / p.totalEnergy; // [At start: lifePercent = 1, At end: lifePercent = 0]

	// 3a. Update vertexInfo.Position
	// Add the velocity (m_EmitterSettings) multiplied by the elapsedTime, 
	// this way our particle moves in the direction of the velocity defined by the emitter settings.
	p.vertexInfo.Position.x += m_EmitterSettings.velocity.x * elapsedTime;
	p.vertexInfo.Position.y += m_EmitterSettings.velocity.y * elapsedTime;
	p.vertexInfo.Position.z += m_EmitterSettings.velocity.z * elapsedTime;


	// 3b. vertexInfo.Color
	// i. Our color equals the color given by the emitter settings
	p.vertexInfo.Color = m_EmitterSettings.color;
	// ii. The alpha value of the particle color should fade out over time. 
	p.vertexInfo.Color.w *= lifePercent * 2; // Adjust the 2 as needed to delay the fade-out effect

	// 3c. vertexInfo.Size
	// Based on the particle’s sizeChange value, our particle shrinks or grows over time.
	if (p.sizeChange < 1) // ii. If sizeChange is smaller than 1 (Shrink)
	{
		p.vertexInfo.Size = p.initialSize * (1 - lifePercent * (1 - p.sizeChange));
	}
	else if (p.sizeChange > 1) // iii. If sizeChange is bigger than 1 (Grow)
	{
		p.vertexInfo.Size = p.initialSize * (1 + lifePercent * (p.sizeChange - 1));
	}
}

void ParticleEmitterComponent::SpawnParticle(Particle& p)
{
	//TODO_W9(L"Implement SpawnParticle")
	// 1. Set particle’s isActive to true
	p.isActive = true;

	// 2. Energy Initialization
	// a. totalEnergy and currentEnergy of the particle are both equal to a random float between minEnergy and maxEnergy (see m_EmitterSettings) 
	p.totalEnergy = p.currentEnergy = MathHelper::randF(m_EmitterSettings.minEnergy, m_EmitterSettings.maxEnergy);

	// 3. Position Initialization
	// a. We need to calculate a random position; this position is determined by the emitter radius of our particle system.
	XMVECTOR randomDirection = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f); // i. We start by defining a unit vector.
	XMMATRIX randomRotationMatrix = XMMatrixRotationRollPitchYaw(MathHelper::randF(-XM_PI, XM_PI), MathHelper::randF(-XM_PI, XM_PI), MathHelper::randF(-XM_PI, XM_PI)); // ii. random rotation matrix
	randomDirection = XMVector3TransformNormal(randomDirection, randomRotationMatrix); // iii. transform our randomDirection vector with our randomRotationMatrix.
	float randomDistance = MathHelper::randF(m_EmitterSettings.minEmitterRadius, m_EmitterSettings.maxEmitterRadius); // c. calculating the distance of our particle starting point.
	auto xmvec = XMLoadFloat3(&GetTransform()->GetPosition()) + randomDirection * randomDistance; // d. vertexInfo.Position = ‘our random direction’ * ‘our random distance’
	XMStoreFloat3(&p.vertexInfo.Position, xmvec);
	
	// 4. Size Initialization
	// a. Our vertexInfo.Size and initialSize are both equal to a random value that lays between MinSize and MaxSize (see EmitterSettings)
	p.vertexInfo.Size = p.initialSize = MathHelper::randF(m_EmitterSettings.minSize, m_EmitterSettings.maxSize);
	// b. sizeChange is equal to a random value that lays between minScale and maxScale (see EmitterSettings)
	p.sizeChange = MathHelper::randF(m_EmitterSettings.minScale, m_EmitterSettings.maxScale);

	// 5. Rotation Initialization
	// a. The rotation (vertexInfo.Rotation) is a random value between –PI and PI.
	p.vertexInfo.Rotation = MathHelper::randF(-XM_PI, XM_PI);

	// 6. Color Initialisation
	// a. The particle’s color (vertexInfo.Color) is equal to the color from the emitter settings.
	p.vertexInfo.Color = m_EmitterSettings.color;
	
}

void ParticleEmitterComponent::PostDraw(const SceneContext& sceneContext)
{
	//TODO_W9(L"Implement PostDraw")

	// 1. Set shader variables
	auto worldViewProjMatrix = sceneContext.pCamera->GetViewProjection();
	m_pParticleMaterial->SetVariable_Matrix(L"gWorldViewProj", worldViewProjMatrix);

	auto viewInverseMatrix = sceneContext.pCamera->GetViewInverse();
	m_pParticleMaterial->SetVariable_Matrix(L"gViewInverse", viewInverseMatrix);

	m_pParticleMaterial->SetVariable_Texture(L"gParticleTexture", m_pParticleTexture);

	// 2. Get technique context
	auto techniqueContext = m_pParticleMaterial->GetTechniqueContext();

	// 3. Set input layout
	auto deviceContext = sceneContext.d3dContext.pDeviceContext;
	deviceContext->IASetInputLayout(techniqueContext.pInputLayout);

	// 4. Set primitive topology
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// 5. Set vertex buffer
	UINT stride = sizeof(VertexParticle);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// 6. Draw for each pass
	D3DX11_TECHNIQUE_DESC techDesc;
	techniqueContext.pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		techniqueContext.pTechnique->GetPassByIndex(p)->Apply(0, deviceContext);
		deviceContext->Draw(m_ActiveParticles, 0);
	}
}

void ParticleEmitterComponent::DrawImGui()
{
	if(ImGui::CollapsingHeader("Particle System"))
	{
		ImGui::SliderUInt("Count", &m_ParticleCount, 0, m_MaxParticles);
		ImGui::InputFloatRange("Energy Bounds", &m_EmitterSettings.minEnergy, &m_EmitterSettings.maxEnergy);
		ImGui::InputFloatRange("Size Bounds", &m_EmitterSettings.minSize, &m_EmitterSettings.maxSize);
		ImGui::InputFloatRange("Scale Bounds", &m_EmitterSettings.minScale, &m_EmitterSettings.maxScale);
		ImGui::InputFloatRange("Radius Bounds", &m_EmitterSettings.minEmitterRadius, &m_EmitterSettings.maxEmitterRadius);
		ImGui::InputFloat3("Velocity", &m_EmitterSettings.velocity.x);
		ImGui::ColorEdit4("Color", &m_EmitterSettings.color.x, ImGuiColorEditFlags_NoInputs);
	}
}