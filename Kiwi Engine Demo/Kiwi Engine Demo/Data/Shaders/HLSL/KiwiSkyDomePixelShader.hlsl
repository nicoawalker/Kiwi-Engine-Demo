//sample state
SamplerState sampleType; //sampler to apply effects to the texture resource

//constant buffers
cbuffer GradientBuffer
{
	float4 apexColor;
	float4 centerColor;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 domePosition : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{

	float height;
	float4 outputColor;

	height = input.domePosition.y;

	if(height < 0.0f)
	{
		height = 0.0f;
	}

	height += 0.2f;
	saturate(height);

	outputColor = lerp(centerColor, apexColor, height);

	return outputColor;

}