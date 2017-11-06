#pragma pack_matrix(row_major) //shader will convert matrices into row major automatically, no need to transpose them first

Texture2D shaderTexture; //texture resource to be used
SamplerState sampleType; //sampler to apply effects to the texture resource

cbuffer ObjectBuffer : register(b0)
{
	float4 diffuseColor;
	float4 isTextured;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

float4 main( PixelInputType input ) : SV_TARGET
{

	float4 color;

	if( isTextured.x == 1.0f )
	{
		//sample pixel color using the sampler
		color = shaderTexture.Sample( sampleType, input.tex );
	} else
	{
		color = diffuseColor;
	}

	return color;
}