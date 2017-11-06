
struct VertexOutput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main( VertexOutput input ) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}