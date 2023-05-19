#include "stdafx.h"
#include "DeferredRenderingScene.h"

#include "Materials/BasicMaterial.h"
#include "Materials/BasicMaterial_Deferred.h"

#define FULL_SPONZA

void DeferredRenderingScene::Initialize()
{
	//Settings
	//********
	m_SceneContext.useDeferredRendering = true;

	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;

	//Sponza Mesh
	//***********
	m_pSponza = AddChild(new GameObject);

	LoadSponzaMesh(L"Arch");
	LoadSponzaMesh(L"Bricks");
	LoadSponzaMesh(L"Ceiling");
	LoadSponzaMesh(L"Floor");
	LoadSponzaMesh(L"Roof");
	LoadSponzaMesh(L"Column_A");
	LoadSponzaMesh(L"Column_B");
	LoadSponzaMesh(L"Column_C");

	LoadSponzaMesh(L"Lion_Head");
	LoadSponzaMesh(L"Lion_Plate");

	LoadSponzaMesh(L"FlagPole");
	LoadSponzaMesh(L"Details");

#ifdef FULL_SPONZA
	LoadSponzaMesh(L"Curtain_Blue", L"Curtain_Specular", L"Curtain_Normal");
	LoadSponzaMesh(L"Curtain_Green", L"Curtain_Specular", L"Curtain_Normal");
	LoadSponzaMesh(L"Curtain_Red", L"Curtain_Specular", L"Curtain_Normal");

	LoadSponzaMesh(L"Fabric_Blue", L"Fabric_Specular");
	LoadSponzaMesh(L"Fabric_Green", L"Fabric_Specular");
	LoadSponzaMesh(L"Fabric_Red", L"Fabric_Specular");

	LoadSponzaMesh(L"HangingVase");
	LoadSponzaMesh(L"Vase");
	LoadSponzaMesh(L"RoundVase");

	LoadSponzaMesh(L"Thorns", L"", L"", true);
	LoadSponzaMesh(L"HangingVase_Chain", L"", L"", true);
	LoadSponzaMesh(L"RoundVase_Plant", L"", L"", true);
#endif

	m_pSponza->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	m_pSponza->GetTransform()->Rotate(0, 90, 0);


	m_SceneContext.pCamera->GetTransform()->Translate(7.18050051f, 18.6554089f, -114.360588f);
	dynamic_cast<FreeCamera*>(m_SceneContext.pCamera->GetGameObject())->SetRotation(-14.0423603f, -2.72134018f);

	//Lights
	//******

	//Directional
	auto& dirLight = m_SceneContext.pLights->GetDirectionalLight();
	dirLight.isEnabled = true;
	dirLight.direction = { -0.577f, -0.577f, 0.577f , 1.0f };

	//Spot Light
	Light light = {};
	light.isEnabled = true;
	light.position = { 0.f,5.f,0.f,1.0f };
	light.direction = { 0.f,0.f,1.f,0.f };
	light.color = { 0.7f,0.f,0.f,1.f };
	light.intensity = 1.0f;
	light.spotLightAngle = 35.f;
	light.range = 150.0f;
	light.type = LightType::Spot;
	m_SceneContext.pLights->AddLight(light);

	//Point Light
	light = {};
	light.isEnabled = true;
	light.position = { 0.f,10.f,0.f,1.0f };
	light.color = { 0.f,1.f,0.f,1.f };
	light.intensity = 1.f;
	light.range = 30.0f;
	light.type = LightType::Point;
	m_SceneContext.pLights->AddLight(light);
}

void DeferredRenderingScene::Update()
{
	XMFLOAT4 pos{};
	pos.x = sin(m_SceneContext.pGameTime->GetTotal()) * 30.0f;
	pos.y = 15.0f;
	pos.z = cos(m_SceneContext.pGameTime->GetTotal()) * 30.0f;
	pos.w = 1.0f;

	m_SceneContext.pLights->GetLight(1).position = pos;
	m_SceneContext.pLights->GetLight(1).intensity = MathHelper::randF(0.2f, 1.2f);

	if (m_FlashLightMode)
	{
		const auto camPos = m_SceneContext.pCamera->GetTransform()->GetWorldPosition();
		const auto camForward = m_SceneContext.pCamera->GetTransform()->GetForward();
		m_SceneContext.pLights->GetLight(0).position = XMFLOAT4(camPos.x, camPos.y, camPos.z, 1.0f);
		m_SceneContext.pLights->GetLight(0).direction = XMFLOAT4(camForward.x, camForward.y, camForward.z, 1.0f);
	}
}

void DeferredRenderingScene::OnGUI()
{
	DeferredRenderer::Get()->DrawImGui();

	ImGui::Checkbox("Flashlight Mode", &m_FlashLightMode);
}

void DeferredRenderingScene::LoadSponzaMesh(const std::wstring& meshName, const std::wstring& specularMap, const std::wstring& normalMap, bool useTransparency) const
{
	//Mesh
	const auto meshPath = std::format(L"Meshes/sponza/Sponza_{}.ovm", meshName);
	const auto pModel = new ModelComponent(meshPath);

	//Material
	if (m_SceneContext.useDeferredRendering)
	{
		//BASIC-EFFECT_DEFERRED
		//*********************
		const auto pMaterial = MaterialManager::Get()->CreateMaterial<BasicMaterial_Deferred>();
		pModel->SetMaterial(pMaterial);
		pMaterial->UseTransparency(useTransparency);

		//Diffuse
		std::wstring texPath{};
		if(GetSponzaTexture(meshName, L"Diffuse", L"", texPath))
		{
			pMaterial->SetDiffuseMap(texPath);
		}

		//Specular
		if (GetSponzaTexture(meshName, L"Specular", specularMap, texPath))
		{
			pMaterial->SetSpecularMap(texPath);
		}

		//Normal
		if (GetSponzaTexture(meshName, L"Normal", normalMap, texPath))
		{
			pMaterial->SetNormalMap(texPath);
		}
	}
	else
	{
		//BASIC-EFFECT
		//************
		const auto pMaterial = MaterialManager::Get()->CreateMaterial<BasicMaterial>();
		pModel->SetMaterial(pMaterial);
		pMaterial->UseTransparency(useTransparency);

		//Diffuse
		std::wstring texPath{};
		if (GetSponzaTexture(meshName, L"Diffuse", L"", texPath))
		{
			pMaterial->SetDiffuseMap(texPath);
		}

		//Specular
		if (GetSponzaTexture(meshName, L"Specular", specularMap, texPath))
		{
			pMaterial->SetSpecularMap(texPath);
		}

		//Normal
		if (GetSponzaTexture(meshName, L"Normal", normalMap, texPath))
		{
			pMaterial->SetNormalMap(texPath);
		}
	}

	//Append to Root Object
	m_pSponza->AddComponent(pModel);
}

bool DeferredRenderingScene::GetSponzaTexture(const std::wstring& baseName, const std::wstring& suffix, const std::wstring& overrideName, std::wstring& result) const
{
	if(overrideName.empty())
	{
		result = std::format(L"Textures/sponza/Sponza_{}_{}.png", baseName, suffix);
		if (fs::exists(std::format(L"Resources/{}", result)))
		{
			return true;
		}
	}
	else 
	{
		result = std::format(L"Textures/sponza/Sponza_{}.png", overrideName);
		return true;
	}

	return false;
}
