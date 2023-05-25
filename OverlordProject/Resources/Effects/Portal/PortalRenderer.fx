float4x4 gWorldViewProj : WORLDVIEWPROJECTION;
float4x4 gWorld : WORLD;
Texture2D gTexture;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;// or Mirror or Clamp or Border
	AddressV = Wrap;// or Mirror or Clamp or Border
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = NONE;
};

BlendState EnableBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD0;

};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	//output.pos = mul(float4(input.pos, 1.f), mul(gWorld, gWorldViewProj));
	output.pos = float4(input.pos, 1.f);

	output.texCoord = input.texCoord;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_TARGET
{
	float4 color = gTexture.Sample(samLinear, input.texCoord);
	return color;
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
	pass P0
	{
		// Set states...
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}

//technique11 TransparencyTech
//{
//	pass P0
//	{
//		SetRasterizerState(NoCulling);
//		SetDepthStencilState(EnableDepth, 0);
//		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
//
//		SetVertexShader(CompileShader(vs_4_0, VS()));
//		SetGeometryShader(NULL);
//		SetPixelShader(CompileShader(ps_4_0, PS()));
//	}
//}