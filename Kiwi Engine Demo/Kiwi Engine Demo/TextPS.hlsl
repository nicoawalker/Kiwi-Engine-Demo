#pragma pack_matrix(row_major) //shader will convert matrices into row major automatically, no need to transpose them first

Texture2D shaderTexture; //texture resource to be used
SamplerState sampleType; //sampler to apply effects to the texture resource

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

float4 main( PixelInputType input ) : SV_TARGET
{

	float4 textureColor;
	float4 difColor = input.color;

	//sample pixel color using the sampler
	textureColor = shaderTexture.Sample( sampleType, input.tex );

	//clip( textureColor.a - 0.001f );

	difColor.r = 1.0f - difColor.r;
	difColor.g = 1.0f - difColor.g;
	difColor.b = 1.0f - difColor.b;
	difColor.a = 1.0f - difColor.a;

	textureColor -= difColor;
	saturate( textureColor );

	return textureColor;
}