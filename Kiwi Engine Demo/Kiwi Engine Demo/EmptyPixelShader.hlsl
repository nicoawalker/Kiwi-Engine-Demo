#pragma pack_matrix(row_major) //shader will convert matrices into row major automatically, no need to transpose them first

Texture2D ObjTexture;
SamplerState ObjSamplerState;

cbuffer ObjectBuffer : register(b0)
{
	float4 diffuseColor;
	float4 isTextured;
};

cbuffer FrameBuffer : register(b1)
{
	float4 ambientLight;
	float4 diffuseLight;
	float4 lightDirection;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

float4 main( VertexOutput input ) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;

	//set the ambient color for all pixels first
	color = ambientLight;

	//The light intensity value is calculated as the dot product between the normal vector of triangle and 
	//the light direction vector
	lightDir = (float3)-lightDirection;

	//calculate the amount of light on this pixel
	lightIntensity = saturate(dot(input.normal, lightDir));

	//Check if the N dot L is greater than zero. 
	//If it is then add the diffuse color to the ambient color. 
	if(lightIntensity > 0.0f)
	{
		color += (diffuseLight * lightIntensity);
	}

	//combine diffuse color with diffuse light
	color = saturate(color);

	if( isTextured.x == 1.0f )
	{
		//combine color and texture color to get final color
		textureColor = ObjTexture.Sample( ObjSamplerState, input.texCoord );
		color = color * textureColor;
		//color.a = textureColor.a;

	} else
	{
		//combine color and diffuse color to get the final color
		color = color * diffuseColor;
		color.a = diffuseColor.a;
	}

	clip( color.a - 0.001f );

	return color;

}