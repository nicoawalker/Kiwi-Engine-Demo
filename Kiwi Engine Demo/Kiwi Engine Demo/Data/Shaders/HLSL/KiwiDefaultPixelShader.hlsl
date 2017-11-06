Texture2D shaderTexture; //texture resource to be used

SamplerState sampleType; //sampler to apply effects to the texture resource

cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseLightColor;
	float4 diffusePixelColor;
	float3 lightDirection;
	bool usingTexture;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main(PixelInputType input) : SV_TARGET
{

	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;

	//set the ambient color for all pixels first
	color = ambientColor;

	//The light intensity value is calculated as the dot product between the normal vector of triangle and 
	//the light direction vector
	lightDir = -lightDirection;

	//calculate the amount of light on this pixel
	lightIntensity = saturate(dot(input.normal, lightDir));

	//Check if the N dot L is greater than zero. 
	//If it is then add the diffuse color to the ambient color. 
	if(lightIntensity > 0.0f)
	{
		color += (diffuseLightColor * lightIntensity);
	}

	//combine diffuse color with diffuse light
	color = saturate(color);

	if(usingTexture == true)
	{
		//sample pixel color using the sampler
		textureColor = shaderTexture.Sample(sampleType, input.tex);
		//combine color and texture color
		color = color * textureColor;
	}
	if(usingTexture == false)
	{
		//combine color and diffuse pixel color
		color = color * diffusePixelColor;
	}

	return color;
}