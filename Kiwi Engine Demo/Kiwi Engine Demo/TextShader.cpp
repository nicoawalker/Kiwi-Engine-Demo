#include "TextShader.h"

#include <KiwiCore.h>
#include <KiwiGraphics.h>

TextShader::TextShader( Kiwi::Renderer* renderer ) :
	Kiwi::IShader( L"TextShader", renderer, L"Data\\Shaders\\TextVS.cso", L"Data\\Shaders\\TextPS.cso" )
{

	try
	{
		D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		m_inputLayout = renderer->CreateInputLayout( this, layoutDesc, 4 );

		//create the vertex object buffer
		D3D11_BUFFER_DESC vobDesc;
		vobDesc.Usage = D3D11_USAGE_DEFAULT; // dynamic as it will be updated each frame
		vobDesc.ByteWidth = sizeof( Vertex_ObjectBuffer );
		vobDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vobDesc.CPUAccessFlags = 0; // allow the cpu to write data to the buffer
		vobDesc.MiscFlags = 0;
		vobDesc.StructureByteStride = 0;

		m_vBuffers.push_back( renderer->CreateBuffer( &vobDesc, 0 ) );

		D3D11_SAMPLER_DESC sDesc;
		//filter that decides which pixels to use to display the texture
		sDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		//any texture coordinates greater than 1.0f wrap around
		sDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.MipLODBias = 0.0f;
		sDesc.MaxAnisotropy = 1;
		sDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sDesc.BorderColor[0] = 0;
		sDesc.BorderColor[1] = 0;
		sDesc.BorderColor[2] = 0;
		sDesc.BorderColor[3] = 0;
		sDesc.MinLOD = 0;
		sDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_samplerStates.push_back( renderer->CreateSampler( &sDesc ) );

	} catch( ... )
	{
		throw;
	}

}

void TextShader::SetObjectParameters( Kiwi::Scene* scene, Kiwi::RenderTarget* renderTarget, Kiwi::Mesh::Submesh* meshSubset )
{

	assert( scene != 0 );
	assert( renderTarget != 0 );

	Kiwi::Renderer* renderer = scene->GetRenderer();
	Kiwi::Camera* camera = renderTarget->GetViewport( 0 )->GetCamera();
	Kiwi::Entity* entity = meshSubset->parent->GetEntity();
	if( entity == 0 )
	{
		return;
	}
	Kiwi::Transform* transform = entity->FindComponent<Kiwi::Transform>();
	if( transform == 0 || camera == 0 || renderer == 0 )
	{
		return;
	}

	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	Kiwi::Matrix4ToXMMATRIX( transform->GetWorldMatrix(), world );
	Kiwi::Matrix4ToXMMATRIX( camera->GetViewMatrix2D(), view );
	Kiwi::Matrix4ToXMMATRIX( camera->GetOrthoMatrix(), projection );

	//set the worldViewProject matrix
	DirectX::XMMATRIX wvp = world * view * projection;

	// lock the vertex object buffer so that it can be written to
	ID3D11Buffer* vobBuffer = m_vBuffers[0];
	if( vobBuffer )
	{

		Vertex_ObjectBuffer vertexBuffer = { wvp };

		renderer->UpdateSubresource( vobBuffer,
									 0,
									 NULL,
									 &vertexBuffer,
									 0,
									 0 );

	} else
	{
		throw Kiwi::Exception( L"Default2DShader::SetEntityParameters", L"[" + m_shaderName + L"] The vertex object buffer is null" );
	}

	//MessageBox( NULL, Kiwi::ToWString(meshSubset->endIndex).c_str(), L"A", MB_OK );

	static long lastTextureID = -1; //stores the ID of the last texture so we only switch textures when a new one needs to be set
	if( meshSubset->material.IsTextured() )
	{
		Kiwi::Texture* matTexture = meshSubset->material.GetTexture( L"Diffuse" );
		if( matTexture )
		{
			ID3D11ShaderResourceView* matSRV = matTexture->GetShaderResourceView();
			if( matSRV )
			{
				renderer->SetPixelShaderResources( 0, 1, &matSRV );
				lastTextureID = matTexture->GetAssetID();
			}
		}
	}

}