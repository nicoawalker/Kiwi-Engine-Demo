#ifndef _DEFAULTSHADER_H_
#define _DEFAULTSHADER_H_

#include <KiwiGraphics.h>
#include <KiwiCore.h>

class DefaultShader :
	public Kiwi::IShader
{

friend class DefaultEffect;

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
		DirectX::XMFLOAT4 isTextured;
	};

	//matches the "FrameBuffer" constant buffer in the pixel shader (per frame data)
	struct Pixel_FrameBuffer
	{
		DirectX::XMFLOAT4 ambientLight;
		DirectX::XMFLOAT4 diffuseLight;
		DirectX::XMFLOAT4 lightDirection;
	};

public:

	DefaultShader( std::wstring shaderName, Kiwi::Renderer* renderer );
	~DefaultShader();

	void SetFrameParameters( Kiwi::Scene* scene );
	void SetObjectParameters( Kiwi::Scene* scene, Kiwi::RenderTarget* renderTarget, Kiwi::Mesh::Submesh* submesh );

};

class DefaultEffect :
	public Kiwi::IShaderEffect
{
protected:

	DefaultShader::Vertex_ObjectBuffer* m_vertexObjectBuffer;
	DefaultShader::Pixel_ObjectBuffer* m_pixelObjectBuffer;

	Kiwi::Material* m_material;

public:

	DefaultEffect()
	{

		m_effectShader = L"default";

		m_vertexObjectBuffer = new DefaultShader::Vertex_ObjectBuffer;
		m_pixelObjectBuffer = new DefaultShader::Pixel_ObjectBuffer;

		m_material = 0;

		m_effectBuffers.push_back( new Kiwi::Any( m_vertexObjectBuffer ) ); //add a new FrameBuffer as buffer
		m_effectBuffers.push_back( new Kiwi::Any( m_pixelObjectBuffer ) ); //add a new PixelBuffer as buffer 2

		m_bufferParams[L"World"] = new Kiwi::Any( m_vertexObjectBuffer->world );

		m_bufferParams[L"DiffuseColor"] = new Kiwi::Any( m_pixelObjectBuffer->diffuseColor );
		m_bufferParams[L"IsTextured"] = new Kiwi::Any( m_pixelObjectBuffer->isTextured );
		m_bufferParams[L"Material"] = new Kiwi::Any( m_material );

	}

	template<typename ValueType>
	void SetValue( std::wstring valueName, const ValueType& value )
	{

		auto itr = m_bufferParams.find( valueName );
		if( itr != m_bufferParams.end() )
		{
			*itr->second = value;
		}

	}

	template<typename ValueType>
	const ValueType* GetValue( std::wstring valueName )
	{

		auto itr = m_bufferParams.find( valueName );
		if( itr != m_bufferParams.end() )
		{
			return itr->second->GetPointer<ValueType>();
		}

		return 0;

	}

};

#endif