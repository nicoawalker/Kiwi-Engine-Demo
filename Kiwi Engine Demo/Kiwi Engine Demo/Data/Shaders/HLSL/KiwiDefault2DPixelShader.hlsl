Texture2D shaderTexture; //texture resource to be used

SamplerState sampleType; //sampler to apply effects to the texture resource

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main(PixelInputType input) : SV_TARGET
{

	float4 textureColor;
	float4 color;

	//set the ambient color for all pixels first
	//color = ambientColor;

	//sample pixel color using the sampler
	textureColor = shaderTexture.Sample(sampleType, input.tex);

	//The light intensity value is calculated as the dot product between the normal vector of triangle and 
	//the light direction vector
	//lightDir = -lightDirection;

	//calculate the amount of light on this pixel
	//lightIntensity = saturate(dot(input.normal, lightDir));

	//Check if the N dot L is greater than zero. 
	//If it is then add the diffuse color to the ambient color. 
	//if(lightIntensity > 0.0f)
	//{
		//color += (diffuseColor * lightIntensity);
	//}

	//combine diffuse color with diffuse light
	//color = saturate(color);

	//combine color and diffuse color to get the final color
	color = textureColor;

	return color;
}