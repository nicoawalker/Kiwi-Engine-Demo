#include "VoxelTerrainChunk.h"

#include <Core\Utilities.h>
#include <Core\Assert.h>
#include <Core\Math.h>

#include <Graphics\Camera.h>

VoxelTerrainChunk::VoxelTerrainChunk( std::wstring name, Kiwi::Scene& scene, unsigned int chunkSize )
{

	m_chunkSize = chunkSize;
	m_totalVoxelCount = 0;
	m_surfaceMeshVoxelCount = 0;
	m_scene = &scene;
	m_opaqueSurfaces = 0;
	m_name = name;
	m_voxelSize = 0.5;
	m_left = m_right = m_top = m_bottom = m_front = m_back = 0;
	m_rebuildMeshes = false;

	//create the voxel mesh
	Kiwi::Mesh* opaqueSurfaceMesh = new Kiwi::Mesh( m_name + L"_O/Mesh" );

	m_opaqueSurfaces = new Kiwi::Entity( m_name + L"_O", *m_scene );

	//Kiwi::Renderable* opaqueRenderable = new Kiwi::Renderable( m_name + L"_O/Renderable", m_opaqueSurfaces, opaqueSurfaceMesh, L"TerrainShader" );

	///m_opaqueSurfaces->SetRenderable( opaqueRenderable );

	//m_opaqueSurfaces->GetTransform()->SetPosition( m_position );
	//m_opaqueSurfaces->SetActive( false );

	m_scene->AddEntity( m_opaqueSurfaces );

}

VoxelTerrainChunk::~VoxelTerrainChunk()
{

	this->Clear();

}

void VoxelTerrainChunk::_LinkVoxel( Voxel* voxel )
{

	//if( !voxel )
	//{
	//	return;
	//}

	//double x = voxel->position.x - m_position.x;
	//double y = voxel->position.y - m_position.y;
	//double z = voxel->position.z - m_position.z;
	//double chunkSideLength = m_chunkSize * m_voxelSize;

	////get pointers to all of the voxels around this one
	//Voxel* left = 0;
	//Voxel* right = 0;
	//Voxel* top = 0;
	//Voxel* bottom = 0;
	//Voxel* front = 0;
	//Voxel* back = 0;
	//
	////for the voxels around the edge of the chunk, try to link them to the voxels in surrounding chunks
	//if( x == 0.0 )
	//{//left side of chunk
	//	if( m_left )
	//	{
	//		left = m_left->FindVoxelAtPosition( Kiwi::Vector3d( chunkSideLength - m_voxelSize, y, z ) );
	//	}
	//	right = this->FindVoxelAtPosition( Kiwi::Vector3d( x + m_voxelSize, y, z ) );

	//} else if( x == chunkSideLength - m_voxelSize )
	//{//right side of chunk
	//	left = this->FindVoxelAtPosition( Kiwi::Vector3d( x - m_voxelSize, y, z ) );
	//	if( m_right )
	//	{
	//		right = m_right->FindVoxelAtPosition( Kiwi::Vector3d( 0.0, y, z ) );
	//	}

	//} else
	//{//neither side
	//	left = this->FindVoxelAtPosition( Kiwi::Vector3d( x - m_voxelSize, y, z ) );
	//	right = this->FindVoxelAtPosition( Kiwi::Vector3d( x + m_voxelSize, y, z ) );
	//}

	//if( y == 0.0 )
	//{//bottom of the chunk
	//	top = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y + m_voxelSize, z ) );
	//	if( m_bottom )
	//	{
	//		bottom = m_bottom->FindVoxelAtPosition( Kiwi::Vector3d( x, chunkSideLength - m_voxelSize, z ) );
	//	}

	//} else if( y == chunkSideLength - m_voxelSize )
	//{//top of the chunk
	//	if( m_top )
	//	{
	//		top = m_top->FindVoxelAtPosition( Kiwi::Vector3d( x, 0.0, z ) );
	//	}
	//	bottom = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y - m_voxelSize, z ) );

	//} else
	//{//neither side
	//	top = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y + m_voxelSize, z ) );
	//	bottom = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y - m_voxelSize, z ) );
	//}

	//if( z == 0.0 )
	//{//back side of the chunk
	//	front = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y, z + m_voxelSize ) );
	//	if( m_back )
	//	{
	//		back = m_back->FindVoxelAtPosition( Kiwi::Vector3d( x, y, chunkSideLength - m_voxelSize) );
	//	}

	//} else if( z == chunkSideLength - m_voxelSize )
	//{//front side of the chunk
	//	if( m_front )
	//	{
	//		front = m_front->FindVoxelAtPosition( Kiwi::Vector3d( x, y, 0.0 ) );
	//	}
	//	back = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y, z - m_voxelSize ) );

	//} else
	//{//neither side
	//	front = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y, z + m_voxelSize ) );
	//	back = this->FindVoxelAtPosition( Kiwi::Vector3d( x, y, z - m_voxelSize ) );
	//}

	////now link this voxel to all of those ones
	//if( left )
	//{
	//	voxel->left = left;
	//	left->right = voxel;
	//}
	//if( right )
	//{
	//	voxel->right = right;
	//	right->left = voxel;
	//}
	//if( front )
	//{
	//	voxel->front = front;
	//	front->back = voxel;
	//}
	//if( back )
	//{
	//	voxel->back = back;
	//	back->front = voxel;
	//}
	//if( top )
	//{
	//	voxel->top = top;
	//	top->bottom = voxel;
	//}
	//if( bottom )
	//{
	//	voxel->bottom = bottom;
	//	bottom->top = voxel;
	//}

}

void VoxelTerrainChunk::RebuildSurfaceMesh()
{

	//if( !m_rebuildMeshes ) return;

	//this->LinkVoxels();

	//std::unordered_set<Voxel*, hashFunc, equalsFunc> transparentVoxels;
	//std::vector<Voxel*> transVoxels;
	//std::vector<Kiwi::Vector3d> surfaceVertices;
	//std::vector<Kiwi::Vector2d> surfaceUVs;
	//std::vector<Kiwi::Vector3d> surfaceNormals;
	//std::vector<Kiwi::Color> surfaceColors;

	///*iterate over every voxel and generate the visible surfaces for the opaque mesh*/
	//auto zItr = m_chunk.zVoxels.begin();
	//for( ; zItr != m_chunk.zVoxels.end(); zItr++ )
	//{
	//	auto xItr = zItr->second.xVoxels.begin();
	//	for( ; xItr != zItr->second.xVoxels.end(); xItr++ )
	//	{
	//		auto yItr = xItr->second.yVoxels.begin();
	//		for( ; yItr != xItr->second.yVoxels.end(); yItr++ )
	//		{
	//			Voxel* voxel = yItr->second;

	//			if( voxel->color.alpha != 1.0 )
	//			{
	//				transVoxels.push_back( voxel );
	//				continue;
	//			}

	//			double xPos = m_voxelSize * voxel->scale.x;
	//			double yPos = m_voxelSize * voxel->scale.y;
	//			double zPos = m_voxelSize * voxel->scale.z;

	//			unsigned int vertexCount = surfaceVertices.size();

	//			double sideColorMod = 0.1;
	//			Kiwi::Color sideColor( voxel->color.red - (voxel->color.red * sideColorMod), voxel->color.green - (voxel->color.green * sideColorMod), voxel->color.blue - (voxel->color.blue * sideColorMod), voxel->color.alpha );

	//			//if there is no voxel to the left, or the voxel is transparent, generate the surface
	//			Kiwi::Vector3d voxelPos = voxel->position - m_position;
	//			if( (voxel->left == 0 || voxel->left->color.alpha != 1.0) )
	//			{
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//			}
	//			if( voxel->right == 0 || voxel->right->color.alpha != 1.0 )
	//			{
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//			}
	//			if( voxel->front == 0 || voxel->front->color.alpha != 1.0 )
	//			{
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//			}
	//			if( voxel->back == 0 || voxel->back->color.alpha != 1.0 )
	//			{
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//				surfaceColors.push_back( sideColor );
	//			}
	//			if( voxel->top == 0 || voxel->top->color.alpha != 1.0 )
	//			{
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//				surfaceVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//				surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//				surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//				surfaceColors.push_back( voxel->color );
	//				surfaceColors.push_back( voxel->color );
	//				surfaceColors.push_back( voxel->color );
	//				surfaceColors.push_back( voxel->color );
	//				surfaceColors.push_back( voxel->color );
	//				surfaceColors.push_back( voxel->color );
	//			}
	//			if( voxel->bottom == 0 || voxel->bottom->color.alpha != 1.0 )
	//			{//no voxel or transparent voxel directly below this one

	//				//if this is the lowest voxel on the lowest chunk, can skip the bottom since there is no way to see it
	//				if( m_position.y == 0.0 )
	//				{
	//					double minHeight = ( m_chunkSize * m_voxelSize );
	//					auto zItr = m_chunk.zVoxels.find( voxelPos.z );
	//					if( zItr != m_chunk.zVoxels.end() )
	//					{
	//						auto xItr = zItr->second.xVoxels.find( voxelPos.x );
	//						if( xItr != zItr->second.xVoxels.end() )
	//						{
	//							auto yItr = xItr->second.yVoxels.begin();
	//							for( ; yItr != xItr->second.yVoxels.end(); yItr++ )
	//							{
	//								minHeight = min( minHeight, yItr->second->position.y );
	//							}
	//						}
	//					}

	//					if( voxel->position.y > minHeight )
	//					{
	//						surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//						surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//						surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//						surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//						surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//						surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//						surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//						surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//						surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//						surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//						surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//						surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//						surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//						surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//						surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//						surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//						surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//						surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//						surfaceColors.push_back( voxel->color );
	//						surfaceColors.push_back( voxel->color );
	//						surfaceColors.push_back( voxel->color );
	//						surfaceColors.push_back( voxel->color );
	//						surfaceColors.push_back( voxel->color );
	//						surfaceColors.push_back( voxel->color );
	//					}

	//				} else
	//				{
	//					surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//					surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//					surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//					surfaceVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//					surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//					surfaceVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//					surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//					surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//					surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//					surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//					surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//					surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//					surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//					surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//					surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//					surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//					surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//					surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//					surfaceColors.push_back( voxel->color );
	//					surfaceColors.push_back( voxel->color );
	//					surfaceColors.push_back( voxel->color );
	//					surfaceColors.push_back( voxel->color );
	//					surfaceColors.push_back( voxel->color );
	//					surfaceColors.push_back( voxel->color );
	//				}
	//			}

	//			if( surfaceVertices.size() > vertexCount )
	//			{
	//				m_surfaceMeshVoxelCount++;
	//			}
	//		}
	//	}
	//}

	//if( surfaceVertices.size() > 0 )
	//{
	//	Kiwi::Mesh* mesh = m_opaqueSurfaces->GetRenderable()->GetMesh();
	//	if( mesh == 0 )
	//	{
	//		mesh = new Kiwi::Mesh( m_name + L"/Mesh", *m_scene->GetRenderer(), surfaceVertices, surfaceUVs, surfaceNormals );
	//	} else
	//	{
	//		mesh->Clear();
	//		mesh->SetVertices( surfaceVertices );
	//		mesh->SetNormals( surfaceNormals );
	//		mesh->SetUVs( surfaceUVs );
	//		mesh->SetColors( surfaceColors );
	//		mesh->BuildMesh();
	//	}
	//	m_opaqueSurfaces->SetActive( true );
	//}

	//if( transVoxels.size() > 0 )
	//{
	//	//generate the mesh for the transparent voxels

	//	std::vector<Kiwi::Vector3d> groupVertices;
	//	std::vector<Kiwi::Vector2d> groupUVs;
	//	std::vector<Kiwi::Vector3d> groupNormals;
	//	std::vector<Kiwi::Color> groupColors;

	//	for( Voxel* voxel : transVoxels )
	//	{
	//		double xPos = 0.5 * voxel->scale.x;
	//		double yPos = 0.5 * voxel->scale.y;
	//		double zPos = 0.5 * voxel->scale.z;

	//		unsigned int vertexCount = groupVertices.size();

	//		//if there is no voxel to the left, or the voxel is transparent, generate the surface
	//		Kiwi::Vector3d voxelPos = voxel->position - m_position;
	//		if( voxel->left == 0 )
	//		{
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//		}
	//		if( voxel->right == 0 )
	//		{
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//		}
	//		if( voxel->front == 0 )
	//		{
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//		}
	//		if( voxel->back == 0 )
	//		{
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//		}
	//		if( voxel->top == 0 )
	//		{
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, yPos, -zPos ) + voxelPos );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//		}
	//		if( voxel->bottom == 0 )
	//		{
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( -xPos, -yPos, zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, -zPos ) + voxelPos );
	//			groupVertices.push_back( Kiwi::Vector3d( xPos, -yPos, zPos ) + voxelPos );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
	//			groupUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//			groupNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//			groupColors.push_back( voxel->color );
	//		}

	//		if( groupVertices.size() > vertexCount )
	//		{
	//			m_surfaceMeshVoxelCount++;
	//		}
	//	}

	//	if( groupVertices.size() > 0 )
	//	{
	//		for( unsigned int i = 0; i < m_transparentSurfaces.size(); i++ )
	//		{
	//			SAFE_DELETE( m_transparentSurfaces[i] );
	//		}
	//		Kiwi::FreeMemory( m_transparentSurfaces );

	//		Kiwi::Mesh::Submesh submesh;
	//		submesh.startIndex = 0;
	//		submesh.endIndex = groupVertices.size() - 1;
	//		submesh.material.SetColor( L"Diffuse", transVoxels[0][0].color );
	//		Kiwi::Mesh* mesh = new Kiwi::Mesh( m_name + L"_T/Mesh", *m_scene->GetRenderer() );
	//		mesh->SetVertices( groupVertices );
	//		mesh->SetNormals( groupNormals );
	//		mesh->SetUVs( groupUVs );
	//		//mesh->SetColors( groupColors );
	//		mesh->AddSubmesh( submesh );
	//		mesh->BuildMesh();

	//		Kiwi::Entity* entity = new Kiwi::Entity( m_name + L"O", *m_scene );

	//		Kiwi::Renderable* transRenderable = new Kiwi::Renderable( m_name + L"/TransparentMesh", entity, mesh, L"TerrainShader" );

	//		entity->SetRenderable( transRenderable );
	//		entity->GetTransform()->SetPosition( m_position );
	//		entity->GetTransform()->Translate( Kiwi::Vector3( 0.0f, -0.15f, 0.0f ) );
	//		entity->SetActive( true );
	//		m_transparentSurfaces.push_back( entity );
	//		m_scene->AddEntity( entity );
	//	}
	//}

	//m_rebuildMeshes = false;

}

void VoxelTerrainChunk::AddVoxel( Voxel* voxel )
{

	//if( !voxel )
	//{
	//	return;
	//}

	////delete any existing voxels at the same position. 
	//this->DestroyVoxelAtPosition( voxel->position - m_position );
	//
	//m_chunk.zVoxels[voxel->position.z - m_position.z].xVoxels[voxel->position.x - m_position.x].yVoxels[voxel->position.y - m_position.y] = voxel;
	//m_totalVoxelCount++;
	//m_rebuildMeshes = true;

}

void VoxelTerrainChunk::LinkChunks( VoxelTerrainChunk* chunk )
{

	if( chunk )
	{
		double chunkSideLength = m_chunkSize * m_voxelSize;
		if( chunk->GetPosition().x == m_position.x + chunkSideLength && chunk->GetPosition().z == m_position.z && chunk->GetPosition().y == m_position.y )
		{
			m_right = chunk;
			chunk->m_left = this;

		} else if( chunk->GetPosition().x == m_position.x - chunkSideLength && chunk->GetPosition().z == m_position.z && chunk->GetPosition().y == m_position.y )
		{
			m_left = chunk;
			chunk->m_right = this;
		}

		if( chunk->GetPosition().y == m_position.y + chunkSideLength && chunk->GetPosition().z == m_position.z && chunk->GetPosition().x == m_position.x )
		{
			m_top = chunk;
			chunk->m_bottom = this;

		} else if( chunk->GetPosition().y == m_position.y - chunkSideLength && chunk->GetPosition().z == m_position.z && chunk->GetPosition().x == m_position.x )
		{
			m_bottom = chunk;
			chunk->m_top = this;
		}

		if( chunk->GetPosition().z == m_position.z + chunkSideLength && chunk->GetPosition().x == m_position.x && chunk->GetPosition().y == m_position.y )
		{
			m_front = chunk;
			chunk->m_back = this;

		} else if( chunk->GetPosition().z == m_position.z - chunkSideLength && chunk->GetPosition().x == m_position.x && chunk->GetPosition().y == m_position.y )
		{
			m_back = chunk;
			chunk->m_front = this;
		}
	}

}

void VoxelTerrainChunk::BuildChunk( std::vector<Voxel*>& voxels )
{

	//this->Clear();

	////first add all of the voxels into the maps
	//for( unsigned int i = 0; i < voxels.size(); i++ )
	//{
	//	if( voxels[i] )
	//	{
	//		m_chunk.zVoxels[voxels[i]->position.z - m_position.z].xVoxels[voxels[i]->position.x - m_position.x].yVoxels[voxels[i]->position.y - m_position.y] = voxels[i];
	//		m_totalVoxelCount++;
	//	}
	//}

	//m_rebuildMeshes = true;

}

void VoxelTerrainChunk::GenerateWater( double heightCutoff )
{

	//convert world height to height within chunk
	//heightCutoff -= m_position.y;

	//if( heightCutoff <= 0.0 )
	//{
	//	//the entire chunk is above the height cutoff so nothing to do here
	//	return;
	//}

	//for( double z = 0; z < ((double)m_chunkSize * m_voxelSize); z += m_voxelSize )
	//{
	//	auto zItr = m_chunk.zVoxels.find( z );
	//	if( zItr != m_chunk.zVoxels.end() )
	//	{
	//		for( double x = 0; x < ((double)m_chunkSize * m_voxelSize); x += m_voxelSize )
	//		{
	//			auto xItr = zItr->second.xVoxels.find( x );
	//			if( xItr != zItr->second.xVoxels.end() )
	//			{
	//				double maxHeight = 0.0;
	//				for( double y = 0.0; y < ((double)m_chunkSize * m_voxelSize); y += m_voxelSize )
	//				{
	//					auto yItr = xItr->second.yVoxels.find( y );
	//					if( yItr != xItr->second.yVoxels.end() )
	//					{
	//						if( yItr->second->position.y > maxHeight )
	//						{
	//							maxHeight = yItr->second->position.y;
	//						}
	//					}
	//				}
	//				
	//				if( maxHeight < m_voxelSize && m_position.y <= m_voxelSize )
	//				{
	//					continue;
	//				}

	//				for( double y = maxHeight + m_voxelSize; y <= heightCutoff; y += m_voxelSize )
	//				{
	//					Voxel* waterVoxel = new Voxel( VoxelType::WATER, Kiwi::Vector3d( x + m_position.x, y + m_position.y, z + m_position.z ) );
	//					m_chunk.zVoxels[z].xVoxels[x].yVoxels[y] = waterVoxel;
	//					m_totalVoxelCount++;
	//				}

	//			} else
	//			{ //there are no voxels along the x axis

	//			}
	//		}
	//	} else
	//	{//there are no voxels along the z axis

	//	}
	//}

	//m_rebuildMeshes = true;

}

void VoxelTerrainChunk::LinkVoxels()
{

	//interate through all of the voxels and link them with their neighbors
	auto zItr = m_chunk.zVoxels.begin();
	for( ; zItr != m_chunk.zVoxels.end(); zItr++ )
	{
		auto xItr = zItr->second.xVoxels.begin();
		for( ; xItr != zItr->second.xVoxels.end(); xItr++ )
		{
			auto yItr = xItr->second.yVoxels.begin();
			for( ; yItr != xItr->second.yVoxels.end(); yItr++ )
			{
				this->_LinkVoxel( yItr->second );
			}
		}
	}

}

void VoxelTerrainChunk::Clear()
{

	auto zItr = m_chunk.zVoxels.begin();
	for( ; zItr != m_chunk.zVoxels.end(); )
	{
		auto xItr = zItr->second.xVoxels.begin();
		for( ; xItr != zItr->second.xVoxels.end();)
		{
			auto yItr = xItr->second.yVoxels.begin();
			for( ; yItr != xItr->second.yVoxels.end(); )
			{
				SAFE_DELETE( yItr->second );
				yItr = xItr->second.yVoxels.erase( yItr );
			}

			xItr = zItr->second.xVoxels.erase( xItr );
		}

		zItr = m_chunk.zVoxels.erase( zItr );
	}

}

void VoxelTerrainChunk::DestroyVoxelAtPosition( const Kiwi::Vector3d& position )
{

	/*auto zItr = m_chunk.zVoxels.find( position.z );
	if( zItr != m_chunk.zVoxels.end() )
	{
		auto xItr = zItr->second.xVoxels.find( position.x );
		if( xItr != zItr->second.xVoxels.end() )
		{
			auto yItr = xItr->second.yVoxels.find( position.y );
			if( yItr != xItr->second.yVoxels.end() )
			{
				Voxel* voxel = yItr->second;
				before deleting the voxel, unlink it from any surrounding voxels
				if( voxel->left )
				{
					voxel->left->right = 0;
				}
				if( voxel->right )
				{
					voxel->right->left = 0;
				}
				if( voxel->top )
				{
					voxel->top->bottom = 0;
				}
				if( voxel->bottom )
				{
					voxel->bottom->top = 0;
				}
				if( voxel->front )
				{
					voxel->front->back = 0;
				}
				if( voxel->back )
				{
					voxel->back->front = 0;
				}
				SAFE_DELETE( voxel );
				xItr->second.yVoxels.erase( yItr );
			}
		}
	}

	m_rebuildMeshes = true;*/

}

Voxel* VoxelTerrainChunk::FindVoxelAtPosition( const Kiwi::Vector3d& position )
{

	auto zItr = m_chunk.zVoxels.find( position.z );
	if( zItr != m_chunk.zVoxels.end() )
	{
		auto xItr = zItr->second.xVoxels.find( position.x );
		if( xItr != zItr->second.xVoxels.end() )
		{
			auto yItr = xItr->second.yVoxels.find( position.y );
			if( yItr != xItr->second.yVoxels.end() )
			{
				return yItr->second;
			}
		}
	}

	return 0;

}

double VoxelTerrainChunk::FindHeightAtPosition( const Kiwi::Vector3d& position )
{

	//auto zItr = m_chunk.zVoxels.find( position.z );
	//if( zItr != m_chunk.zVoxels.end() )
	//{
	//	auto xItr = zItr->second.xVoxels.find( position.x );
	//	if( xItr != zItr->second.xVoxels.end() )
	//	{
	//		//iterate over all of the y-voxels to find the highest one
	//		double maxHeight = 0;

	//		auto yItr = xItr->second.yVoxels.begin();
	//		for( ; yItr != xItr->second.yVoxels.end(); yItr++ )
	//		{
	//			if( yItr->second->position.y > maxHeight && yItr->second->solid )
	//			{
	//				maxHeight = yItr->second->position.y + (m_voxelSize / 2.0);
	//			}
	//		}
	//		
	//		return maxHeight;
	//	}
	//}
	
	return -1;

}

double VoxelTerrainChunk::FindHeightUnderPosition( const Kiwi::Vector3d& position )
{

	//auto zItr = m_chunk.zVoxels.find( position.z );
	//if( zItr != m_chunk.zVoxels.end() )
	//{
	//	auto xItr = zItr->second.xVoxels.find( position.x );
	//	if( xItr != zItr->second.xVoxels.end() )
	//	{
	//		//iterate over all of the y-voxels to find the highest one
	//		double maxHeight = -1;

	//		auto yItr = xItr->second.yVoxels.begin();
	//		for( ; yItr != xItr->second.yVoxels.end(); yItr++ )
	//		{
	//			if( yItr->second->position.y > maxHeight && yItr->second->solid && yItr->second->position.y <= position.y )
	//			{
	//				maxHeight = yItr->second->position.y + (m_voxelSize / 2.0);
	//			}
	//		}

	//		return maxHeight;
	//	}
	//}

	return -1;

}

void VoxelTerrainChunk::SetPosition( const Kiwi::Vector3d& position )
{

	m_position = position;
	if(m_opaqueSurfaces) m_opaqueSurfaces->FindComponent<Kiwi::Transform>()->SetPosition( m_position );
	for( unsigned int i = 0; i < m_transparentSurfaces.size(); i++ )
	{
		m_transparentSurfaces[i]->FindComponent<Kiwi::Transform>()->SetPosition( m_position );
	}

}

long long VoxelTerrainChunk::GetInstanceBufferSize()
{

	/*Kiwi::InstancedMesh<VoxelInstance>* mesh = dynamic_cast<Kiwi::InstancedMesh<VoxelInstance>*>(m_opaqueSurfaces->GetRenderable()->GetMesh());
	if( mesh )
	{
		return (mesh->GetInstanceCount() * sizeof( Voxel ));
	}*/

	return 0;

}