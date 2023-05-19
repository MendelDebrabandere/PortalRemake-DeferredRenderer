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

float threshold = 0.6f;

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
    output.Position = float4(input.Position,1);
	// Set the TexCoord
    output.TexCoord = input.TexCoord;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
    // Apply the bright pass filter.
    float4 color = gTexture.Sample(samPoint, input.TexCoord);
    // Calc brightness by accounting for human eye
    float brightness = dot(color.rgb, float3(0.299, 0.587, 0.114));
    
    // If the color is below the threshold, discard it.
    if(brightness < threshold)
    {
        return color;
    }
    
	// find the dimensions of the texture (the texture has a method for that)	
	int2 textureSize;
	gTexture.GetDimensions(textureSize.x, textureSize.y);

	// calculate dx and dy (UV space for 1 pixel)	
	float2 dxdy = 1.0f / float2(textureSize);

	// Create a double for loop (5 iterations each)
	color = float4(0,0,0,0);
	for (int i = -2; i < 3 ; ++i)
	{
		for (int j = -2; j < 3; ++j)
		{
			color += gTexture.Sample(samPoint, float2(input.TexCoord.x + i * dxdy.x, input.TexCoord.y + j * dxdy.y ));
		}
	}

	// Divide the final color by the number of passes (in this case 5*5)	
	color /= 25;

	// return the final color
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
