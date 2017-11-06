//textures
Texture2D flatTexture : register(t0); //texture for flat surfaces
Texture2D slopeTexture : register(t1); //texture for moderate slopes
Texture2D verticalTexture : register(t2); //texture for sharp, near-vertical slopes

//sample states
SamplerState sampleType; //sampler to apply effects to the texture resource

//constant buffers
cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float numTextures;
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

	float4 flatColor;
	float4 slopeColor;
	float4 verticalColor;
	float slope;
	float blendAmount;

	//determine the slope for this pixel
	slope = 1.0f - input.normal.y;

	//sample the texture colors using the sampler
	//if(numTextures == 1.0f)
	//{
	//	flatColor = flatTexture.Sample(sampleType, input.tex);

	//	textureColor = flatColor;
	//}
	//if(numTextures == 2.0f)
	//{

	//	flatColor = flatTexture.Sample(sampleType, input.tex);
	//	slopeColor = slopeTexture.Sample(sampleType, input.tex);

	//	//determine which texture to apply based on the slope
	//	if(slope < 0.2f)
	//	{
	//		blendAmount = slope / 0.2f;
	//		textureColor = lerp(flatColor, slopeColor, blendAmount);
	//	}
	//	if(slope >= 0.2f)
	//	{
	//		textureColor = slopeColor;
	//	}

	//}
	if(numTextures == 3.0f)
	{

		flatColor = flatTexture.Sample(sampleType, input.tex);
		slopeColor = slopeTexture.Sample(sampleType, input.tex);
		verticalColor = verticalTexture.Sample(sampleType, input.tex);

		//determine which texture to apply based on the slope
		if(slope < 0.2f)
		{
			blendAmount = slope / 0.2f;
			textureColor = lerp(flatColor, slopeColor, blendAmount);
			color = float4(0.0f, 0.8f, 0.0f, 1.0f);
		}

		if(slope < 0.7f && slope >= 0.2f)
		{
			blendAmount = (slope - 0.2f) * 2.0f;
			textureColor = lerp(slopeColor, verticalColor, blendAmount);
			color = float4(0.0f, 0.0f, 0.8f, 1.0f);
		}

		if(slope >= 0.7f)
		{
			textureColor = verticalColor;
			color = float4(0.85f, 0.0f, 0.0f, 1.0f);
		}
	}

	//set the ambient color for all pixels
	//color = ambientColor;

	//The light intensity value is calculated as the dot product between the normal vector of triangle and 
	//the light direction vector
	lightDir = -lightDirection;

	//calculate the amount of light on this pixel
	lightIntensity = saturate(dot(input.normal, lightDir));

	//Check if the N dot L is greater than zero. 
	//If it is then add the diffuse color to the ambient color. 
	if(lightIntensity > 0.0f)
	{
		//color += (diffuseColor * lightIntensity);
	}

	//combine diffuse color with diffuse light
	//color = saturate(color);

	//combine color and diffuse color to get the final color
	//if(numTextures > 0.0f) color = color * textureColor;

	return color;
}