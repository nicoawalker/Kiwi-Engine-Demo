#include "VoxelChunk.h"
#include "VoxelTerrain.h"
#include "NavMesh.h"

#include "City.h"
#include "House.h"

#include <Utilities\PerlinNoiseGenerator.h>

#include <Core\EngineRoot.h>
#include <Core\IThreadParam.h>

#include <chrono>

VoxelChunk::VoxelChunk( VoxelTerrain& terrain, const Kiwi::Vector3d& dimensions )
{

	m_terrain = &terrain;
	m_scene = m_terrain->GetScene();
	m_chunkDimensions = dimensions;

	m_chunkStatus = UNINITIALIZED;
	m_currentTask = NONE;

	m_shutdown = false;
	m_active = true;
	m_isSurfaceChunk = true;
	m_navMesh = 0;

	m_engine = terrain.GetScene()->GetEngine();

	m_verticalScale = 30.0;
	m_surfaceFrequency = 0.02;

	//flat plains biome parameters
	m_biomeParams.frequency = 0.006;
	m_biomeParams.persistance = 0.05;
	m_biomeParams.amplitude = 30.0;
	m_biomeParams.octaveCount = 10;

	m_opaqueSurfaceMesh = 0;

	m_neighborLeft = m_neighborRight = m_neighborFront = m_neighborBack = m_neighborBottom = m_neighborTop = 0;

	m_playerModified = false;

	m_fullRebuildPending = false;
	m_surfaceRebuildPending = false;
	m_built = false;
	m_currentlyRebuilding = false;
	m_linked = false;

}

VoxelChunk::~VoxelChunk()
{

	if( m_chunkStatus != SHUTDOWN )
	{
		this->Shutdown();
	}

}

Kiwi::IThreadParam* VoxelChunk::_RebuildAll( Kiwi::IThreadParam* param )
{

	if( !this->_Allocate() )
	{
		throw Kiwi::Exception( L"VoxelChunk::_BuildChunk", L"Not enough memory!" );
	}

	this->_GenerateTerrainData();

	return this->_RebuildSurfaces( param );

}

Kiwi::IThreadParam* VoxelChunk::_RebuildSurfaces( Kiwi::IThreadParam* param )
{

	SurfaceParam* surfaceParam = dynamic_cast<SurfaceParam*>(param);
	if( surfaceParam == 0 )
	{
		return 0;
	}

	//once the chunk has been built, link it to new neighbors
	if( m_built )
	{
		this->_LinkNeighbors();
	}

	static int once = 0;
	if( once == 0 && m_position.y == 0.0 && m_position.z == 0 && m_position.x == 0 )
	{
		once = 1;
		//MessageBox( NULL, L"A", L"A", MB_OK );

		/*City* city = new City( m_terrain, Kiwi::Vector2d( 160.0, 160.0 ), m_position + Kiwi::Vector3d(16.0, 20.0, 16.0) );
		city->Generate();
		city->PlaceVoxels( this );*/

		/*House* house = new House(0, 1, 0, m_position + Kiwi::Vector3d( 16.0, 16.0, 16.0 ), Kiwi::Vector3d(16.0, 16.0, 16.0) );
		house->Generate();

		int counter = 0;
		for( auto itr = house->GetVoxels().begin(); itr != house->GetVoxels().end(); itr++ )
		{
		counter++;
		this->ReplaceVoxelAtPosition( itr->first + house->GetPosition(), itr->second );
		}*/
	}

	this->_GenerateSurfaceVoxels( surfaceParam );

	Kiwi::Mesh* solidMesh = this->_GenerateSurfaceMeshes();

	surfaceParam->solidMesh = solidMesh;

	this->_GenerateNavMesh();

	return surfaceParam;

}

bool VoxelChunk::_Allocate()
{

	if( m_terrain == 0 )
	{
		return false;
	}

	//determine whether there's enough ram available to hold the all of the VoxelData structures, as well as the heightmap
	unsigned long long chunkSize = (unsigned long long)(m_chunkDimensions.x * m_chunkDimensions.y * m_chunkDimensions.z) * (sizeof( VoxelData ) + sizeof( double ));
	unsigned long long availRam = Kiwi::GetSysMemInfo().availablePhysical;
	if( availRam < chunkSize )
	{
		return false;
	}

	VoxelDefinition* airDef = m_terrain->GetVoxelDefinition( VoxelType::AIR );
	if( airDef == 0 )
	{
		return false;
	}

	//allocate space for the chunk data and initialize the chunk to air
	m_voxelDataMutex.lock();
	m_voxelData.resize( (unsigned int)(m_chunkDimensions.x * m_chunkDimensions.y * m_chunkDimensions.z), { airDef, 0 } ); //initialize chunk to air
	m_voxelDataMutex.unlock();

	m_heightmap.resize( (unsigned int)(m_chunkDimensions.x * m_chunkDimensions.z), -1.0 );

	return true;

}

void VoxelChunk::_GenerateTerrainData()
{

	m_voxelDataMutex.lock();

	Kiwi::PerlinNoiseGenerator pGen( m_terrain->GetWorldSeed() );

	double voxelSize = m_terrain->GetVoxelSize();
	double oceanLevel = m_terrain->GetSeaLevel();
	double surfaceLevel = m_terrain->GetSurfaceLevel();

	srand( m_terrain->GetWorldSeed() );

	double cutoffHeight = 4;
	unsigned int cdx = (unsigned int)m_chunkDimensions.x;
	unsigned int cdy = (unsigned int)m_chunkDimensions.y;
	unsigned int cdz = (unsigned int)m_chunkDimensions.z;
	unsigned int errorCount = 0;
	unsigned int xIndex = 0, yIndex = 0, zIndex = 0;
	for( double x = 0.0, xIndex = 0; xIndex < cdx && m_shutdown == false; x += voxelSize, xIndex++ )
	{
		for( double z = 0.0, zIndex = 0; zIndex < cdz && m_shutdown == false; z += voxelSize, zIndex++ )
		{

			//double height = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, 0.0 ), 6, 0.25, m_surfaceFrequency, 65 );

			double height = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, 0.0 ), m_biomeParams.octaveCount, m_biomeParams.persistance, m_biomeParams.frequency );
			height = (height + 1.0) / 2.0; //clamp height between 0 and 1
			height *= m_biomeParams.amplitude; //scale the height to the proper height

			height -= m_position.y; //convert global height to local height
			if( height >= (m_chunkDimensions.y * voxelSize) ) height = (m_chunkDimensions.y * voxelSize) - voxelSize;

			for( double y = 0.0, yIndex = 0; y < height && m_shutdown == false; y += voxelSize, yIndex++ )
			{

				bool genCave = true;
				if( y > height - cutoffHeight )
				{
					int randVal = rand() % 10;
					if( randVal > 2 )
					{
						genCave = false;
					}
				}

				//if( m_biomeParams.biomeType != 0 )
				//{//generate surface features only if the biome is not super flat
				//	double posValue = 1.0;
				//	if( m_position.y >= 0.0 )
				//	{//generate surface features
				//		posValue = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, m_position.y + y ), 4, 0.25, 0.1 );
				//		//posValue = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, m_position.y + y ), 6, 0.25, m_surfaceFrequency, 65 );
				//		if( posValue < -0.4 )
				//		{
				//			continue;
				//		}

				//	} else
				//	{//generate large underground caves
				//		posValue = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, m_position.y + y ), 4, 0.25, 0.07 );
				//		if( posValue < 0.0 )
				//		{
				//			continue;
				//		}
				//	}
				//}

				double posValue = 1.0;

				if( (m_position.y == 0.0 && y > 10) || m_position.y > 0.0 )
				{//generate surface features
					//posValue = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, m_position.y + y ), 4, 0.25, 0.1, 128 );
					
					double threshold = 0.0;
					if( m_position.y == 0 && y < 20 )
					{
						threshold = 0.1 * (20 - y);
					}
					posValue = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, m_position.y + y ), 6, 0.25, 0.01 );
					if( posValue < -threshold )
					{
						continue;
					}

				} 
				else
				{//generate large underground caves
					posValue = pGen.Perlin( Kiwi::Vector3d( m_position.x + x, m_position.z + z, m_position.y + y ), 4, 0.25, 0.07 );
					if( posValue < 0.0 )
					{
						continue;
					}
				}

				unsigned int voxelIndex = xIndex + (cdx * yIndex) + ((cdx * cdy) * zIndex);

				if( voxelIndex >= m_voxelData.size() )
				{
					continue;
				}

				m_heightmap[xIndex + (zIndex * cdx)] = y; //voxel isn't air so store the y position as the greatest height

				/*if( posValue < -0.5)
				{*/
					if( y < height - 1.0 * voxelSize )
					{
						m_voxelData[voxelIndex].definition = m_terrain->GetVoxelDefinition( VoxelType::DIRT );

					} else
					{
						m_voxelData[voxelIndex].definition = m_terrain->GetVoxelDefinition( VoxelType::GRASS );
					}

				//} else
				//{
				//	if( y < oceanLevel )
				//	{//air below ocean level is converted to water
				//		m_voxelData[voxelIndex].type = Voxel::WATER;
				//		m_voxelData[voxelIndex].transparent = true;
				//	}
				//}
			}
		}
	}

	m_voxelDataMutex.unlock();

}

void VoxelChunk::_GenerateSurfaceVoxels( Kiwi::IThreadParam* param )
{

	if( param == 0 )
	{
		return;
	}

	if( m_voxelData.size() < (unsigned int)(m_chunkDimensions.x * m_chunkDimensions.y * m_chunkDimensions.z) )
	{//some voxels are missing
		throw Kiwi::Exception( L"VoxelChunk::RebuildSurfaceMesh", L"Voxel data missing." );
	}

	m_voxelDataMutex.lock();

	SurfaceParam* sParam = dynamic_cast<SurfaceParam*>(param);
	if( sParam == 0 )
	{ 
		m_voxelDataMutex.unlock();
		return;
	}

	double voxelSize = m_terrain->GetVoxelSize();

	unsigned int cdx = (unsigned int)m_chunkDimensions.x;
	unsigned int cdy = (unsigned int)m_chunkDimensions.y;
	unsigned int cdz = (unsigned int)m_chunkDimensions.z;
	unsigned int errorCount = 0;
	unsigned int xIndex = 0, yIndex = 0, zIndex = 0;
	for( double x = 0.0, xIndex = 0; xIndex < cdx && m_shutdown == false; x += voxelSize, xIndex++ )
	{
		for( double y = 0.0, yIndex = 0; yIndex < cdy && m_shutdown == false; y += voxelSize, yIndex++ )
		{
			for( double z = 0.0, zIndex = 0; zIndex < cdz && m_shutdown == false; z += voxelSize, zIndex++ )
			{
				unsigned int voxelIndex = xIndex + (cdx * yIndex) + (cdx * cdy * zIndex);

				if( voxelIndex >= m_voxelData.size() )
				{
					if( m_scene && errorCount < 10 )
					{
						Kiwi::EngineRoot* engine = m_scene->GetEngine();
						engine->GetConsole()->PrintDebug( L"Chunk: Out of bounds index: " + Kiwi::ToWString( voxelIndex ) + L" at position: " + Kiwi::Vector3d( x, y, z ).ToString() );
						errorCount++;
					}
					continue;
				}

				VoxelData* voxelData = &m_voxelData[voxelIndex];
				if( voxelData->definition == 0 )
				{
					voxelData->definition = m_terrain->GetVoxelDefinition( VoxelType::AIR );
					continue;
				}

				//check whether this is a solid voxel and if it is, generate it if at least one face is visible
				if( voxelData->definition->type == VoxelType::AIR )
				{
					continue;

				} else
				{
					if( xIndex > 0 )
					{//check voxel to the left, in the current chunk
						unsigned int left = (xIndex - 1) + (cdx * yIndex) + ((cdx* cdy) * zIndex);
						if( m_voxelData[left].definition->type == VoxelType::AIR || m_voxelData[left].definition->color.alpha < 0.99 )
						{
							voxelData->visibility |= 0x02;
						}

					} else
					{//check voxel to the left, which is in the chunk to the left
						if( m_neighborLeft != 0 && m_neighborLeft->IsBuilt() )
						{
							VoxelData* voxelLeft = m_neighborLeft->GetVoxelDataAtPosition( Kiwi::Vector3d( (m_chunkDimensions.x * voxelSize) - voxelSize, y, z ) );
							if( voxelLeft == 0 || voxelLeft->definition->type == VoxelType::AIR )
							{
								voxelData->visibility |= 0x02;
							}
						}
					}

					if( xIndex < cdx - 1 )
					{//check voxel to the right, in the current chunk
						unsigned int right = (xIndex + 1) + (cdx * yIndex) + ((cdx * cdy) * zIndex);
						if( m_voxelData[right].definition->type == VoxelType::AIR || m_voxelData[right].definition->color.alpha < 0.99 )
						{
							voxelData->visibility |= 0x01;
						}

					} else
					{//check voxel to the right, which is in the chunk to the right
						if( m_neighborRight != 0 && m_neighborRight->IsBuilt() )
						{
							VoxelData* voxelRight = m_neighborRight->GetVoxelDataAtPosition( Kiwi::Vector3d( 0.0, y, z ) );
							if( voxelRight == 0 || voxelRight->definition->type == VoxelType::AIR )
							{
								voxelData->visibility |= 0x01;
							}
						}
					}

					if( yIndex > 0 )
					{
						unsigned int below = xIndex + (cdx * (yIndex - 1)) + ((cdx * cdy) * zIndex);
						if( m_voxelData[below].definition->type == VoxelType::AIR || m_voxelData[below].definition->color.alpha < 0.99 )
						{
							voxelData->visibility |= 0x04;
						}

					} else if( m_neighborBottom && m_neighborBottom->IsBuilt() )
					{
						VoxelData* voxelBottom = m_neighborBottom->GetVoxelDataAtPosition( Kiwi::Vector3d( x, (m_chunkDimensions.y * voxelSize) - voxelSize, z ) );
						if( voxelBottom == 0 || voxelBottom->definition->type == VoxelType::AIR )
						{
							voxelData->visibility |= 0x04;
						}
					}

					if( yIndex < cdy - 1 )
					{//check voxel above, in the current chunk
						unsigned int above = xIndex + (cdx * (yIndex + 1)) + ((cdx * cdy) * zIndex);
						if( m_voxelData[above].definition->type == VoxelType::AIR || m_voxelData[above].definition->color.alpha < 0.99 )
						{
							voxelData->visibility |= 0x08;
						}

					} else if( m_neighborTop && m_neighborTop->IsBuilt() )
					{//check voxel above, which is in the chunk above
						VoxelData* voxelAbove = m_neighborTop->GetVoxelDataAtPosition( Kiwi::Vector3d( x, 0.0, z ) );
						if( voxelAbove == 0 || voxelAbove->definition->type == VoxelType::AIR )
						{
							voxelData->visibility |= 0x08;
						}
					}

					if( zIndex > 0 )
					{
						unsigned int back = voxelIndex - (cdx * cdy);
						if( m_voxelData[back].definition->type == VoxelType::AIR || m_voxelData[back].definition->color.alpha < 0.99 )
						{
							voxelData->visibility |= 0x10;
						}

					} else if( m_neighborBack && m_neighborBack->IsBuilt() )
					{
						VoxelData* voxelBack = m_neighborBack->GetVoxelDataAtPosition( Kiwi::Vector3d( x, y, (m_chunkDimensions.z * voxelSize) - voxelSize ) );
						if( voxelBack == 0 || voxelBack->definition->type == VoxelType::AIR )
						{
							voxelData->visibility |= 0x10;
						}
					}

					if( zIndex < cdz - 1 )
					{//check voxel in front, in the current chunk
						unsigned int front = voxelIndex + (cdx * cdy);
						if( m_voxelData[front].definition->type == VoxelType::AIR || m_voxelData[front].definition->color.alpha < 0.99 )
						{
							voxelData->visibility |= 0x20;
						}

					} else if( m_neighborFront && m_neighborFront->IsBuilt() )
					{//check voxel in front, which is in the chunk in front
						VoxelData* voxelFront = m_neighborFront->GetVoxelDataAtPosition( Kiwi::Vector3d( x, y, 0.0 ) );
						if( voxelFront == 0 || voxelFront->definition->type == VoxelType::AIR )
						{
							voxelData->visibility |= 0x20;
						}
					}

					if( voxelData->visibility != 0 )
					{
						sParam->Lock();
						sParam->surfaceVoxelData[Kiwi::Vector3d( x, y, z )] = voxelData;
						sParam->Unlock();
					}
				}
			}
		}
	}

	m_voxelDataMutex.unlock();

}

Kiwi::Mesh* VoxelChunk::_GenerateSurfaceMeshes()
{

	double voxelSize = m_terrain->GetVoxelSize();

	std::vector<VoxelData*> transparentVoxels;
	std::vector<Kiwi::Vector3d> surfaceVertices;
	std::vector<Kiwi::Vector2d> surfaceUVs;
	std::vector<Kiwi::Vector3d> surfaceNormals;
	std::vector<Kiwi::Color> surfaceColors;

	unsigned int cdx = (unsigned int)m_chunkDimensions.x;
	unsigned int cdy = (unsigned int)m_chunkDimensions.y;
	unsigned int cdz = (unsigned int)m_chunkDimensions.z;
	unsigned int xIndex = 0, yIndex = 0, zIndex = 0;
	for( double x = 0.0, xIndex = 0; xIndex < cdx && m_shutdown == false; x += voxelSize, xIndex++ )
	{
		for( double y = 0.0, yIndex = 0; yIndex < cdy && m_shutdown == false; y += voxelSize, yIndex++ )
		{
			for( double z = 0.0, zIndex = 0; zIndex < cdz && m_shutdown == false; z += voxelSize, zIndex++ )
			{
				unsigned int voxelIndex = xIndex + (cdx * yIndex) + (cdx * cdy * zIndex);
				if( voxelIndex >= m_voxelData.size() )
				{
					continue;
				}

				VoxelData* voxelData = &m_voxelData[voxelIndex];
				if( voxelData )
				{
					VoxelDefinition* voxelDef = voxelData->definition;
					if( voxelDef == 0 )
					{
						continue;
					}

					if( voxelDef->type == VoxelType::AIR )
					{
						continue;
					}

					if( voxelDef )
					{
						if( voxelDef->color.alpha < 0.999 )
						{//will generate the transparent voxels after
							transparentVoxels.push_back( voxelData );
							continue;
						}

						Kiwi::Vector3d voxelPos( x, y, z );

						double ve = voxelSize * 0.5; //local voxel vertex edge

						double sideColorMod = 0.1;
						Kiwi::Color sideColor( voxelDef->color.red - (voxelDef->color.red * sideColorMod), voxelDef->color.green - (voxelDef->color.green * sideColorMod), voxelDef->color.blue - (voxelDef->color.blue * sideColorMod), voxelDef->color.alpha );

						if( voxelData->GetSideVisibility( Voxel::RIGHT ) )
						{//there is no voxel to the right, or it has transparency
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, ve ) + voxelPos );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 1.0, 0.0, 0.0 ) );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
						}
						if( voxelData->GetSideVisibility( Voxel::LEFT ) )
						{//there is no voxel to the left, or it has transparency
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, -ve ) + voxelPos );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( -1.0, 0.0, 0.0 ) );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
						}
						if( voxelData->GetSideVisibility( Voxel::BOTTOM ) )
						{//there is no voxel below, or it has transparency
							if( voxelPos.y != 0.0 || m_neighborBottom != 0 ) //if this is the bottom voxel, and there is no chunk underneath, dont generate the face
							{
								surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, -ve ) + voxelPos );
								surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, -ve ) + voxelPos );
								surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, ve ) + voxelPos );
								surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, ve ) + voxelPos );
								surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, -ve ) + voxelPos );
								surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, ve ) + voxelPos );
								surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
								surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
								surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
								surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
								surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
								surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
								surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
								surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
								surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
								surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
								surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
								surfaceNormals.push_back( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
								surfaceColors.push_back( voxelDef->color );
								surfaceColors.push_back( voxelDef->color );
								surfaceColors.push_back( voxelDef->color );
								surfaceColors.push_back( voxelDef->color );
								surfaceColors.push_back( voxelDef->color );
								surfaceColors.push_back( voxelDef->color );
							}
						}
						if( voxelData->GetSideVisibility( Voxel::TOP ) )
						{//there is no voxel above, or it has transparency
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, -ve ) + voxelPos );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 1.0, 0.0 ) );
							surfaceColors.push_back( voxelDef->color );
							surfaceColors.push_back( voxelDef->color );
							surfaceColors.push_back( voxelDef->color );
							surfaceColors.push_back( voxelDef->color );
							surfaceColors.push_back( voxelDef->color );
							surfaceColors.push_back( voxelDef->color );
						}
						if( voxelData->GetSideVisibility( Voxel::BACK ) )
						{//there is no voxel behind, or it has transparency
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, -ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, -ve ) + voxelPos );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
						}
						if( voxelData->GetSideVisibility( Voxel::FRONT ) )
						{//there is no voxel in front, or it has transparency
							surfaceVertices.push_back( Kiwi::Vector3d( ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( ve, -ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, ve, ve ) + voxelPos );
							surfaceVertices.push_back( Kiwi::Vector3d( -ve, -ve, ve ) + voxelPos );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 0.0, 1.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 0.0 ) );
							surfaceUVs.push_back( Kiwi::Vector2d( 1.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
							surfaceNormals.push_back( Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
							surfaceColors.push_back( sideColor );
						}
					}
				}
			}
		}
	}

	if( surfaceVertices.size() == 0 )
	{
		return 0;
	}

	Kiwi::Mesh* mesh = new Kiwi::Mesh( L"Chunk Entity " + m_position.ToString() + L"/mesh" );
	mesh->SetShader( L"TerrainShader" );
	mesh->SetVertices( surfaceVertices );
	mesh->SetNormals( surfaceNormals );
	mesh->SetUVs( surfaceUVs );
	mesh->SetColors( surfaceColors );

	return mesh;

}

void VoxelChunk::_GenerateNavMesh()
{

	if( m_terrain == 0 || m_surfaceVoxels.size() == 0 )
	{
		return;
	}

	double voxelSize = m_terrain->GetVoxelSize();

	SAFE_DELETE( m_navMesh );

	unsigned int width = (unsigned int)m_chunkDimensions.x;
	unsigned int length = (unsigned int)m_chunkDimensions.z;

	unsigned int cdy = (unsigned int)m_chunkDimensions.y;

	m_navMesh = new NavMesh( m_position, width, length, Kiwi::Vector3d( voxelSize, voxelSize, voxelSize) );
	assert( m_navMesh );

	for( auto itr = m_surfaceVoxels.begin(); itr != m_surfaceVoxels.end(); itr++ )
	{
		if( itr->second != 0 )
		{
			NavMesh::Node* newNode = new NavMesh::Node();
			newNode->cost = itr->second->GetDefinition()->moveCost;
			newNode->position = itr->second->GetPosition();
			m_navMesh->AddNodeAtGlobalPoint( itr->second->GetPosition(), newNode );
		}
	}

}

void VoxelChunk::_LinkNeighbors()
{

	double voxelSize = m_terrain->GetVoxelSize();

	m_neighborRight = m_terrain->FindChunkContainingPosition( m_position + Kiwi::Vector3d( m_chunkDimensions.x * voxelSize, 0.0, 0.0 ) );
	m_neighborLeft = m_terrain->FindChunkContainingPosition( m_position - Kiwi::Vector3d( m_chunkDimensions.x * voxelSize, 0.0, 0.0 ) );
	m_neighborTop = m_terrain->FindChunkContainingPosition( m_position + Kiwi::Vector3d( 0.0, m_chunkDimensions.y * voxelSize, 0.0 ) );
	m_neighborBottom = m_terrain->FindChunkContainingPosition( m_position - Kiwi::Vector3d( 0.0, m_chunkDimensions.y * voxelSize, 0.0 ) );
	m_neighborFront = m_terrain->FindChunkContainingPosition( m_position + Kiwi::Vector3d( 0.0, 0.0, m_chunkDimensions.z * voxelSize ) );
	m_neighborBack = m_terrain->FindChunkContainingPosition( m_position - Kiwi::Vector3d( 0.0, 0.0, m_chunkDimensions.z * voxelSize ) );

	//wait for all neighbors to finish generating their terrain
	while( (m_neighborRight != 0 && !m_neighborRight->IsBuilt()) ||
		   (m_neighborLeft != 0 && !m_neighborLeft->IsBuilt()) ||
		   (m_neighborTop != 0 && !m_neighborTop->IsBuilt()) ||
		   (m_neighborBottom != 0 && !m_neighborBottom->IsBuilt()) ||
		   (m_neighborFront != 0 && !m_neighborFront->IsBuilt()) ||
		   (m_neighborBack != 0 && !m_neighborBack->IsBuilt()) )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	m_linked = true;

}

void VoxelChunk::Shutdown()
{

	m_shutdown = true;

	this->Unload();

	m_chunkStatus = SHUTDOWN;

}

void VoxelChunk::FixedUpdate()
{

	if( m_shutdown ) return;

	for( auto itr = m_runningThreads.begin(); itr != m_runningThreads.end(); )
	{
		try
		{
			if( m_engine->GetThreadStatus( *itr ) == Kiwi::THREAD_READY )
			{
				Kiwi::IThreadParam* result = m_engine->JoinThread( *itr );

				itr = m_runningThreads.erase( itr );

				if( result != 0 && result->GetID() == 1 )
				{
					SurfaceParam* param = dynamic_cast<SurfaceParam*>(result);
					if( param != 0 )
					{
						if( param->solidMesh != 0 )
						{
							if( m_opaqueSurfaceMesh == 0 )
							{//create a new entity for the surface mesh, if one doesnt exist already
								m_opaqueSurfaceMesh = m_scene->CreateEntity( L"Chunk Entity " + m_position.ToString() );
								m_opaqueSurfaceMesh->AddTag( L"terrain" );

							} else
							{
								Kiwi::Mesh* oldMesh = m_opaqueSurfaceMesh->FindComponent<Kiwi::Mesh>();
								m_opaqueSurfaceMesh->DetachComponent( oldMesh );
								//oldMesh->Shutdown();
								//SAFE_DELETE( oldMesh );
							}

							m_opaqueSurfaceMesh->FindComponent<Kiwi::Transform>()->SetPosition( m_position );
							m_opaqueSurfaceMesh->AttachComponent( param->solidMesh );
						}

						//now update the surface voxel array
						Kiwi::FreeMemory( m_surfaceVoxels );

						if( param->surfaceVoxelData.size() != 0 )
						{
							m_surfaceVoxelMutex.lock();
							m_surfaceVoxels.reserve( param->surfaceVoxelData.size() );

							for( auto defItr = param->surfaceVoxelData.begin(); defItr != param->surfaceVoxelData.end() && m_shutdown == false; defItr++ )
							{
								Voxel* newVoxel = new Voxel( defItr->second->definition, Kiwi::Vector3d( m_position.x + defItr->first.x, m_position.y + defItr->first.y, m_position.z + defItr->first.z ), defItr->second->definition->state );
								m_surfaceVoxels[newVoxel->GetPosition()] = newVoxel;
							}
							m_surfaceVoxelMutex.unlock();
						}

						SAFE_DELETE( param );

						m_currentlyRebuilding = false;

						if( !m_built )
						{
							m_built = true;
							m_terrain->OnChunkLoaded( this );
						}

						//if the chunk hasnt been linked yet, do so now
						if( m_linked == false )
						{
							this->TriggerSurfaceRebuild();
						}
					}
				}

			} else
			{
				itr++;
			}

		} catch( Kiwi::Exception& e )
		{
			itr = m_runningThreads.erase( itr );
			if( m_scene )
			{
				Kiwi::EngineRoot* engine = m_scene->GetEngine();
				engine->GetConsole()->PrintDebug( L"Chunk loading generated exception (" + m_position.ToString() + L"): " + e.GetError() );
			}
		}
	}

	if( m_currentlyRebuilding == false )
	{
		if( m_surfaceRebuildPending == true )
		{
			m_surfaceRebuildPending = false;
			m_currentlyRebuilding = true;
			m_chunkStatus = BUSY;

			m_runningThreads.push_back( m_engine->SpawnThread( this, &VoxelChunk::_RebuildSurfaces, new SurfaceParam() ) );

		} else if( m_fullRebuildPending == true )
		{
			m_fullRebuildPending = false;
			m_surfaceRebuildPending = false;
			m_currentlyRebuilding = true;
			m_chunkStatus = BUSY;

			this->Unload();

			m_runningThreads.push_back( m_engine->SpawnThread( this, &VoxelChunk::_RebuildAll, new SurfaceParam() ) );
		}
	}

}

void VoxelChunk::TriggerFullRebuild()
{

	if( m_shutdown == false )
	{
		m_fullRebuildPending = true;
		m_surfaceRebuildPending = false;
	}

}

void VoxelChunk::TriggerSurfaceRebuild()
{

	if( m_shutdown == false )
	{
		m_surfaceRebuildPending = true;
	}

}

void VoxelChunk::SetBiome( const VoxelTerrain::BiomeParameters& biomeParams )
{



}

Voxel* VoxelChunk::_FindNextAbovePoint( const Kiwi::Vector3d& globalPos )
{

	double voxelSize = m_terrain->GetVoxelSize();

	for( double height = globalPos.y + voxelSize; height < m_position.y + (m_chunkDimensions.y * voxelSize); height += voxelSize )
	{
		auto voxelItr = m_surfaceVoxels.find( Kiwi::Vector3d( globalPos.x, height, globalPos.z ) );
		if( voxelItr != m_surfaceVoxels.end() )
		{
			return voxelItr->second;
		}
	}

	return 0;

}

/*destroys every voxel in the chunk*/
void VoxelChunk::Unload()
{

	//wait for all threads to finish
	if( m_engine != 0 )
	{
		for( auto itr = m_runningThreads.begin(); itr != m_runningThreads.end(); itr++ )
		{
			m_engine->JoinThread( *itr );
		}
	}
	m_runningThreads.clear();

	std::lock_guard<std::mutex> dataGuard( m_voxelDataMutex );
	std::lock_guard<std::mutex> surfaceGuard( m_surfaceVoxelMutex );

	Kiwi::FreeMemory( m_voxelData );

	for( auto itr = m_surfaceVoxels.begin(); itr != m_surfaceVoxels.end(); )
	{
		SAFE_DELETE( itr->second );
		itr = m_surfaceVoxels.erase( itr );
	}

	if(m_opaqueSurfaceMesh) m_opaqueSurfaceMesh->Shutdown();

	for( auto itr = m_transparentSurfaceMeshes.begin(); itr != m_transparentSurfaceMeshes.end(); )
	{
		if( *itr )
		{
			(*itr)->Shutdown();
		}
		itr = m_transparentSurfaceMeshes.erase( itr );
	}

	SAFE_DELETE( m_navMesh );

	m_built = false;
	m_surfaceRebuildPending = false;
	m_fullRebuildPending = false;
	m_currentlyRebuilding = false;

}

/*replaces a the voxel at the given position, or creates a new one if one doesnt exist*/
void VoxelChunk::ReplaceVoxelAtPosition( const Kiwi::Vector3d& position, int newVoxelType )
{

	if( !m_built || (m_voxelData.size() == 0 && m_surfaceVoxels.size() == 0) )
	{
		return;
	}

	double voxelSize = m_terrain->GetVoxelSize();

	//round position to nearest voxel
	double rx = Kiwi::RoundToNearestd( position.x, voxelSize );
	double ry = Kiwi::RoundToNearestd( position.y, voxelSize );
	double rz = Kiwi::RoundToNearestd( position.z, voxelSize );

	if( rx - m_position.x >= m_chunkDimensions.x * voxelSize ||
		rx - m_position.x < 0.0 ||
		ry - m_position.y >= m_chunkDimensions.y * voxelSize ||
		ry - m_position.y < 0.0 ||
		rz - m_position.z >= m_chunkDimensions.z * voxelSize ||
		rz - m_position.z < 0.0 )
	{
		return;
	}

	Voxel newVoxel( m_terrain->GetVoxelDefinition( newVoxelType ), Kiwi::Vector3d( rx, ry, rz ) );

	if( m_surfaceVoxels.size() > 0 )
	{
		m_surfaceVoxelMutex.lock();

		auto voxelItr = m_surfaceVoxels.find( Kiwi::Vector3d( rx, ry, rz ) );
		if( voxelItr != m_surfaceVoxels.end() )
		{
			*voxelItr->second = newVoxel;
			this->TriggerSurfaceRebuild();
		}

		m_surfaceVoxelMutex.unlock();
	}

	if( m_voxelData.size() > 0 )
	{
		//convert position to index in data array
		unsigned int xIndex = (unsigned int)((position.x - m_position.x) / voxelSize);
		unsigned int yIndex = (unsigned int)((position.y - m_position.y) / voxelSize);
		unsigned int zIndex = (unsigned int)((position.z - m_position.z) / voxelSize);

		unsigned int voxelIndex = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
		if( voxelIndex >= m_voxelData.size() )
		{
			//MessageBox( NULL, L"Out of bounds index", L"VoxelChunk::ReplaceVoxelAtPosition", MB_OK );
			//throw Kiwi::Exception( L"VoxelChunk::ReplaceVoxelAtPosition", L"Out of bounds index (" + Kiwi::ToWString( voxelIndex ) + L" in chunk " + m_position.ToString() );
			return;
		}

		VoxelData* vd = &m_voxelData[voxelIndex];
		if( vd != 0 )
		{
			VoxelDefinition* def = newVoxel.GetDefinition();
			if( def == 0 )
			{
				return;
			}

			vd->definition = def;

			m_playerModified = true;

			//if this new voxel is not solid we need to update the visibility of the surrounding voxels
			if( def->type == VoxelType::AIR )
			{
				int left = (xIndex - 1) + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x* (unsigned int)m_chunkDimensions.y) * zIndex);
				int right = (xIndex + 1) + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
				int below = xIndex + ((unsigned int)m_chunkDimensions.x * (yIndex - 1)) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
				int above = xIndex + ((unsigned int)m_chunkDimensions.x * (yIndex + 1)) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
				int back = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * (zIndex - 1));//voxelIndex - ((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y);
				int front = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * (zIndex + 1));//voxelIndex + ((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y);

				if( xIndex > 0 )
				{//check voxel to the left, in the current chunk
					if( m_voxelData[left].definition->type != VoxelType::AIR )
					{
						m_voxelData[left].SetSideVisibility( Voxel::RIGHT, true );
					}

				} else
				{//check voxel to the left, which is in the chunk to the left
					if( m_neighborLeft != 0 && m_neighborLeft->IsBuilt() )
					{
						VoxelData* voxelLeft = m_neighborLeft->GetVoxelDataAtPosition( Kiwi::Vector3d( (m_chunkDimensions.x * voxelSize) - voxelSize, yIndex*voxelSize, zIndex*voxelSize ) );
						if( voxelLeft != 0 && voxelLeft->definition->type != VoxelType::AIR )
						{
							voxelLeft->SetSideVisibility( Voxel::RIGHT, true );
							m_neighborLeft->TriggerSurfaceRebuild();
						}
					}
				}

				if( xIndex < (unsigned int)m_chunkDimensions.x - 1 )
				{//check voxel to the right, in the current chunk

					if( m_voxelData[right].definition->type != VoxelType::AIR )
					{
						m_voxelData[right].SetSideVisibility( Voxel::LEFT, true );
					}

				} else if( m_neighborRight )
				{//check voxel to the right, which is in the chunk to the right
					if( m_neighborRight != 0 && m_neighborRight->IsBuilt() )
					{
						VoxelData* voxelRight = m_neighborRight->GetVoxelDataAtPosition( Kiwi::Vector3d( 0.0, yIndex*voxelSize, zIndex*voxelSize ) );
						if( voxelRight != 0 && voxelRight->definition->type != VoxelType::AIR )
						{
							voxelRight->SetSideVisibility( Voxel::LEFT, true );
							m_neighborRight->TriggerSurfaceRebuild();
						}
					}
				}

				if( yIndex > 0 )
				{
					if( m_voxelData[below].definition->type != VoxelType::AIR )
					{
						m_voxelData[below].SetSideVisibility( Voxel::TOP, true );
					}

				} else if( m_neighborBottom && m_neighborBottom->IsBuilt() )
				{
					VoxelData* voxelBottom = m_neighborBottom->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, (m_chunkDimensions.y * voxelSize) - voxelSize, zIndex*voxelSize ) );
					if( voxelBottom != 0 && voxelBottom->definition->type != VoxelType::AIR )
					{
						voxelBottom->SetSideVisibility( Voxel::TOP, true );
						m_neighborBottom->TriggerSurfaceRebuild();
					}
				}

				if( yIndex < (unsigned int)m_chunkDimensions.y - 1 )
				{//check voxel above, in the current chunk
					if( m_voxelData[above].definition->type != VoxelType::AIR )
					{
						m_voxelData[above].SetSideVisibility( Voxel::BOTTOM, true );
					}

				} else if( m_neighborTop && m_neighborTop->IsBuilt() )
				{//check voxel above, which is in the chunk above
					VoxelData* voxelAbove = m_neighborTop->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, 0.0, zIndex*voxelSize ) );
					if( voxelAbove != 0 && voxelAbove->definition->type != VoxelType::AIR )
					{
						voxelAbove->SetSideVisibility( Voxel::BOTTOM, true );
						m_neighborTop->TriggerSurfaceRebuild();
					}
				}

				if( zIndex > 0 )
				{
					if( m_voxelData[back].definition->type != VoxelType::AIR )
					{
						m_voxelData[back].SetSideVisibility( Voxel::FRONT, true );
					}

				} else if( m_neighborBack && m_neighborBack->IsBuilt() )
				{
					VoxelData* voxelBack = m_neighborBack->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, yIndex*voxelSize, (m_chunkDimensions.z * voxelSize) - voxelSize ) );
					if( voxelBack != 0 && voxelBack->definition->type != VoxelType::AIR )
					{
						voxelBack->SetSideVisibility( Voxel::FRONT, true );
						m_neighborBack->TriggerSurfaceRebuild();
					}
				}

				if( zIndex < m_chunkDimensions.z - 1 )
				{//check voxel in front, in the current chunk
					if( m_voxelData[front].definition->type != VoxelType::AIR )
					{
						m_voxelData[front].SetSideVisibility( Voxel::BACK, true );
					}

				} else if( m_neighborFront && m_neighborFront->IsBuilt() )
				{//check voxel in front, which is in the chunk in front
					VoxelData* voxelFront = m_neighborFront->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, yIndex*voxelSize, 0.0 ) );
					if( voxelFront != 0 && voxelFront->definition->type != VoxelType::AIR )
					{
						voxelFront->SetSideVisibility( Voxel::BACK, true );
						m_neighborFront->TriggerSurfaceRebuild();
					}
				}
			}

			this->TriggerSurfaceRebuild();
		}
	}

}

void VoxelChunk::RemoveVoxelAtPosition( const Kiwi::Vector3d& position )
{

	if( !m_built || (m_voxelData.size() == 0 && m_surfaceVoxels.size() == 0) )
	{
		return;
	}

	m_surfaceVoxels.erase( position );

	if( m_voxelData.size() > 0 )
	{
		double voxelSize = m_terrain->GetVoxelSize();

		//convert position to index in data array
		unsigned int xIndex = (unsigned int)((position.x - m_position.x) / voxelSize);
		unsigned int yIndex = (unsigned int)((position.y - m_position.y) / voxelSize);
		unsigned int zIndex = (unsigned int)((position.z - m_position.z) / voxelSize);

		unsigned int voxelIndex = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
		if( voxelIndex >= m_voxelData.size() )
		{
			throw Kiwi::Exception( L"VoxelChunk::ReplaceVoxelAtPosition", L"Out of bounds index (" + Kiwi::ToWString( voxelIndex ) + L" in chunk " + m_position.ToString() );
		}

		m_voxelData[voxelIndex].definition = m_terrain->GetVoxelDefinition( VoxelType::AIR );
		m_voxelData[voxelIndex].visibility = 0;

		int left = (xIndex - 1) + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x* (unsigned int)m_chunkDimensions.y) * zIndex);
		int right = (xIndex + 1) + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
		int below = xIndex + ((unsigned int)m_chunkDimensions.x * (yIndex - 1)) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
		int above = xIndex + ((unsigned int)m_chunkDimensions.x * (yIndex + 1)) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
		int back = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * (zIndex - 1));//voxelIndex - ((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y);
		int front = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * (zIndex + 1));//voxelIndex + ((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y);

		if( xIndex > 0 )
		{//check voxel to the left, in the current chunk
			if( m_voxelData[left].definition->type != VoxelType::AIR )
			{
				m_voxelData[left].SetSideVisibility( Voxel::RIGHT, true );
			}

		} else
		{//check voxel to the left, which is in the chunk to the left
			if( m_neighborLeft != 0 && m_neighborLeft->IsBuilt() )
			{
				VoxelData* voxelLeft = m_neighborLeft->GetVoxelDataAtPosition( Kiwi::Vector3d( (m_chunkDimensions.x * voxelSize) - voxelSize, yIndex*voxelSize, zIndex*voxelSize ) );
				if( voxelLeft != 0 && voxelLeft->definition->type != VoxelType::AIR )
				{
					voxelLeft->SetSideVisibility( Voxel::RIGHT, true );
					m_neighborLeft->TriggerSurfaceRebuild();
				}
			}
		}

		if( xIndex < (unsigned int)m_chunkDimensions.x - 1 )
		{//check voxel to the right, in the current chunk
			
			if( m_voxelData[right].definition->type != VoxelType::AIR )
			{
				m_voxelData[right].SetSideVisibility( Voxel::LEFT, true );
			}

		} else if( m_neighborRight )
		{//check voxel to the right, which is in the chunk to the right
			if( m_neighborRight != 0 && m_neighborRight->IsBuilt() )
			{
				VoxelData* voxelRight = m_neighborRight->GetVoxelDataAtPosition( Kiwi::Vector3d( 0.0, yIndex*voxelSize, zIndex*voxelSize ) );
				if( voxelRight != 0 && voxelRight->definition->type != VoxelType::AIR )
				{
					voxelRight->SetSideVisibility( Voxel::LEFT, true );
					m_neighborRight->TriggerSurfaceRebuild();
				}
			}
		}

		if( yIndex > 0 )
		{
			if( m_voxelData[below].definition->type != VoxelType::AIR )
			{
				m_voxelData[below].SetSideVisibility( Voxel::TOP, true );
			}

		} else if( m_neighborBottom && m_neighborBottom->IsBuilt() )
		{
			VoxelData* voxelBottom = m_neighborBottom->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, (m_chunkDimensions.y * voxelSize) - voxelSize, zIndex*voxelSize ) );
			if( voxelBottom != 0 && voxelBottom->definition->type != VoxelType::AIR )
			{
				voxelBottom->SetSideVisibility( Voxel::TOP, true );
				m_neighborBottom->TriggerSurfaceRebuild();
			}
		}

		if( yIndex < (unsigned int)m_chunkDimensions.y - 1 )
		{//check voxel above, in the current chunk
			if( m_voxelData[above].definition->type != VoxelType::AIR )
			{
				m_voxelData[above].SetSideVisibility( Voxel::BOTTOM, true );
			}

		} else if( m_neighborTop && m_neighborTop->IsBuilt() )
		{//check voxel above, which is in the chunk above
			VoxelData* voxelAbove = m_neighborTop->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, 0.0, zIndex*voxelSize ) );
			if( voxelAbove != 0 && voxelAbove->definition->type != VoxelType::AIR )
			{
				voxelAbove->SetSideVisibility( Voxel::BOTTOM, true );
				m_neighborTop->TriggerSurfaceRebuild();
			}
		}

		if( zIndex > 0 )
		{
			if( m_voxelData[back].definition->type != VoxelType::AIR )
			{
				m_voxelData[back].SetSideVisibility( Voxel::FRONT, true );
			}

		} else if( m_neighborBack && m_neighborBack->IsBuilt() )
		{
			VoxelData* voxelBack = m_neighborBack->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, yIndex*voxelSize, (m_chunkDimensions.z * voxelSize) - voxelSize ) );
			if( voxelBack != 0 && voxelBack->definition->type != VoxelType::AIR )
			{
				voxelBack->SetSideVisibility( Voxel::FRONT, true );
				m_neighborBack->TriggerSurfaceRebuild();
			}
		}

		if( zIndex < m_chunkDimensions.z - 1 )
		{//check voxel in front, in the current chunk
			if( m_voxelData[front].definition->type != VoxelType::AIR )
			{
				m_voxelData[front].SetSideVisibility( Voxel::BACK, true );
			}

		} else if( m_neighborFront && m_neighborFront->IsBuilt() )
		{//check voxel in front, which is in the chunk in front
			VoxelData* voxelFront = m_neighborFront->GetVoxelDataAtPosition( Kiwi::Vector3d( xIndex*voxelSize, yIndex*voxelSize, 0.0 ) );
			if( voxelFront != 0 && voxelFront->definition->type != VoxelType::AIR )
			{
				voxelFront->SetSideVisibility( Voxel::BACK, true );
				m_neighborFront->TriggerSurfaceRebuild();
			}
		}

		m_playerModified = true;

		this->TriggerSurfaceRebuild();

	}

}

/*returns the voxel at the given position*/
Voxel* VoxelChunk::FindSurfaceVoxelAtPosition( const Kiwi::Vector3d& position )
{

	auto voxelItr = m_surfaceVoxels.find( Kiwi::Vector3d( position.x, position.y, position.z ) );
	if( voxelItr != m_surfaceVoxels.end() )
	{
		return voxelItr->second;
	}

	return 0;

}

Voxel* VoxelChunk::FindSurfaceVoxelUnderPosition( const Kiwi::Vector3d& position )
{

	if( m_surfaceVoxels.size() == 0 )
	{
		return 0;
	}

	double voxelSize = m_terrain->GetVoxelSize();

	//round position to nearest (lower) voxel
	double rx = Kiwi::RoundToNearestd( position.x, voxelSize );
	double ry = Kiwi::RoundToNearestd( position.y, voxelSize );
	double rz = Kiwi::RoundToNearestd( position.z, voxelSize );

	double height = ry - voxelSize; //set height to position immediately under the given position

	for( ; height >= m_position.y; height -= voxelSize )
	{
		auto voxelItr = m_surfaceVoxels.find( Kiwi::Vector3d( rx, height, rz ) );
		if( voxelItr != m_surfaceVoxels.end() )
		{
			return voxelItr->second;
		}
	}

	return 0;

}

Voxel* VoxelChunk::FindSurfaceVoxelAbovePosition( const Kiwi::Vector3d& position )
{

	if( m_surfaceVoxels.size() == 0 )
	{
		return 0;
	}

	double voxelSize = m_terrain->GetVoxelSize();

	//round position to nearest (lower) voxel
	double rx = Kiwi::RoundToNearestd( position.x, voxelSize );
	double ry = Kiwi::RoundToNearestd( position.y, voxelSize );
	double rz = Kiwi::RoundToNearestd( position.z, voxelSize );

	return this->_FindNextAbovePoint( Kiwi::Vector3d( rx, ry, rz ) );

}

Voxel* VoxelChunk::FindWalkableAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int minClearance, unsigned char maxSolidity, unsigned int maxHeight )
{

	if( m_surfaceVoxels.size() == 0 )
	{
		return 0;
	}

	double voxelSize = m_terrain->GetVoxelSize();

	//round position to nearest (lower) voxel
	double rx = Kiwi::RoundToNearestd( globalPos.x, voxelSize );
	double ry = Kiwi::RoundToNearestd( globalPos.y, voxelSize );
	double rz = Kiwi::RoundToNearestd( globalPos.z, voxelSize );

	unsigned int count = 0;
	if( maxHeight == 0 )
	{
		count = UINT_MAX;
	}
	for( double height = globalPos.y; height < m_position.y + (m_chunkDimensions.y * voxelSize) && count < maxHeight; height += voxelSize, count++ )
	{

		Voxel* v = this->_FindNextAbovePoint( Kiwi::Vector3d( rx, height, rz ) );
		if( v == 0 )
		{
			return 0;
		}

		bool walkable = true;
		for( unsigned int i = 0; i < minClearance; i++ )
		{
			auto itr = m_surfaceVoxels.find( Kiwi::Vector3d( rx, v->GetPosition().y + voxelSize, rz ) );
			if( itr != m_surfaceVoxels.end() )
			{
				Voxel* vAbove = itr->second;
				if( vAbove == 0 ) break;

				if( vAbove->GetDefinition() == 0 || vAbove->GetDefinition()->solidity > maxSolidity )
				{
					walkable = false;
					break;
				}
			}
		}

		if( walkable )
		{
			return v;
		}
	}

	return 0;

}

std::vector<Voxel*> VoxelChunk::FindVoxelsAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int numVoxels )
{

	std::vector<Voxel*> voxels;

	if( m_surfaceVoxels.size() == 0 )
	{
		return voxels;
	}

	double voxelSize = m_terrain->GetVoxelSize();

	//round position to nearest (lower) voxel
	double rx = Kiwi::RoundToNearestd( globalPos.x, voxelSize );
	double ry = Kiwi::RoundToNearestd( globalPos.y, voxelSize );
	double rz = Kiwi::RoundToNearestd( globalPos.z, voxelSize );

	unsigned int count = 0;
	for( double height = globalPos.y + voxelSize; height < m_position.y + (m_chunkDimensions.y * voxelSize) && count < numVoxels; height += voxelSize, count++ )
	{
		auto itr = m_surfaceVoxels.find( Kiwi::Vector3d( rx, height, rz ) );
		if( itr != m_surfaceVoxels.end() )
		{
			voxels.push_back( itr->second );
		} else
		{//position lays outside the chunk
			voxels.push_back( 0 );
		}
	}

	return voxels;

}

/*if there are any voxels at the given position it will return the maximum height
if there are no voxels at the position it will return -1*/
double VoxelChunk::GetMaxHeightAtPosition( const Kiwi::Vector3d& position )
{

	return this->GetMaxHeightAtPosition( Kiwi::Vector2d( position.x, position.z ) );

}

double VoxelChunk::GetMaxHeightAtPosition( const Kiwi::Vector2d& position )
{

	if( m_heightmap.size() == 0 )
	{
		return -1.0;
	}

	double voxelSize = m_terrain->GetVoxelSize();
	unsigned int xIndex = (unsigned int)(Kiwi::RoundToNearestd( position.x, voxelSize ) / voxelSize);
	unsigned int zIndex = (unsigned int)(Kiwi::RoundToNearestd( position.y, voxelSize ) / voxelSize);

	if( xIndex + (zIndex * (unsigned int)m_chunkDimensions.x) < m_heightmap.size() )
	{
		return m_heightmap[xIndex + (zIndex * (unsigned int)m_chunkDimensions.x)];

	} else
	{
		return -1.0;
	}

}

/*returns the height value of the closest non-air voxel under the given position
if the position is above the chunk the y dimension of the chunk is returned instead*/
double VoxelChunk::FindHeightUnderPosition( const Kiwi::Vector3d& position )
{

	if( m_heightmap.size() == 0 || m_voxelData.size() == 0 )
	{
		return -1.0;
	}

	double voxelSize = m_terrain->GetVoxelSize();
	unsigned int xIndex = (unsigned int)(Kiwi::RoundToNearestd( position.x, voxelSize ) / voxelSize);
	unsigned int zIndex = (unsigned int)(Kiwi::RoundToNearestd( position.z, voxelSize ) / voxelSize);

	double height = m_heightmap[xIndex + (zIndex * (unsigned int)m_chunkDimensions.x)]; //start from the highest point
	if( height == -1.0 )
	{
		return height;
	}

	double ry = height;

	//start from the highest point
	if( ry <= position.y )
	{
		return ry;
	}

	for( ; ry >= 0; ry -= voxelSize )
	{
		unsigned int yIndex = (unsigned int)(ry / voxelSize);
		unsigned int voxelIndex = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
		if( voxelIndex >= m_voxelData.size() )
		{
			return -1.0;
		}
		if( ry <= position.y && m_voxelData[voxelIndex].definition->type != VoxelType::AIR )
		{
			return ry;
		}
	}

	return -1.0;

}

VoxelData* VoxelChunk::GetVoxelDataAtPosition( const Kiwi::Vector3d& position )
{

	if( m_voxelData.size() == 0 )
	{
		return 0;
	}

	double voxelSize = m_terrain->GetVoxelSize();
	unsigned int xIndex = (unsigned int)(Kiwi::RoundToNearestd( position.x, voxelSize ) / voxelSize);
	unsigned int yIndex = (unsigned int)(Kiwi::RoundToNearestd( position.y, voxelSize ) / voxelSize);
	unsigned int zIndex = (unsigned int)(Kiwi::RoundToNearestd( position.z, voxelSize ) / voxelSize);

	unsigned int voxelIndex = xIndex + ((unsigned int)m_chunkDimensions.x * yIndex) + (((unsigned int)m_chunkDimensions.x * (unsigned int)m_chunkDimensions.y) * zIndex);
	if( voxelIndex >= m_voxelData.size() )
	{
		return 0;
	}

	return &m_voxelData[voxelIndex];

}

void VoxelChunk::SetPosition( const Kiwi::Vector3d& position )
{

	m_position = position;

	std::lock_guard<std::mutex> meshGuard( m_meshMutex );

	if( m_opaqueSurfaceMesh )
	{
		m_opaqueSurfaceMesh->FindComponent<Kiwi::Transform>()->SetPosition( position );
	}

	for( auto itr = m_transparentSurfaceMeshes.begin(); itr != m_transparentSurfaceMeshes.end(); itr++ )
	{
		if( *itr )
		{
			(*itr)->FindComponent<Kiwi::Transform>()->SetPosition( position );
		}
	}

}

void VoxelChunk::SetActive( bool active )
{

	m_active = active;

	std::lock_guard<std::mutex> meshGuard( m_meshMutex );

	if( m_opaqueSurfaceMesh )
	{
		m_opaqueSurfaceMesh->SetActive( active );
	}

	for( auto itr = m_transparentSurfaceMeshes.begin(); itr != m_transparentSurfaceMeshes.end(); itr++ )
	{
		if( *itr )
		{
			(*itr)->SetActive( active );
		}
	}

}