//You could just use #pragma pack_matrix(row_major) in your shader instead of tranposing matrices on the cpu side

struct VertexInput
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

VertexOutput main( VertexInput input )
{

	VertexOutput output;

	input.position.w = 1.0f;

	output.position = input.position ;

	output.tex = input.tex;
	output.normal = input.normal;

	return output;
}