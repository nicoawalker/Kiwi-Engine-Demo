Texture2D shaderTexture; //texture resource to be used

SamplerState sampleType; //sampler to apply effects to the texture resource

cbuffer PixelBuffer
{
	float4 fontColor;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{

	float4 color;

	//sample pixel color using the sampler
	color = shaderTexture.Sample(sampleType, input.tex);

	//if the sum of all the color channels is 0, the background is black
	float bgColor = color.r + color.g + color.b;

	if(bgColor == 0.0f)
	{
		//remove black background
		color.a = 0.0f;
		
	}else
	{
		color.a = 1.0f;
		color = color * fontColor;
	}

	return color;
}