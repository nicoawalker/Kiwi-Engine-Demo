Texture2D shaderTexture; //texture resource to be used

SamplerState sampleType; //sampler to apply effects to the texture resource

cbuffer FrameBuffer
{
	float4 ambientLight;
	float4 diffuseLight;
	float4 diffuseDirection;
};

cbuffer ObjectBuffer
{
	float4 diffuseColor;
	float4 usingTexture; //x-value is used as bool to check if a texture is being used
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main( PixelInputType input ) : SV_TARGET
{

	return float4( 1.0f, 1.0f, 1.0f, 1.0f );

	//float4 textureColor;
	//float3 lightDir;
	//float lightIntensity;
	//float4 color;

	////set the ambient color for all pixels first
	//color = ambientLight;

	////The light intensity value is calculated as the dot product between the normal vector of triangle and 
	////the light direction vector
	//lightDir = -diffuseDirection;

	////calculate the amount of light on this pixel
	//lightIntensity = saturate( dot( input.normal, lightDir ) );

	////Check if the N dot L is greater than zero. 
	////If it is then add the diffuse color to the ambient color. 
	//if( lightIntensity > 0.0f )
	//{
	//	color += (diffuseLight * lightIntensity);
	//}

	////combine diffuse color with diffuse light
	//color = saturate( color );

	////if( usingTexture == true )
	////{
	////	//sample pixel color using the sampler
	////	textureColor = shaderTexture.Sample( sampleType, input.tex );
	////	//combine color and texture color
	////	color = color * textureColor;
	////}

	////combine color and diffuse pixel color
	//color = color * diffuseColor;

	//return color;
}