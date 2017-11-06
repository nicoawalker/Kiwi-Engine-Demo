#pragma pack_matrix(row_major) //shader will convert matrices into row major automatically, no need to transpose them first

cbuffer ObjectBuffer : register(b0) //contains data modified for every entity
{
	float4x4 WVP;
	float4x4 world;
};

struct VertexInput
{
	float4 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal: NORMAL;
	float4 color : COLOR;
};

VertexOutput main( VertexInput input, uint instanceID : SV_InstanceID )
{

	VertexOutput output;

	input.position.w = 1.0f;

	output.position = mul( input.position, WVP );

	output.texCoord = input.texCoord;

	output.normal = mul( input.normal, (float3x3)world );

	output.color = input.color;

	return output;
}