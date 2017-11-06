#ifndef _TERRAINSHADER_H_
#define _TERRAINSHADER_H_

#include <KiwiGraphics.h>
#include <KiwiCore.h>

class TerrainShader :
	public Kiwi::IShader
{
protected:

	//matches the "ObjectBuffer" constant buffer in the vertex shader (per object data)
	struct Vertex_ObjectBuffer
	{
		DirectX::XMMATRIX wvp;
		DirectX::XMMATRIX world;
	};

	//matches the "ObjectBuffer" constant buffer in the pixel shader (per object data)
	struct Pixel_ObjectBuffer
	{
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT4 bool1; //x coodinate stores whether to use a texture, y coordinate stores whether to use per-vertex color
	};

	//matches the "FrameBuffer" constant buffer in the pixel shader (per frame data)
	struct Pixel_FrameBuffer
	{
		DirectX::XMFLOAT4 ambientLight;
		DirectX::XMFLOAT4 diffuseLight;
		DirectX::XMFLOAT4 lightDirection;
	};

public:

	TerrainShader( std::wstring shaderName, Kiwi::Renderer* renderer );
	~TerrainShader();

	void SetFrameParameters( Kiwi::Scene* scene );
	void SetObjectParameters( Kiwi::Scene* scene, Kiwi::RenderTarget* renderTarget, Kiwi::Mesh::Submesh* submesh );

};

#endif