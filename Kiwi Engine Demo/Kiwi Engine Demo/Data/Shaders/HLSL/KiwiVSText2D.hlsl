struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

cbuffer MatrixBuffer
{
	matrix worldMat;
};

PixelInputType main(VertexInputType input)
{

	PixelInputType output;

	input.position.w = 1.0f;

	output.position = mul(input.position, worldMat);

	// Store the input texture for the pixel shader to use.
	output.tex = input.tex;

	return output;
}