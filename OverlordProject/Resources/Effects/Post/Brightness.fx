float brightnessFactor = 1.5; // Adjust this factor to control brightness

//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState EnableDepthWriting
{
    DepthEnable = true;
    DepthWriteMask = true;
};

/// Create Rasterizer State (Backface culling) 
RasterizerState BackfaceCulling
{
    CullMode = BACK;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD1;
};

//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    // Set the Position
    output.Position = float4(input.Position, 1);
    // Set the TexCoord
    output.TexCoord = input.TexCoord;

    return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
    // Step 1: sample the texture
    float4 color = gTexture.Sample(samPoint ,input.TexCoord);
    // Step 2: multiply with brightnessfactor
    color.rgb *= brightnessFactor;
    // Step 3: return the color
    return color;
}

//TECHNIQUE
//---------
technique11 Brightness
{
    pass P0
    {
        SetRasterizerState(BackfaceCulling);
        SetDepthStencilState(EnableDepthWriting, 0);

        // Set states...
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }
}

