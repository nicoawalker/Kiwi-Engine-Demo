#ifndef _KIWI_DEFAULT2DSHADER_H_
#define _KIWI_DEFAULT2DSHADER_H_

#include <KiwiGraphics.h>
#include <KiwiCore.h>

class Default2DShader :
	public Kiwi::IShader
{

	friend class DefaultEffect;

protected:

	//matches the "ObjectBuffer" constant buffer in the vertex shader (per object data)
	struct Vertex_ObjectBuffer
	{
		DirectX::XMMATRIX wvp;
	};

	//matches the "ObjectBuffer" constant buffer in the pixel shader (per object data)
	struct Pixel_ObjectBuffer
	{
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT4 isTextured;
	};

public:

	Default2DShader( Kiwi::Renderer* renderer );
	~Default2DShader() {}

	void SetObjectParameters( Kiwi::Scene* scene, Kiwi::RenderTarget* renderTarget, Kiwi::Mesh::Submesh* submesh );

};

#endif