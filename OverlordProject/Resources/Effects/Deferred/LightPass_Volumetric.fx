#include "LightPass_Helpers.fx"

//VARIABLES
//*********
float4x4 gWorldViewProjection : WORLDVIEWPROJECTION;
float4x4 gMatrixViewProjInv;
float3 gEyePos = float3(0, 0, 0);
Light gCurrentLight;

//G-BUFFER DATA
Texture2D gTextureDiffuse;
Texture2D gTextureSpecular;
Texture2D gTextureNormal;
Texture2D gTextureDepth;


SamplerState gTextureSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;// or Mirror or Clamp or Border
	AddressV = Wrap;// or Mirror or Clamp or Border
};

//FIRST PASS STATES
//*****************

DepthStencilState gDepthStencilState_FIRST
{
	DepthEnable = TRUE; //Depth-Testing enabled
	DepthWriteMask = ZERO; //Don't write to the DepthBuffer (Read-Only DSV)
	DepthFunc = GREATER; //If the source data is greater than the destination data, the comparison passes

	StencilEnable = TRUE; //Stencil-Testing enabled
	FrontFaceStencilFunc = ALWAYS; //Always pass the comparison
	FrontFaceStencilPass = DECR_SAT; //Decrement the stencil value by 1, and clamp the result.
};

RasterizerState gRasterizerState_FIRST
{
	CullMode = BACK;
	FillMode = SOLID;
};

BlendState gBlendState_FIRST
{
	BlendEnable[0] = false;
};


//SECOND PASS STATES
//******************
DepthStencilState gDepthStencilState_SECOND
{
	DepthEnable = TRUE; //Depth-Testing Enabled
	DepthWriteMask = ZERO; //Depth-Writing disabled (Read-Only DSV)
	DepthFunc = GREATER_EQUAL;  //If the source data is greater than or equal to the destination data, the comparison passes.

	StencilEnable = TRUE; //Stencil-Testing Enabled
	BackFaceStencilFunc = EQUAL; //If the source data is equal to the destination data, the comparison passes
	BackFaceStencilPass = KEEP; //Keep the existing stencil data
};

RasterizerState gRasterizerState_SECOND
{
	CullMode = FRONT;
	FillMode = SOLID;
	DepthClipEnable = FALSE;
};

BlendState gBlendState_SECOND
{
	BlendEnable[0] = true;
	SrcBlend = ONE;
	DestBlend = ONE;
	BlendOp = ADD;
};

//VS-PS IO
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};


//VERTEX SHADER
//*************
VS_OUTPUT VS(float3 position:POSITION, float2 texCoord:TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = mul(float4(position,1), gWorldViewProjection);
	output.TexCoord = texCoord;

	return output;
}

//PIXEL SHADER
//************
[earlydepthstencil] //Execute Depth&Stencil tests before PS execution (not during as usual)
float4 PS(VS_OUTPUT input) :SV_TARGET
{
	int2 screenCoord = input.Position.xy;
	int3 loadCoord = int3(screenCoord, 0);

	//World Position from depth
	float depth = gTextureDepth.Load(loadCoord).r;
	float textureWidth;
	float textureHeight;
	gTextureDepth.GetDimensions(textureWidth, textureHeight);
	float3 P = DepthToWorldPosition(depth, screenCoord, float2(textureWidth, textureHeight), gMatrixViewProjInv);

	//View Direction
	float3 V = normalize(P - gEyePos);

	//Retrieve Components
	float3 diffuse = gTextureDiffuse.Load(loadCoord).rgb; //DIFFUSE
	float4 specular = gTextureSpecular.Load(loadCoord); //SPECULAR
	float shininess = exp2(specular.a * 10.5f);
	float3 N = gTextureNormal.Load(loadCoord).xyz; //NORMAL
	
	//Material
	Material mat = (Material)0;
	mat.Diffuse = diffuse;
	mat.Specular = specular.rgb;
	mat.Shininess = shininess;

	//Do Lighting (Point=0, Spot=1)
	LightingResult result = (LightingResult)0;
	if (gCurrentLight.Type == 0) // Pointlight
	{
		result = DoPointLighting(gCurrentLight, mat, V, N, P);
	}
	else //Spotlight
	{
		result = DoSpotLighting(gCurrentLight, mat, V, N, P);
	}

	//FINAL COLOR
	return float4((mat.Diffuse * result.Diffuse) + (mat.Specular * result.Specular), 1); // + Ambient (What's on backbuffer)
}

technique11 Default
{
	//PASS ONE
	pass one
	{
		SetRasterizerState(gRasterizerState_FIRST);
		SetDepthStencilState(gDepthStencilState_FIRST, 1);
		SetBlendState(gBlendState_FIRST, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader( CompileShader ( vs_4_0, VS() ));
		SetGeometryShader( NULL );
		SetPixelShader( NULL );
	}

	//PASS TWO
	pass two
	{
		SetRasterizerState(gRasterizerState_SECOND);
		SetDepthStencilState(gDepthStencilState_SECOND, 1);
		SetBlendState(gBlendState_SECOND, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader(CompileShader (vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader (ps_4_0, PS()));
	}
}

