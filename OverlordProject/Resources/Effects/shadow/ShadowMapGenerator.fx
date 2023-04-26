float4x4 gWorld;
float4x4 gLightViewProj;
float4x4 gBones[70];
 
DepthStencilState depthStencilState
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState rasterizerState
{
	FillMode = SOLID;
	CullMode = NONE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader [STATIC]
//--------------------------------------------------------------------------------------
float4 ShadowMapVS(float3 position:POSITION):SV_POSITION
{
	//TODO: return the position of the vertex in correct space (hint: seen from the view of the light)
	float4 worldPosition = mul(float4(position, 1.0f), gWorld);
    float4 lightSpacePosition = mul(worldPosition, gLightViewProj);
    return lightSpacePosition;
}

//--------------------------------------------------------------------------------------
// Vertex Shader [SKINNED]
//--------------------------------------------------------------------------------------
float4 ShadowMapVS_Skinned(float3 position:POSITION, float4 BoneIndices : BLENDINDICES, float4 BoneWeights : BLENDWEIGHTS) : SV_POSITION
{
	//TODO: return the position of the ANIMATED vertex in correct space (hint: seen from the view of the light)
	float4x4 BoneMatrix0 = gBones[BoneIndices.x];
    float4x4 BoneMatrix1 = gBones[BoneIndices.y];
    float4x4 BoneMatrix2 = gBones[BoneIndices.z];
    float4x4 BoneMatrix3 = gBones[BoneIndices.w];

    float4 skinnedPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    skinnedPosition += mul(float4(position, 1.0f), BoneMatrix0) * BoneWeights.x;
    skinnedPosition += mul(float4(position, 1.0f), BoneMatrix1) * BoneWeights.y;
    skinnedPosition += mul(float4(position, 1.0f), BoneMatrix2) * BoneWeights.z;
    skinnedPosition += mul(float4(position, 1.0f), BoneMatrix3) * BoneWeights.w;

    float4 worldPosition = mul(skinnedPosition, gWorld);
    float4 lightSpacePosition = mul(worldPosition, gLightViewProj);

    return lightSpacePosition;
}
 
//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
void ShadowMapPS_VOID(float4 position:SV_POSITION){}

technique11 GenerateShadows
{
	pass P0
	{
		SetRasterizerState(rasterizerState);
	    SetDepthStencilState(depthStencilState, 0);
		SetVertexShader(CompileShader(vs_4_0, ShadowMapVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapPS_VOID()));
	}
}

technique11 GenerateShadows_Skinned
{
	pass P0
	{
		SetRasterizerState(rasterizerState);
		SetDepthStencilState(depthStencilState, 0);
		SetVertexShader(CompileShader(vs_4_0, ShadowMapVS_Skinned()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapPS_VOID()));
	}
}