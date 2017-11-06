#include "VoxelTerrain.h"
#include "VoxelChunk.h"
#include "City.h"

#include <Core\Assert.h>
#include <Core\Utilities.h>
#include <Core\Exception.h>
#include <Core\ThreadManager.h>

#include <Utilities\PerlinNoiseGenerator.h>

VoxelTerrain::VoxelTerrain( Kiwi::Scene* scene, short chunkSize, unsigned int loadChunkDistance )
{

	assert( scene != 0 );

	m_scene = scene;
	m_chunkSize = chunkSize;
	m_voxelCount = 0;
	m_voxelSize = 1.0;
	m_seaLevel = 7.0;
	m_surfaceLevel = 64.0;
	//m_renderDistance = 3;
	m_playerChunk = 0;
	m_loadStatus = TERRAIN_UNLOADED;

	m_target = 0;

	m_centerChunk = 0;
	m_renderDistance.Set( 5.0, 1.0, 5.0 );

	m_chunkDimensions.Set( chunkSize, chunkSize, chunkSize );
	m_centerLoadOffset.Set( ((double)chunkSize * m_voxelSize) / 2.0, ((double)chunkSize * m_voxelSize) / 2.0, ((double)chunkSize * m_voxelSize) / 2.0 );

	m_centerPosition.Set( ((m_chunkDimensions.x * m_voxelSize) / 2.0), (m_renderDistance.y + 1.0) * (m_chunkDimensions.y * m_voxelSize) + ((m_chunkDimensions.y * m_voxelSize) / 2.0), ((m_chunkDimensions.z * m_voxelSize) / 2.0) );

	srand( time( NULL ) );
	m_worldSeed = rand() * MAXINT;

	m_startingCity = 0;

	m_doResize = true;

	m_voxelDefinitions[VoxelType::AIR] = new VoxelDefinition( VoxelType::AIR, Kiwi::Color( 1.0, 1.0, 1.0, 0.0 ), 0 );
	m_voxelDefinitions[VoxelType::GRASS] = new VoxelDefinition( VoxelType::GRASS, Kiwi::Color( 1.0 / 255.0, 92.0 / 255.0, 0.0 / 255.0, 1.0 ) );
	m_voxelDefinitions[VoxelType::DIRT] = new VoxelDefinition( VoxelType::DIRT, Kiwi::Color( 59.0 / 255.0, 34.0 / 255, 2.0 / 255.0, 1.0 ), 254, 0, 10 );
	m_voxelDefinitions[VoxelType::WATER] = new VoxelDefinition( VoxelType::WATER, Kiwi::Color( 15.0 / 255.0, 160.0 / 255.0, 202.0 / 255.0, 0.92 ), 0 );
	m_voxelDefinitions[VoxelType::STONE] = new VoxelDefinition( VoxelType::STONE, Kiwi::Color( 114.0 / 255.0, 115.0 / 255.0, 120.0 / 255.0, 1.0 ), 70 );
	m_voxelDefinitions[VoxelType::TERRACOTTA] = new VoxelDefinition( VoxelType::TERRACOTTA, Kiwi::Color( 142.0 / 255.0, 48.0 / 255.0, 20.0 / 255.0, 1.0 ), 50 );
	m_voxelDefinitions[VoxelType::WHITE_PLASTER] = new VoxelDefinition( VoxelType::WHITE_PLASTER, Kiwi::Color( 234.0 / 255.0, 233.0 / 255.0, 228.0 / 255.0, 1.0 ), 30 );
	m_voxelDefinitions[VoxelType::WOOD_WALNUT] = new VoxelDefinition( VoxelType::WOOD_WALNUT, Kiwi::Color( 53.0 / 255.0, 23.0 / 255.0, 0.0 / 255.0, 1.0 ), 30 );
	m_voxelDefinitions[VoxelType::WOOD_OAK] = new VoxelDefinition( VoxelType::WOOD_OAK, Kiwi::Color( 174.0 / 255.0, 131.0 / 255.0, 75.0 / 255.0, 1.0 ), 30 );
	m_voxelDefinitions[VoxelType::BLACK_MARBLE] = new VoxelDefinition( VoxelType::BLACK_MARBLE, Kiwi::Color( 10.0 / 255.0, 11.0 / 255.0, 13.0 / 255.0, 1.0 ), 150 );

}

VoxelTerrain::~VoxelTerrain()
{

	for( auto yItr = m_chunks.begin(); yItr != m_chunks.end(); )
	{
		for( auto xItr = (*yItr).begin(); xItr != (*yItr).end(); )
		{
			for( auto zItr = (*xItr).begin(); zItr != (*xItr).end(); )
			{
				SAFE_DELETE( *zItr );
				zItr = (*xItr).erase( zItr );
			}
			xItr = (*yItr).erase( xItr );
		}
		yItr = m_chunks.erase( yItr );
	}

}

VoxelTerrain::VoxelData* VoxelTerrain::_BuildChunk( VoxelData* vData )
{

	if( vData == 0 || vData->chunk == 0 ) return 0;

	VoxelTerrainChunk* chunk = vData->chunk;

	chunk->BuildChunk( vData->voxels );
	chunk->GenerateWater( m_seaLevel );

	return vData;

}

VoxelTerrainChunk* VoxelTerrain::_GenerateChunkMesh( VoxelData* vData )
{

	if( vData == 0 || vData->chunk == 0 ) return 0;

	VoxelTerrainChunk* chunk = vData->chunk;

	chunk->RebuildSurfaceMesh();

	SAFE_DELETE( vData );

	return chunk;

}

Kiwi::Vector3d _ClampPosition( const Kiwi::Vector3d& position )
{

	return position;

}

VoxelChunk* VoxelTerrain::_FindChunkContainingPosition( const Kiwi::Vector3d& position )
{

	if( m_chunks.size() == 0 || m_chunks[0].size() == 0 || m_chunks[0][0].size() == 0 )
	{
		MessageBox( NULL, L"Chunks size 0", L"VoxelTerrain::_FindChunkContainingPosition", MB_OK );
		return 0;
	}

	VoxelChunk* centerChunk = m_chunks[0][0][0];
	if( centerChunk == 0 )
	{
		MessageBox( NULL, L"Center chunk null", L"VoxelTerrain::_FindChunkContainingPosition", MB_OK );
		return 0;
	}

	//convert global position into position relative to the rear left corner
	double chunkX = position.x - centerChunk->GetPosition().x;
	double chunkY = position.y - centerChunk->GetPosition().y;
	double chunkZ = position.z - centerChunk->GetPosition().z;

	if( chunkX < 0.0 || chunkY < 0.0 || chunkZ < 0.0 )
	{//position lays outside currently loaded map
		return 0;
	}

	long cxi = (long)std::floor( chunkX / (m_chunkDimensions.x * m_voxelSize) );
	long cyi = (long)std::floor( chunkY / (m_chunkDimensions.y * m_voxelSize) );
	long czi = (long)std::floor( chunkZ / (m_chunkDimensions.z * m_voxelSize) );

	if( cxi < 0 || cyi < 0 || czi < 0 || cyi >= m_chunks.size() || cxi >= m_chunks[0].size() || czi >= m_chunks[0][0].size() )
	{//position lays outside currently loaded map
		return 0;
	}

	if( m_chunks[cyi][cxi][czi] != 0 )
	{
		return m_chunks[cyi][cxi][czi];

	} else
	{
		return 0;
	}

}

VoxelChunk* VoxelTerrain::_GetCenterChunk()
{

	if( m_chunks.size() == 0 )
	{
		return 0;
	}

	//get current center chunk position
	unsigned int yIndex = (m_chunks.size() - 1) / 2;
	unsigned int xIndex = (m_chunks[yIndex].size() - 1) / 2;
	unsigned int zIndex = (m_chunks[yIndex][xIndex].size() - 1) / 2;
	return m_chunks[yIndex][xIndex][zIndex];

}

void VoxelTerrain::_GenerateAroundCenter()
{

	if( m_renderDistance.x == 0.0 || m_renderDistance.y == 0.0 || m_renderDistance.z == 0.0 )
	{
		return;
	}

	this->_ClearTerrain();

	int rdx = (int)m_renderDistance.x;
	int rdy = (int)m_renderDistance.y;
	int rdz = (int)m_renderDistance.z;

	int height = (rdy * 2) + 1;
	int width = (rdx * 2) + 1;
	int depth = (rdz * 2) + 1;

	m_startingCity = new City( this, Kiwi::Vector2d( 160.0, 160.0 ), Kiwi::Vector3d( 16.0, m_centerPosition.y + 10.0, 16.0 ) );
	m_startingCity->Generate();

	//position of bottom left corner of center chunk
	Kiwi::Vector3d center = m_centerPosition -Kiwi::Vector3d( ((m_chunkDimensions.x * m_voxelSize) / 2.0), ((m_chunkDimensions.y * m_voxelSize) / 2.0), ((m_chunkDimensions.z * m_voxelSize) / 2.0) );
	int numChunks = 0;
	m_chunks.resize( height );
	for( int y = -rdy; y <= rdy; y++ )
	{
		m_chunks[y + rdy].resize( width );
		for( int x = -rdx; x <= rdx; x++ )
		{
			for( int z = -rdz; z <= rdz; z++ )
			{

				VoxelChunk* newChunk = new VoxelChunk( *this, m_chunkDimensions );
				newChunk->SetPosition( Kiwi::Vector3d( center.x + ((m_chunkDimensions.x * m_voxelSize) * (double)x),
													   center.y + (((m_chunkDimensions.y - m_voxelSize) * m_voxelSize) * (double)y),
														center.z + ((m_chunkDimensions.z * m_voxelSize) * (double)z) ) );
				m_chunks[y + rdy][x + rdx].push_back( newChunk );
				numChunks++;
			}
		}
	}

	m_centerChunk = this->FindChunkContainingPosition( m_centerPosition );

	//start building all chunks
	
	for( auto yItr = m_chunks.begin(); yItr != m_chunks.end(); yItr++ )
	{
		for( auto xItr = (*yItr).begin(); xItr != (*yItr).end(); xItr++ )
		{
			for( auto zItr = (*xItr).begin(); zItr != (*xItr).end(); zItr++ )
			{
				(*zItr)->TriggerFullRebuild();
				
			}
		}
	}

	//block until all chunks have been built
	while( true )
	{
		int builtChunks = 0;
		for( auto yItr = m_chunks.begin(); yItr != m_chunks.end(); yItr++ )
		{
			for( auto xItr = (*yItr).begin(); xItr != (*yItr).end(); xItr++ )
			{
				for( auto zItr = (*xItr).begin(); zItr != (*xItr).end(); zItr++ )
				{
					if( (*zItr)->IsBuilt() == false && (*zItr)->GetStatus() != VoxelChunk::FAILED )
					{
						(*zItr)->FixedUpdate();

					} else
					{
						builtChunks++;
					}
				}
			}
		}

		if( builtChunks >= numChunks )
		{
			break;
		}
	}

}

void VoxelTerrain::_ScrollToCenter()
{

	if( m_renderDistance.x == 0.0 || m_renderDistance.y == 0.0 || m_renderDistance.z == 0.0 )
	{
		return;
	}

	if( m_centerChunk == 0 )
	{
		m_centerChunk = this->_GetCenterChunk();
		if( m_centerChunk == 0 )
		{
			this->_GenerateAroundCenter();
		}
	}

	//change center position to the center of the center chunk, rather than the left bottom corner
	Kiwi::Vector3d centerChunkPos = m_centerChunk->GetPosition() + ((m_chunkDimensions * m_voxelSize) / 2.0);

	Kiwi::Vector3d offset = centerChunkPos - m_centerPosition;

	int numX = std::abs( std::floor( offset.x / (m_chunkDimensions.x * m_voxelSize) ) ); //how many chunks need to be loaded/unloaded
	int numZ = std::abs( std::floor( offset.z / (m_chunkDimensions.z * m_voxelSize) ) ); //how many chunks need to be loaded/unloaded
	int numY = std::abs( std::floor( offset.y / (m_chunkDimensions.y * m_voxelSize) ) ); //how many chunks need to be loaded/unloaded

	if( numX >= (m_renderDistance.x * 2) + 1 ||
		numZ >= (m_renderDistance.z * 2) + 1 ||
		numY >= (m_renderDistance.y * 2) + 1 )
	{//need to regenerate the entire map

		this->_GenerateAroundCenter();
	}

	if( std::fabs( offset.x ) >= (m_chunkDimensions.x * m_voxelSize) )
	{//need to center horizontally
		if( offset.x < 0.0 )
		{//need to add on the right and remove on the left
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				for( unsigned int i = 0; i < numX; i++ )
				{
					std::vector<VoxelChunk*> rightChunks;
					for( int z = 0; z < m_chunks[0][0].size(); z++ )
					{
						VoxelChunk* rightChunk = new VoxelChunk( *this, m_chunkDimensions );
						rightChunk->SetPosition( m_chunks[y].back()[z]->GetPosition() + Kiwi::Vector3d( m_chunkDimensions.x * m_voxelSize, 0.0, 0.0 ) );
						rightChunks.push_back( rightChunk );

						VoxelChunk* leftChunk = m_chunks[y][0][z];
						if( leftChunk != 0 )
						{
							leftChunk->Shutdown();
							SAFE_DELETE( leftChunk );
						}
					}

					m_chunks[y].erase( m_chunks[y].begin() ); //erase chunks on the left

					m_chunks[y].push_back( rightChunks ); //add new chunks on the right
				}
			}

		} else
		{//need to add on the left and remove on the right
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				for( unsigned int i = 0; i < numX; i++ )
				{
					std::vector<VoxelChunk*> leftChunks;
					for( int z = 0; z < m_chunks[0][0].size(); z++ )
					{
						VoxelChunk* leftChunk = new VoxelChunk( *this, m_chunkDimensions );
						leftChunk->SetPosition( m_chunks[y][0][z]->GetPosition() - Kiwi::Vector3d( m_chunkDimensions.x * m_voxelSize, 0.0, 0.0 ) );
						leftChunks.push_back( leftChunk );

						VoxelChunk* rightChunk = m_chunks[y].back()[z];
						if( rightChunk != 0 )
						{
							rightChunk->Shutdown();
							SAFE_DELETE( rightChunk );
						}
					}

					m_chunks[y].erase( m_chunks[y].end() - 1 ); //erase chunks on the left
					m_chunks[y].insert( m_chunks[y].begin(), leftChunks ); //add new chunks on the right
				}
			}
		}
	}

	if( std::fabs( offset.y ) > m_centerLoadOffset.y )
	{//need to center vertically
		if( offset.y < 0.0 )
		{//need to add above and remove below


		} else
		{//need to add below and remove above

		}
	}

	if( std::fabs( offset.z ) > m_centerLoadOffset.z )
	{//need to center depth-wise
		if( offset.z < 0.0 )
		{//need to add in front and remove behind
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				for( int x = 0; x < m_chunks[0].size(); x++ )
				{
					for( unsigned int i = 0; i < numZ; i++ )
					{
						VoxelChunk* frontChunk = new VoxelChunk( *this, m_chunkDimensions );
						frontChunk->SetPosition( m_chunks[y][x].back()->GetPosition() + Kiwi::Vector3d( 0.0, 0.0, m_chunkDimensions.z * m_voxelSize ) );
						m_chunks[y][x].push_back( frontChunk );

						VoxelChunk* backChunk = m_chunks[y][x][0];
						if( backChunk != 0 )
						{
							backChunk->Shutdown();
							SAFE_DELETE( backChunk );
						}
						m_chunks[y][x].erase( m_chunks[y][x].begin() );
					}
				}
			}

		} else
		{//need to add behind and remove in front
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				for( int x = 0; x < m_chunks[0].size(); x++ )
				{
					for( unsigned int i = 0; i < numZ; i++ )
					{
						VoxelChunk* backChunk = new VoxelChunk( *this, m_chunkDimensions );
						backChunk->SetPosition( m_chunks[y][x][0]->GetPosition() - Kiwi::Vector3d( 0.0, 0.0, m_chunkDimensions.z * m_voxelSize ) );
						m_chunks[y][x].insert( m_chunks[y][x].begin(), backChunk );

						VoxelChunk* frontChunk = m_chunks[y][x].back();
						if( frontChunk != 0 )
						{
							frontChunk->Shutdown();
							SAFE_DELETE( frontChunk );
						}
						m_chunks[y][x].erase( m_chunks[y][x].end() - 1 );
					}
				}
			}
		}
	}

	m_centerChunk = this->FindChunkContainingPosition( m_centerPosition );

}

void VoxelTerrain::_ClearTerrain()
{

	for( auto yItr = m_chunks.begin(); yItr != m_chunks.end(); )
	{
		for( auto xItr = (*yItr).begin(); xItr != (*yItr).end(); )
		{
			for( auto zItr = (*xItr).begin(); zItr != (*xItr).end(); )
			{
				SAFE_DELETE( *zItr );
				zItr = (*xItr).erase( zItr );
			}
			xItr = (*yItr).erase( xItr );
		}
		yItr = m_chunks.erase( yItr );
	}

	Kiwi::FreeMemory( m_chunks );

}

void VoxelTerrain::_ResizeTerrain()
{

	int rdx = (int)m_renderDistance.x;
	int rdy = (int)m_renderDistance.y;
	int rdz = (int)m_renderDistance.z;

	int height = (rdy * 2) + 1;
	int width = (rdx * 2) + 1;
	int depth = (rdz * 2) + 1;

	if( m_chunks.size() > 0 && m_chunks[0].size() > 0 && m_chunks[0][0].size() > 0 )
	{
		if( depth > m_chunks[0][0].size() )
		{
			int initialSize = m_chunks[0][0].size();
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				for( int x = 0; x < m_chunks[0].size(); x++ )
				{
					int difference = depth - initialSize;
					while( difference > 0 )
					{
						difference -= 2;

						VoxelChunk* frontChunk = new VoxelChunk( *this, m_chunkDimensions );
						VoxelChunk* backChunk = new VoxelChunk( *this, m_chunkDimensions );
						
						frontChunk->SetPosition(m_chunks[y][x].back()->GetPosition() + Kiwi::Vector3d(0.0, 0.0, m_chunkDimensions.z * m_voxelSize ));
						backChunk->SetPosition(m_chunks[y][x][0]->GetPosition() - Kiwi::Vector3d(0.0, 0.0, m_chunkDimensions.z * m_voxelSize ));

						m_chunks[y][x].insert( m_chunks[y][x].begin(), backChunk );
						m_chunks[y][x].push_back( frontChunk );
					}
				}
			}

		} else if( depth < m_chunks[0][0].size() )
		{
			int initialSize = m_chunks[0][0].size();
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				for( int x = 0; x < m_chunks[0].size(); x++ )
				{
					int difference = initialSize - depth;
					while( difference > 0 )
					{
						difference -= 2;

						VoxelChunk* frontChunk = m_chunks[y][x].back();
						VoxelChunk* backChunk = m_chunks[y][x][0];
						if( frontChunk != 0 )
						{
							frontChunk->Shutdown();
							SAFE_DELETE( frontChunk );
						}
						if( backChunk != 0 )
						{
							backChunk->Shutdown();
							SAFE_DELETE( backChunk );
						}

						m_chunks[y][x].erase( m_chunks[y][x].end() - 1 );
						m_chunks[y][x].erase( m_chunks[y][x].begin() );
					}
				}
			}
		}

		if( width > m_chunks[0].size() )
		{
			int initialSize = m_chunks[0].size();
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				int difference = width - initialSize;
				while( difference > 0 )
				{
					difference -= 2;

					std::vector<VoxelChunk*> rightChunks;
					std::vector<VoxelChunk*> leftChunks;
					for( int z = 0; z < depth; z++ )
					{
						VoxelChunk* leftChunk = new VoxelChunk( *this, m_chunkDimensions );
						VoxelChunk* rightChunk = new VoxelChunk( *this, m_chunkDimensions );

						leftChunk->SetPosition( m_chunks[y][0][z]->GetPosition() - Kiwi::Vector3d( m_chunkDimensions.x * m_voxelSize, 0.0, 0.0 ) );
						rightChunk->SetPosition( m_chunks[y].back()[z]->GetPosition() + Kiwi::Vector3d( m_chunkDimensions.x * m_voxelSize, 0.0, 0.0 ) ); 

						rightChunks.push_back( rightChunk );
						leftChunks.push_back( leftChunk );
					}

					m_chunks[y].insert( m_chunks[y].begin(), leftChunks );
					m_chunks[y].push_back( rightChunks );
				}
			}

		} else if( width < m_chunks[0].size() )
		{
			int initialSize = m_chunks[0].size();
			for( int y = 0; y < m_chunks.size(); y++ )
			{
				int difference = initialSize - width;
				while( difference > 0 )
				{
					difference -= 2;

					for( int z = 0; z < depth; z++ )
					{
						VoxelChunk* leftChunk = m_chunks[y][0][z];
						VoxelChunk* rightChunk = m_chunks[y].back()[z];
						if( leftChunk != 0 )
						{
							leftChunk->Shutdown();
							SAFE_DELETE( leftChunk );
						}
						if( rightChunk != 0 )
						{
							rightChunk->Shutdown();
							SAFE_DELETE( rightChunk );
						}
					}

					m_chunks[y].erase( m_chunks[y].begin() );
					m_chunks[y].erase( m_chunks[y].end() - 1 );
				}
			}
		}

		if( height > m_chunks.size() )
		{//need to add layer(s)
			int difference = height - m_chunks.size();
			while( difference > 0 )
			{
				difference -= 2;

				std::vector<std::vector<VoxelChunk*>> topLayer;
				std::vector<std::vector<VoxelChunk*>> bottomLayer;
				topLayer.resize( width );
				bottomLayer.resize( width );
				for( int x = 0; x < width; x++ )
				{
					for( int z = 0; z < depth; z++ )
					{
						VoxelChunk* topChunk = new VoxelChunk( *this, m_chunkDimensions );
						VoxelChunk* bottomChunk = new VoxelChunk( *this, m_chunkDimensions );

						topChunk->SetPosition( m_chunks.back()[x][z]->GetPosition() + Kiwi::Vector3d( 0.0, m_chunkDimensions.y * m_voxelSize, 0.0 ) );
						bottomChunk->SetPosition( m_chunks[0][x][z]->GetPosition() - Kiwi::Vector3d( 0.0, m_chunkDimensions.y * m_voxelSize, 0.0 ) );

						topLayer[x].push_back( topChunk );
						bottomLayer[x].push_back( bottomChunk );
					}
				}
				m_chunks.insert( m_chunks.begin(), bottomLayer );
				m_chunks.push_back( topLayer );
			}

		} else if( height < m_chunks.size() )
		{//need to remove layer(s)
			int difference = m_chunks.size() - height;
			while( difference > 0 && m_chunks.size() >= 2 )
			{
				difference -= 2;

				for( int x = 0; x < width; x++ )
				{
					for( int z = 0; z < depth; z++ )
					{
						VoxelChunk* topChunk = m_chunks.back()[x][z];
						VoxelChunk* bottomChunk = m_chunks[0][x][z];
						if( topChunk != 0 )
						{
							topChunk->Shutdown();
							SAFE_DELETE( topChunk );
						}
						if( bottomChunk != 0 )
						{
							bottomChunk->Shutdown();
							SAFE_DELETE( bottomChunk );
						}
					}
				}
				m_chunks.erase( m_chunks.begin() );
				m_chunks.erase( m_chunks.end() - 1 );
			}
		}

	} else
	{//do a full rebuild
		this->_GenerateAroundCenter();
	}

}

void VoxelTerrain::FixedUpdate()
{

	if( m_doResize )
	{
		m_doResize = false;
		this->_ResizeTerrain();
	}

	unsigned int loadedCount = 0, chunkCount = 0;
	for( auto yItr = m_chunks.begin(); yItr != m_chunks.end(); yItr++ )
	{
		for( auto xItr = (*yItr).begin(); xItr != (*yItr).end(); xItr++ )
		{
			for( auto zItr = (*xItr).begin(); zItr != (*xItr).end(); )
			{
				if( (*zItr)->GetStatus() != VoxelChunk::SHUTDOWN )
				{
					if( (*zItr)->GetStatus() == VoxelChunk::UNINITIALIZED )
					{
						(*zItr)->TriggerFullRebuild();
					}
					(*zItr)->FixedUpdate();
					if( (*zItr)->IsBuilt() )
					{
						loadedCount++;
					}
					chunkCount++;
					zItr++;

				} else
				{
					SAFE_DELETE( *zItr );
					zItr = (*xItr).erase( zItr );
				}
			}
		}
	}

	if( loadedCount == chunkCount )
	{
		m_loadStatus = TERRAIN_READY;
	}

	if( m_target != 0 )
	{
		Kiwi::Transform* targetTransform = m_target->FindComponent<Kiwi::Transform>();
		if( targetTransform != 0 )
		{
			Kiwi::Vector3d targetOffset = m_centerPosition - targetTransform->GetPosition();
			Kiwi::Vector3d oldCenter = m_centerPosition;

			if( std::abs( targetOffset.x ) > m_centerLoadOffset.x )
			{
				m_centerPosition.x += (m_chunkDimensions.x * m_voxelSize) * -(targetOffset.x / std::abs( targetOffset.x ));
			}
			if( std::abs( targetOffset.z ) > m_centerLoadOffset.z )
			{
				m_centerPosition.z += (m_chunkDimensions.z * m_voxelSize) * -(targetOffset.z / std::abs( targetOffset.z ));
			}
		}
	}

	if( m_centerChunk != 0 )
	{
		VoxelChunk* newCenter = this->FindChunkContainingPosition( m_centerPosition );
		if( m_centerChunk != newCenter )
		{
			this->_ScrollToCenter();
		}

	} else
	{
		m_centerChunk = this->FindChunkContainingPosition( m_centerPosition );
	}

}

void VoxelTerrain::OnChunkLoaded( VoxelChunk* chunk )
{

	if( chunk != 0 )
	{
		m_startingCity->PlaceVoxels( chunk );
	}

}

void VoxelTerrain::ReplaceVoxelContainingPosition( const Kiwi::Vector3d& position )
{

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.z, m_voxelSize );

	Kiwi::Vector3d roundPos( xPos, yPos, zPos );

	VoxelChunk* posChunk = _FindChunkContainingPosition( roundPos );
	if( !posChunk )
	{
		return;
	}

	posChunk->RemoveVoxelAtPosition( roundPos );

}

void VoxelTerrain::RemoveVoxelContainingPosition( const Kiwi::Vector3d& position )
{

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.z, m_voxelSize );

	Kiwi::Vector3d roundPos( xPos, yPos, zPos );

	VoxelChunk* posChunk = _FindChunkContainingPosition( roundPos );
	if( !posChunk )
	{
		return;
	}

	posChunk->RemoveVoxelAtPosition( roundPos );

}

double VoxelTerrain::FindMaxHeightAtPosition( const Kiwi::Vector3d& position )
{

	return this->FindMaxHeightAtPosition( Kiwi::Vector2d( position.x, position.z ) );

}

double VoxelTerrain::FindMaxHeightAtPosition( const Kiwi::Vector2d& position )
{

	if( m_chunks.size() == 0 || m_chunks[0].size() == 0 || m_chunks[0][0].size() == 0 )
	{
		return -1.0;
	}

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );

	double maxHeight = -1.0;
	for( int cyi = m_chunks.size() - 1; cyi >= 0; cyi-- )
	{
		Kiwi::Vector3d chunkPos( xPos, (cyi * (m_chunkDimensions.y * m_voxelSize)) + m_chunks[0][0][0]->GetPosition().y, zPos );

		VoxelChunk* yChunk = this->_FindChunkContainingPosition( chunkPos );
		if( yChunk == 0 )
		{
			continue;

		}else
		{
			double maxChunkHeight = yChunk->GetMaxHeightAtPosition( Kiwi::Vector2d( xPos - yChunk->GetPosition().x, zPos - yChunk->GetPosition().z ) );
			if( maxChunkHeight > -0.9 )
			{
				return maxChunkHeight + yChunk->GetPosition().y + (m_voxelSize / 2.0);
			}
		}
	}

	return -1.0;

}

double VoxelTerrain::FindHeightUnderPosition( const Kiwi::Vector3d& position )
{

	if( m_chunks.size() == 0 )
	{
		return -1.0;
	}

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.z, m_voxelSize );

	for( double height = yPos ; height >= 0; height -= (m_chunkDimensions.y * m_voxelSize) )
	{
		VoxelChunk* posChunk = _FindChunkContainingPosition( Kiwi::Vector3d( xPos, yPos, zPos ) );
		if( !posChunk )
		{
			return -1.0;
		}

		//convert world position into position in chunk
		Kiwi::Vector3d posInChunk( xPos - posChunk->GetPosition().x, yPos - posChunk->GetPosition().y, zPos - posChunk->GetPosition().z );

		double maxChunkHeight = posChunk->GetMaxHeightAtPosition( posInChunk );
		if( maxChunkHeight > -0.9 )
		{
			maxChunkHeight += posChunk->GetPosition().y + (m_voxelSize / 2.0);
			if( maxChunkHeight <= position.y )
			{
				return maxChunkHeight;

			} else
			{
				double heightUnder = posChunk->FindHeightUnderPosition( posInChunk );
				if( heightUnder > -0.9 )
				{
					return heightUnder + posChunk->GetPosition().y + (m_voxelSize / 2.0);
				}
				//nothing under the position in this chunk
			}
			//nothing under the position in this chunk
		}
		//below the first chunk, yPos is always outside the chunks so adjust it to be the top-most position
		yPos = posChunk->GetPosition().y - m_voxelSize;
	}

	return -1.0;

}

Voxel* VoxelTerrain::FindSurfaceVoxelAtPosition( const Kiwi::Vector3d& position )
{

	if( m_chunks.size() == 0 )
	{
		return 0;
	}

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.z, m_voxelSize );

	VoxelChunk* posChunk = _FindChunkContainingPosition( Kiwi::Vector3d( xPos, yPos, zPos ) );
	if( posChunk == 0 )
	{
		return 0;
	}

	Kiwi::Vector3d topPos( xPos, yPos, zPos );

	return posChunk->FindSurfaceVoxelAtPosition( topPos );

}

Voxel* VoxelTerrain::FindSurfaceVoxelUnderPosition( const Kiwi::Vector3d& position )
{

	if( m_chunks.size() == 0 )
	{
		return 0;
	}
	
	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.z, m_voxelSize );

	for( double height = yPos; height >= 0; height -= (m_chunkDimensions.y * m_voxelSize) )
	{
		VoxelChunk* posChunk = _FindChunkContainingPosition( Kiwi::Vector3d( xPos, yPos, zPos ) );
		if( posChunk == 0 )
		{
			return 0;
		}

		Kiwi::Vector3d topPos( xPos, yPos, zPos );

		Voxel* v = posChunk->FindSurfaceVoxelUnderPosition( topPos );
		if( v != 0 )
		{
			return v;
		}
		//below the first chunk, yPos is always outside the chunks so adjust it to be the top-most position
		yPos = posChunk->GetPosition().y - m_voxelSize;
	}

	return 0;

}

Voxel* VoxelTerrain::FindSurfaceVoxelAbovePosition( const Kiwi::Vector3d& position )
{

	if( m_chunks.size() == 0 )
	{
		return 0;
	}

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.z, m_voxelSize );

	for( double height = yPos; height < m_chunks.size() * (m_chunkDimensions.y * m_voxelSize); height += (m_chunkDimensions.y * m_voxelSize) )
	{
		VoxelChunk* posChunk = _FindChunkContainingPosition( Kiwi::Vector3d( xPos, height, zPos ) );
		if( posChunk == 0 )
		{
			return 0;
		}

		Kiwi::Vector3d topPos( xPos, yPos, zPos );

		Voxel* v = posChunk->FindSurfaceVoxelAbovePosition( topPos );
		if( v != 0 )
		{
			return v;
		}
		//set ypos to the bottom of the next chunk
		yPos = posChunk->GetPosition().y + (m_chunkDimensions.y * m_voxelSize);
	}

	return 0;

}

Voxel* VoxelTerrain::FindWalkableVoxelAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int minClearance, unsigned char maxSolidity, unsigned int maxHeight )
{

	if( m_chunks.size() == 0 )
	{
		return 0;
	}

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( globalPos.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( globalPos.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( globalPos.z, m_voxelSize );

	for( double height = yPos; height < m_chunks.size() * (m_chunkDimensions.y * m_voxelSize); height += (m_chunkDimensions.y * m_voxelSize) )
	{
		VoxelChunk* posChunk = _FindChunkContainingPosition( Kiwi::Vector3d( xPos, height, zPos ) );
		if( posChunk == 0 )
		{
			return 0;
		}

		Kiwi::Vector3d topPos( xPos, yPos, zPos );

		Voxel* v = posChunk->FindWalkableAbovePoint( topPos, minClearance, maxSolidity, maxHeight );
		if( v != 0 )
		{
			return v;
		}
		//set ypos to the bottom of the next chunk
		yPos = posChunk->GetPosition().y + (m_chunkDimensions.y * m_voxelSize);
	}

	return 0;

}

std::vector<Voxel*> VoxelTerrain::FindVoxelsAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int numVoxels )
{

	std::vector<Voxel*> voxels;

	if( m_chunks.size() == 0 )
	{
		return voxels;
	}

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( globalPos.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( globalPos.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( globalPos.z, m_voxelSize );

	unsigned int voxelsLeft = numVoxels;
	while( voxels.size() < numVoxels )
	{
		VoxelChunk* posChunk = _FindChunkContainingPosition( Kiwi::Vector3d( xPos, yPos, zPos ) );
		if( posChunk == 0 )
		{
			break;
		}

		std::vector<Voxel*> foundVoxels = posChunk->FindVoxelsAbovePoint( Kiwi::Vector3d( xPos, yPos, zPos ), voxelsLeft );
		if( foundVoxels.size() == 0 )
		{//once there are no voxels found, there won't be any more above so we can stop checking
			break;
		}

		yPos += m_voxelSize * foundVoxels.size(); //if we found less voxels than requested, this should now be a position in the next chunk above
		voxelsLeft -= foundVoxels.size();

		voxels.insert( voxels.end(), foundVoxels.begin(), foundVoxels.end() );
	}

	return voxels;

}

VoxelChunk* VoxelTerrain::FindChunkContainingPosition( const Kiwi::Vector3d& position )
{

	if( m_chunks.size() == 0 )
	{
		MessageBox( NULL, L"chunks size 0", L"VoxelTerrain::FindChunkContainingPosition", MB_OK );
		return 0;
	}

	//round position to nearest multiple of voxel size
	double xPos = Kiwi::RoundToNearestd( position.x, m_voxelSize );
	double yPos = Kiwi::RoundToNearestd( position.y, m_voxelSize );
	double zPos = Kiwi::RoundToNearestd( position.z, m_voxelSize );

	return _FindChunkContainingPosition( Kiwi::Vector3d( xPos, yPos, zPos ) );

}

void VoxelTerrain::SetRenderDistance( const Kiwi::Vector3d& renderDistance )
{ 
	if( m_renderDistance != renderDistance ) 
	{ 
		m_renderDistance = renderDistance;
		Kiwi::Vector3d::Clamp( m_renderDistance, 1.0, 15.0 );
		m_doResize = true; 
	} 
}

void VoxelTerrain::SetCenterPosition( const Kiwi::Vector3d& position )
{

	Kiwi::Vector3d chunkNumber( std::floor( position.x / (m_chunkDimensions.x * m_voxelSize) ), std::floor( position.y / (m_chunkDimensions.y * m_voxelSize) ), std::floor( position.z / (m_chunkDimensions.z * m_voxelSize) ) );

	Kiwi::Vector3d chunkPos( chunkNumber.x * (m_chunkDimensions.x * m_voxelSize), chunkNumber.y * (m_chunkDimensions.y * m_voxelSize), chunkNumber.z * (m_chunkDimensions.z * m_voxelSize));
	m_centerPosition = chunkPos + Kiwi::Vector3d( ((m_chunkDimensions.x * m_voxelSize) / 2.0), ((m_chunkDimensions.y * m_voxelSize) / 2.0), ((m_chunkDimensions.z * m_voxelSize) / 2.0) );

}

VoxelDefinition* VoxelTerrain::GetVoxelDefinition( unsigned int voxelType )
{

	auto defItr = m_voxelDefinitions.find( voxelType );
	if( defItr != m_voxelDefinitions.end() )
	{
		return defItr->second;
	}

	return 0;

}

long VoxelTerrain::GetRenderedVoxelCount()
{

	long long count = 0;
	auto zItr = m_terrain.zChunks.begin();
	for( ; zItr != m_terrain.zChunks.end(); zItr++ )
	{
		auto xItr = zItr->second.xChunks.begin();
		for( ; xItr != zItr->second.xChunks.end(); xItr++)
		{
			auto yItr = xItr->second.yChunks.begin();
			for( ; yItr != xItr->second.yChunks.end(); yItr++)
			{
				count += yItr->second->GetSurfaceMeshVoxelCount();
			}
		}
	}

	return count;

}