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
    DepthEnable = true;		// enable depth testing
    DepthWriteMask = true;	// disable depth writing
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
	output.Position = float4(input.Position, 1.0f);
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
	// Step 1: find the dimensions of the texture (the texture has a method for that)
	int2 textureSize;
	gTexture.GetDimensions(textureSize.x, textureSize.y);

	// Step 2: calculate dx and dy (UV space for 1 pixel)	
	float2 d = 1.0f / (float2)textureSize;

    float bloomThreshold = 0.6f; // Threshold for determining which pixels contribute to bloom
    float bloomIntensity = 2.f; // Intensity of the bloom effect

	// Step 3: Create a double for loop (5 iterations each)
	float4 color = gTexture.Sample(samPoint, input.TexCoord);
    int amountOfPixelsBloomed = 0;
	for (int i = -2; i < 3; ++i)
	{
		for (int j = -2; j < 3; ++j)
		{
            if (i == 0 && j == 0)
                continue;

            float2 offset = float2(i * 2.0 * d.x, j * 2.0 * d.y);
            float4 sampledColor = gTexture.Sample(samPoint, input.TexCoord + offset);

            // Calculate brightness
            float brightness = dot(sampledColor.rgb, float3(0.2126, 0.7152, 0.0722));

            // Apply bloom effect if brightness exceeds the threshold
            if (brightness > bloomThreshold)
            {
                float4 bloomColor = bloomIntensity * sampledColor;  
                color += bloomColor;
                ++amountOfPixelsBloomed;   
            }
        }
	}
    
	// Step 4: Divide the final color by the number of passes (in this case 5*5)	
	color /= 1 + (amountOfPixelsBloomed * bloomIntensity);
	// Step 5: return the final color
	return color;
}


//TECHNIQUE
//---------
technique11 Bloom
{
    pass P0
    {
		// Set states...
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}