#ifndef _KIWI_TEXTSHADER_H_
#define _KIWI_TEXTSHADER_H_

#include <KiwiGraphics.h>
#include <KiwiCore.h>

class TextShader :
	public Kiwi::IShader
{

	friend class DefaultEffect;

protected:

	//matches the "ObjectBuffer" constant buffer in the vertex shader (per object data)
	struct Vertex_ObjectBuffer
	{
		DirectX::XMMATRIX wvp;
	};

public:

	TextShader( Kiwi::Renderer* renderer );
	~TextShader() {}

	void SetObjectParameters( Kiwi::Scene* scene, Kiwi::RenderTarget* renderTarget, Kiwi::Mesh::Submesh* submesh );

};

#endif