//**************//
// BASIC EFFECT //
//**************//

/*
	- Diffuse Color/Texture
	- Specular Color
	- SpecularLevel Texture (Blinn)
	- Specular Intensity (Shininess)
	- NormalMap Texture
	- Ambient Color
	- Ambient Intensity [0-1]
	- Opacity Texture/Value [0-1]
*/

//GLOBAL MATRICES
//***************
// The World View Projection Matrix
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;
// The ViewInverse Matrix - the third row contains the camera position!
float4x4 gViewInverse : VIEWINVERSE;
// The World Matrix
float4x4 gWorld : WORLD;

//STATES
//******
RasterizerState gRasterizerState
{
	FillMode = SOLID;
	CullMode = BACK;
};

BlendState gEnableBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};

BlendState gDisableBlending
{
	BlendEnable[0] = FALSE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};

DepthStencilState gDepthState
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

//SAMPLER STATES
//**************
SamplerState gTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	//Filter = ANISOTROPIC;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};

//LIGHT
//*****
float3 gLightDirection:DIRECTION
<
	string UIName = "Light Direction";
	string Object = "TargetLight";
> = float3(0.577f, 0.577f, 0.577f);

//DIFFUSE
//*******
bool gUseDiffuseMap
<
	string UIName = "Diffuse Texture";
	string UIWidget = "Bool";
> = false;

float4 gDiffuseColor
<
	string UIName = "Diffuse Color";
	string UIWidget = "Color";
> = float4(1, 1, 1, 1);

Texture2D gDiffuseMap
<
	string UIName = "Diffuse Texture";
	string UIWidget = "Texture";
> ;

//SPECULAR
//********
float4 gColorSpecular
<
	string UIName = "Specular Color";
	string UIWidget = "Color";
> = float4(1, 1, 1, 1);

Texture2D gSpecularMap
<
	string UIName = "Specular Level Texture";
	string UIWidget = "Texture";
> ;

bool gUseSpecularMap
<
	string UIName = "Specular Level Texture";
	string UIWidget = "Bool";
> = false;

int gShininess <
	string UIName = "Shininess";
	string UIWidget = "Slider";
	float UIMin = 1;
	float UIMax = 100;
	float UIStep = 0.1f;
> = 15;

//AMBIENT
//*******
float4 gColorAmbient
<
	string UIName = "Ambient Color";
	string UIWidget = "Color";
> = float4(1, 1, 1, 1);

float gAmbientIntensity
<
	string UIName = "Ambient Intensity";
	string UIWidget = "slider";
	float UIMin = 0;
	float UIMax = 1;
> = 0.15f;

//NORMAL MAPPING
//**************
bool gFlipGreenChannel
<
	string UIName = "Flip Green Channel";
	string UIWidget = "Bool";
> = false;

bool gUseNormalMap
<
	string UIName = "Normal Mapping";
	string UIWidget = "Bool";
> = false;

Texture2D gNormalMap
<
	string UIName = "Normal Texture";
	string UIWidget = "Texture";
> ;

//OPACITY
//***************
float gOpacityLevel <
	string UIName = "Opacity";
	string UIWidget = "slider";
	float UIMin = 0;
	float UIMax = 1;
> = 1.0f;


//VS IN & OUT
//***********
struct VS_Input
{
	float3 Position: POSITION;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
	float3 Binormal: BINORMAL;
	float2 TexCoord: TEXCOORD0;
};

struct VS_Output
{
	float4 Position: SV_POSITION;
	float4 WorldPosition: COLOR0;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
	float3 Binormal: BINORMAL;
	float2 TexCoord: TEXCOORD0;
};

float3 CalculateSpecular(float3 viewDirection, float3 normal, float2 texCoord)
{
	float3 hn = normalize(viewDirection + gLightDirection);
	float specularStrength = dot(-normal, hn);
	specularStrength = saturate(specularStrength);
	specularStrength = pow(specularStrength, gShininess);
	float3 specColor = gColorSpecular * specularStrength;

	//Use a Texture to control the specular level/color?
	if (gUseSpecularMap)
		specColor *= gSpecularMap.Sample(gTextureSampler, texCoord);

	return specColor;
}

float3 CalculateNormal(float3 tangent, float3 binormal, float3 normal, float2 texCoord)
{
	float3 newNormal = normal;

	if (gUseNormalMap)
	{
		if (gFlipGreenChannel)
		{
			binormal = -binormal;
		}

		float3x3 localAxis = float3x3(tangent, binormal, normal);

		float3 sampledNormal = 2.0f * gNormalMap.Sample(gTextureSampler, texCoord) - 1.0f;
		newNormal = normalize(mul(sampledNormal, localAxis));
	}

	return newNormal;
}

float4 CalculateDiffuse(float3 normal, float2 texCoord)
{
	float3 diffColor = gDiffuseColor;

	//HalfLambert Diffuse :)
	float diffuseStrength = dot(-normal, gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);

	diffColor *= diffuseStrength;

	//Use a texture to define the diffuse color?
	float alpha = 1.0f;
	if (gUseDiffuseMap)
	{
		float4 sampledColor = gDiffuseMap.Sample(gTextureSampler, texCoord);
		diffColor *= sampledColor.rgb;
		alpha = sampledColor.a;
	}

	return float4(diffColor, alpha);
}

// The main vertex shader
VS_Output MainVS(VS_Input input) {

	VS_Output output = (VS_Output)0;

	output.Position = mul(float4(input.Position, 1.0), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.0), gWorld);
	output.Normal = normalize(mul(input.Normal, (float3x3)gWorld));
	output.Tangent = normalize(mul(input.Tangent, (float3x3)gWorld));
	output.Binormal = normalize(mul(input.Binormal, (float3x3)gWorld));
	output.TexCoord = input.TexCoord;

	return output;
}

// The main pixel shader
float4 MainPS(VS_Output input) : SV_TARGET{
	// NORMALIZE
	input.Normal = normalize(input.Normal);
	input.Tangent = normalize(input.Tangent);
	input.Binormal = normalize(input.Binormal);

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

	//NORMAL
	float3 newNormal = CalculateNormal(input.Tangent, input.Binormal, input.Normal, input.TexCoord);

	//SPECULAR
	float3 specColor = CalculateSpecular(viewDirection, newNormal, input.TexCoord);

	//DIFFUSE
	float4 diffColor = CalculateDiffuse(newNormal, input.TexCoord);

	//AMBIENT
	float3 ambientColor = gColorAmbient * gAmbientIntensity * diffColor;

	//FINAL COLOR CALCULATION
	float3 finalColor = diffColor.rgb + specColor + ambientColor;

	//OPACITY
	float opacity = diffColor.a * gOpacityLevel;

	return float4(finalColor,opacity);
}

// Default Technique
technique10 Default {
	pass p0 {
		SetDepthStencilState(gDepthState, 0);
		SetRasterizerState(gRasterizerState);
		SetBlendState(gDisableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
	}
}

// Default Technique
technique10 Default_Alpha {
	pass p0 {
		SetDepthStencilState(gDepthState, 0);
		SetRasterizerState(gRasterizerState);
		SetBlendState(gEnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
	}
}