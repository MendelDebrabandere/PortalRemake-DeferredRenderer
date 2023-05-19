struct Light
{
	float4 Direction; //16B
	float4 Position; //16B
	float4 Color; //16B
	float Intensity; //4B
	float Range; //4B
	float SpotLightAngle; //4B
	int Type; //4B

	//4* 16B
};

struct Material
{
	float3 Diffuse;
	float3 Specular;
	float Shininess;
};

struct LightingResult
{
	float3 Diffuse;
	float3 Specular;
};

//CONVERSION
//**********
float3 DepthToWorldPosition(float depth, float2 screenCoordinate, float2 screenDimension, float4x4 matrixViewProjectionInverse)
{
	float2 normalizedScreenCoord = screenCoordinate / screenDimension;
	float2 ndcXY = float2(normalizedScreenCoord.x, 1.0f - normalizedScreenCoord.y) * 2.0f - 1.0f;
	float4 ndc = float4(ndcXY, depth, 1.0f);
	float4 posW = mul(ndc, matrixViewProjectionInverse);
	return (posW / posW.w).xyz;
}

float3 DepthToWorldPosition_QUAD(float depth, float2 texCoords, float4x4 matrixViewProjectionInverse)
{
	float2 ndcXY = float2(texCoords.x, 1.0f - texCoords.y) * 2.0f - 1.0f;
	float4 ndc = float4(ndcXY, depth, 1.0f);
	float4 posW = mul(ndc, matrixViewProjectionInverse);
	return (posW / posW.w).xyz;
}


//LIGHTING CALCULATIONS
//*********************
float3 DoDiffuse(Light light, float3 L, float3 N)
{
	//Lambert Diffuse
	float diffuseStrength = dot(N, -L);
	diffuseStrength = saturate(diffuseStrength);

	////Half-Lambert diffuse
	//diffuseStrength = diffuseStrength * 0.5 + 0.5;
	//diffuseStrength = saturate(diffuseStrength);

	return light.Color.rgb * diffuseStrength;
}

float3 DoSpecular(Light light, Material mat, float3 V, float3 L, float3 N)
{
	//Phong Specular
	float3 reflectedVector = reflect(-L, N);
	float specularStrength = dot(V, reflectedVector);
	specularStrength = saturate(specularStrength);
	specularStrength = pow(specularStrength, mat.Shininess);

	return light.Color.rgb * specularStrength;
}

float DoAttenuation(Light light, float distance)
{
	return 1.0f - smoothstep(light.Range * 0.75f, light.Range, distance);
}

float DoSpotCone(Light light, float3 L)
{
	//cos(90 degrees) > 0
	//cos(0 degrees) > 1

	// If the cosine angle of the light's direction 
	// vector and the vector from the light source to the point being 
	// shaded is less than minCos, then the spotlight contribution will be 0.
	float minCos = cos(radians(light.SpotLightAngle));
	// If the cosine angle of the light's direction vector
	// and the vector from the light source to the point being shaded
	// is greater than maxCos, then the spotlight contribution will be 1.
	float maxCos = lerp(minCos, 1, 0.5f);
	float cosAngle = dot(light.Direction.xyz, L);
	// Blend between the maxixmum and minimum cosine angles.
	return smoothstep(minCos, maxCos, cosAngle);
}

//Directional
LightingResult DoDirectionalLighting(Light light, Material mat, float3 L, float3 V, float3 N)
{
	LightingResult result;

	result.Diffuse = DoDiffuse(light, L, N) * light.Intensity;
	result.Specular = DoSpecular(light, mat, L, V, N) * light.Intensity;

	return result;
}

//PointLight
LightingResult DoPointLighting(Light light, Material mat, float3 V, float3 N, float3 P)
{
	LightingResult result;

	//PointLight Shading Logic
	//....

	return result;
}

//SpotLight
LightingResult DoSpotLighting(Light light, Material mat, float3 V, float3 N, float3 P)
{
	LightingResult result;

	//SpotLight Shading Logic
	//...

	return result;
}