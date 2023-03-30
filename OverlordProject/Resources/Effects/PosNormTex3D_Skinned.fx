float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);

float4x4 gBones[70];

Texture2D gDiffuseMap;
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

RasterizerState Solid
{
	FillMode = SOLID;
	CullMode = FRONT;
};

struct VS_INPUT{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 blendIdxs : BLENDINDICES;
	float4 blendWght : BLENDWEIGHTS;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
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

BlendState NoBlending
{
	BlendEnable[0] = FALSE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input){

	VS_OUTPUT output;
	// Step 1:	convert position into float4 and multiply with matWorldViewProj
	//output.pos = mul ( float4(input.pos,1.0f), gWorldViewProj );
	// Step 2:	rotate the normal: NO TRANSLATION
	//			this is achieved by clipping the 4x4 to a 3x3 matrix, 
	//			thus removing the postion row of the matrix
	//output.normal = normalize(mul(input.normal, (float3x3)gWorld));
	//output.texCoord = input.texCoord;

	//1. transformedPosition > contains the final transformed position
	float4 transformedPosition = float4(0,0,0,0);
	//2. transformedNormal > contain the final transformed normal
	float3 transformedNormal = float3(0,0,0);

	//3. For each bone
	for (int i = 0; i < 4; ++i)
	{
		float boneIdx = input.blendIdxs[i];

		//a. Make sure idx is bigger or equal to zero
		if (boneIdx < 0)
			continue;

		//b. Calculate bone space position and normal (gBones Matrix)
		float4 boneSpacePos = mul(float4(input.pos, 1.f), gBones[boneIdx]);
		float3 boneSpaceNormal = mul(input.pos, (float3x3)gBones[boneIdx]);

		//c. Append this position and normal to the transformedPosition and
		//   transformedNormal based on the corresponding bone weight
		transformedPosition += boneSpacePos * input.blendWght[i];
		transformedNormal += boneSpaceNormal * input.blendWght[i];
	}

	//4. Ensure that the w-component of the transformedPosition is 1
	transformedPosition.w = 1.f;

	//5. Transform the transformedPosition to Clipping Space (WVP Matrix)
	output.pos = mul(transformedPosition, gWorldViewProj);

	//6. Transform the transformedNormal to World Space (World Matrix) – (Rotation only!)
	output.normal = normalize(mul(transformedNormal, (float3x3)gWorld));

	output.texCoord = input.texCoord;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET{

	float4 diffuseColor = gDiffuseMap.Sample( samLinear,input.texCoord );
	float3 color_rgb= diffuseColor.rgb;
	float color_a = diffuseColor.a;
	
	//HalfLambert Diffuse :)
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	return float4( color_rgb , color_a );
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

