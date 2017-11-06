struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

cbuffer MatrixBuffer
{
	matrix worldViewProjection;
	matrix worldMat;
};

PixelInputType main(VertexInputType input)
{

	PixelInputType output;

	input.position.w = 1.0f;

	output.position = mul(input.position, worldViewProjection);

	// Store the input texture for the pixel shader to use.
	output.tex = input.tex;

	//calculate the normal using the world matrix
	output.normal = mul(input.normal, (float3x3)worldMat);

	//normalize the normal vector
	output.normal = normalize(output.normal);

	return output;
}