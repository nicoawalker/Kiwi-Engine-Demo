#include "TerrainShader.h"

#include <KiwiCore.h>
#include <KiwiGraphics.h>

TerrainShader::TerrainShader( std::wstring shaderName, Kiwi::Renderer* renderer ) :
	Kiwi::IShader( shaderName, renderer, L"Data\\Shaders\\TerrainVertexShader.cso", L"Data\\Shaders\\TerrainPixelShader.cso" )
{

	try
	{
		D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		m_inputLayout = renderer->CreateInputLayout( this, layoutDesc, 4 );

		//create the vertex object buffer
		D3D11_BUFFER_DESC vobDesc;
		vobDesc.Usage = D3D11_USAGE_DEFAULT;
		vobDesc.ByteWidth = sizeof( Vertex_ObjectBuffer );
		vobDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vobDesc.CPUAccessFlags = 0;
		vobDesc.MiscFlags = 0;
		vobDesc.StructureByteStride = 0;

		m_vBuffers.push_back(renderer->CreateBuffer( &vobDesc, 0 ));

		//create the pixel object buffer
		D3D11_BUFFER_DESC pobDesc;
		pobDesc.Usage = D3D11_USAGE_DEFAULT;
		pobDesc.ByteWidth = sizeof( Pixel_ObjectBuffer );
		pobDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		pobDesc.CPUAccessFlags = 0;
		pobDesc.MiscFlags = 0;
		pobDesc.StructureByteStride = 0;

		m_pBuffers.push_back( renderer->CreateBuffer( &pobDesc, 0 ) );

		//create the pixel frame buffer
		D3D11_BUFFER_DESC pfbDesc;
		pfbDesc.Usage = D3D11_USAGE_DEFAULT;
		pfbDesc.ByteWidth = sizeof( Pixel_FrameBuffer );
		pfbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		pfbDesc.CPUAccessFlags = 0;
		pfbDesc.MiscFlags = 0;
		pfbDesc.StructureByteStride = 0;

		m_pBuffers.push_back( renderer->CreateBuffer( &pfbDesc, 0 ) );

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

TerrainShader::~TerrainShader()
{



}

void TerrainShader::SetFrameParameters( Kiwi::Scene* scene )
{

	Kiwi::Renderer* renderer = scene->GetRenderer();
	assert( renderer != 0 );

	Pixel_FrameBuffer* pfbPtr = 0;

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 direction;

	Kiwi::Vector4ToXMFLOAT4( scene->GetAmbientLight(), ambient );
	Kiwi::Vector4ToXMFLOAT4( scene->GetDiffuseLight(), diffuse );
	Kiwi::Vector4ToXMFLOAT4( scene->GetDiffuseLightDirection(), direction );

	// lock the pixel frame buffer so that it can be written to
	ID3D11Buffer* pfbBuffer = m_pBuffers[1];
	if( pfbBuffer )
	{
		Pixel_FrameBuffer pixelBuffer = { ambient,
			diffuse,
			direction };

		renderer->UpdateSubresource( pfbBuffer,
									 0,
									 NULL,
									 &pixelBuffer,
									 0,
									 0 );
	}

}

void TerrainShader::SetObjectParameters( Kiwi::Scene* scene, Kiwi::RenderTarget* renderTarget, Kiwi::Mesh::Submesh* meshSubset )
{

	assert( scene != 0 );
	assert( renderTarget != 0 );
	assert( meshSubset != 0 );
	assert( meshSubset->parent );

	if( scene == 0 || renderTarget == 0 || meshSubset == 0 || meshSubset->parent == 0 )
	{
		return;
	}

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
	Kiwi::Matrix4ToXMMATRIX( camera->GetViewMatrix(), view );
	Kiwi::Matrix4ToXMMATRIX( camera->GetProjectionMatrix(), projection );

	//set the worldViewProject matrix
	DirectX::XMMATRIX wvp = world * view * projection;

	// lock the vertex object buffer so that it can be written to
	ID3D11Buffer* vobBuffer = m_vBuffers[0];
	if( vobBuffer )
	{

		Vertex_ObjectBuffer vertexBuffer = { wvp,
			world };

		renderer->UpdateSubresource( vobBuffer,
									 0,
									 NULL,
									 &vertexBuffer,
									 0,
									 0 );

	} else
	{
		throw Kiwi::Exception( L"DefaultShader::SetEntityParameters", L"[" + m_shaderName + L"] The vertex object buffer is null" );
	}

	Kiwi::Color kDiffuseColor = meshSubset->material.GetColor( L"Diffuse" );
	DirectX::XMFLOAT4 diffuseColor( (float)kDiffuseColor.red, (float)kDiffuseColor.green, (float)kDiffuseColor.blue, (float)kDiffuseColor.alpha );
	DirectX::XMFLOAT4 bool1( 0.0f, 0.0f, 0.0f, 0.0f );

	if( meshSubset->parent->UsingPerVertexColor() == true )
	{
		bool1.y = 1.0f;
	}

	static unsigned long lastTextureID = 0; //stores the ID of the last texture so we only switch textures when a new one needs to be set

	/*if( meshSubset->material.IsTextured() )
	{
		Kiwi::Texture* matTexture = meshSubset->material.GetTexture( L"Diffuse" );
		if( matTexture && matTexture->GetShaderResourceView() )
		{
			ID3D11ShaderResourceView* matSRV = matTexture->GetShaderResourceView();
			if( matSRV )
			{
				renderer->SetPixelShaderResources( 0, 1, &matSRV );
				isTextured.x = 1.0f;
			}
		}
	}*/

	ID3D11Buffer* pobBuffer = m_pBuffers[0];
	if( pobBuffer )
	{

		Pixel_ObjectBuffer pixelBuffer = { diffuseColor,
											bool1 };

		renderer->UpdateSubresource( pobBuffer,
									0,
									NULL,
									&pixelBuffer,
									0,
									0 );

	} else
	{
		throw Kiwi::Exception( L"DefaultShader::SetEntityParameters", L"[" + m_shaderName + L"] The pixel object buffer is null" );
	}

}