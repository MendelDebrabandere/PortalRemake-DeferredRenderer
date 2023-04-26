float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float4x4 gWorldViewProj_Light;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float gShadowMapBias = 0.25f;
float4x4 gBones[70];

Texture2D gDiffuseMap;
Texture2D gShadowMap;

SamplerComparisonState cmpSampler
{
	// sampler state
	Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;

	// sampler comparison state
	ComparisonFunc = LESS_EQUAL;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 BoneIndices : BLENDINDICES;
	float4 BoneWeights : BLENDWEIGHTS;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 lPos : TEXCOORD1;
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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    // Skinning
    float4 finalPosition = float4(0, 0, 0, 0);
    float3 finalNormal = float3(0, 0, 0);

    for (int i = 0; i < 4; ++i)
    {
        float boneWeight = input.BoneWeights[i];
        int boneIndex = input.BoneIndices[i];

        float4 position = mul(float4(input.pos, 1.0f), gBones[boneIndex]);
        float3 normal = mul(input.normal, (float3x3)gBones[boneIndex]);

        finalPosition += boneWeight * position;
        finalNormal += boneWeight * normal;
    }

    output.pos = mul(finalPosition, gWorldViewProj);
    output.normal = normalize(finalNormal);
    output.texCoord = input.texCoord;
    output.lPos = mul(finalPosition, gWorldViewProj_Light);

    return output;
}

float2 texOffset(int u, int v)
{
    return float2(u / 1280.0f, v / 720.0f);
}

float EvaluateShadowMap(float4 lpos)
{
    // Re-homogenize position after interpolation
    lpos.xyz /= lpos.w;

    // If position is not visible to the light, don't illuminate it
    // Results in hard light frustum
    if (lpos.x < -1.0f || lpos.x > 1.0f ||
        lpos.y < -1.0f || lpos.y > 1.0f ||
        lpos.z < 0.0f || lpos.z > 1.0f) return 0;

    // Transform clip space coords to texture space coords (-1:1 to 0:1)
    lpos.x = lpos.x / 2 + 0.5;
    lpos.y = lpos.y / -2 + 0.5;

    // Percentage-Closer Filtering (PCF)
    float shadowValue = 0.0f;
    float2 texelSize = float2(1.0f / 1280.0f, 1.0f / 720.0f);
    int2 kernelSize = int2(3, 3);
    float2 kernelOffsets[9] = {
        float2(-1, -1), float2(0, -1), float2(1, -1),
        float2(-1,  0), float2(0,  0), float2(1,  0),
        float2(-1,  1), float2(0,  1), float2(1,  1)
    };

    for (int i = 0; i < 9; ++i)
    {
        float2 offset = kernelOffsets[i] * texelSize;
        float shadowMapDepth = gShadowMap.SampleCmpLevelZero(cmpSampler, lpos.xy + offset, lpos.z).r;
        shadowValue += (lpos.z <= shadowMapDepth + gShadowMapBias) ? 1 : shadowMapDepth;
        //shadowValue += shadowMapDepth;
    }

    shadowValue /= 9.0f;
    return shadowValue;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float shadowValue = EvaluateShadowMap(input.lPos);

	float4 diffuseColor = gDiffuseMap.Sample( samLinear,input.texCoord );
	float3 color_rgb= diffuseColor.rgb;
	float color_a = diffuseColor.a;
	
	//HalfLambert Diffuse :)
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	return float4( color_rgb * shadowValue, color_a );
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

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

