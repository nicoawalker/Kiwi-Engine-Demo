struct VertexInputType
{
	float4 position : POSITION;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 domePosition : TEXCOORD0;
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

	output.domePosition = input.position;

	return output;
}