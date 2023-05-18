//************
// VARIABLES *
//************
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;
float4x4 gWorld : WORLD;
float3 gLightDirection : DIRECTION = float3(0.577f, -0.577f, 0.577f);

float4x4 gOtherPortalWorldViewProj;
float4x4 gOtherPortalWorld;


float3 gColorDiffuse : COLOR;

RasterizerState FrontCulling 
{ 
	CullMode = NONE; 
};

//**********
// STRUCTS *
//**********
struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	//output.pos = mul (mul ( float4(input.pos,1.0f), gWorldViewProj ),  gOtherPortalWorldViewProj);
	//output.normal = normalize( mul( mul(input.normal, (float3x3)gWorld), (float3x3)gOtherPortalWorld));
	output.pos = mul ( float4(input.pos,1.0f), gWorldViewProj );
	output.normal = normalize( mul(input.normal, (float3x3)gWorld));
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{	
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	float3 color_rgb = gColorDiffuse * diffuseStrength;

	return float4( gColorDiffuse , 1 );
}

// Default Technique
technique11 Default {
	pass p0 {
		SetRasterizerState(FrontCulling);
		SetVertexShader(CompileShader(vs_4_0,VS()));
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}
