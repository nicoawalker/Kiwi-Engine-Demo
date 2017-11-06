#include "House.h"
#include "Voxel.h"
#include "VoxelTerrain.h"
#include "City.h"
#include "VoxelChunk.h"

#include <Core\Entity.h>

#include <random>

House::House( City* city, VoxelTerrain* terrain, int voxelSize, short houseType, const Kiwi::Vector3d position, const Kiwi::Vector3d& dimensions ):
	IStructure(voxelSize)
{

	m_city = city;
	m_terrain = terrain;

	m_maxHouseDimensions = dimensions;
	m_boundingVolume = dimensions + Kiwi::Vector3d( 2.0, 2.0, 2.0 );
	m_upperFloorCount = 0;
	m_lowerFloorCount = 0;
	m_npcCount = 0;
	m_houseType = houseType;
	m_position = position;
	m_enableLowerFloors = true;
	m_doorHeight = 2;

	m_minRoomSize.Set( 5.0, 5.0, 5.0 ); //minimum size is 4x4x4 voxels + walls
	m_maxRoomSize.Set( 10.0, 7.0, 10.0 );

	m_houseStyle[AGRND_EXTERIOR] = VoxelType::WHITE_PLASTER;
	m_houseStyle[AGRND_INTERIOR] = VoxelType::WHITE_PLASTER;
	m_houseStyle[AGRND_FLOOR] = VoxelType::WOOD_WALNUT;
	m_houseStyle[UGRND_EXTERIOR] = VoxelType::STONE;
	m_houseStyle[UGRND_FLOOR] = VoxelType::STONE;
	m_houseStyle[UGRND_INTERIOR] = VoxelType::WHITE_PLASTER;
	m_houseStyle[ROOF] = VoxelType::TERRACOTTA;
	m_houseStyle[AGRND_EXTERIOR_TRIM] = VoxelType::WOOD_WALNUT;
	m_houseStyle[UGRND_EXT_TRIM] = VoxelType::STONE;


}

void House::_GenerateRoomsFromLayout()
{

	std::random_device rd;
	std::mt19937 e2( rd() );
	std::uniform_real_distribution<> randomGen( 0.0, 1.0 );

	//stores the rooms on the ground floor which have an exterior wall (in which to place the door)
	std::vector<Room*> exteriorGrndFloor;

	for( auto roomItr = m_roomLayout.begin(); roomItr != m_roomLayout.end(); roomItr++ )
	{
		Room* current = roomItr->second;

		if( current == 0 )
		{
			continue;
		}

		Kiwi::Vector3d right = current->position + Kiwi::Vector3d( 1.0, 0.0, 0.0 );
		Kiwi::Vector3d left = current->position - Kiwi::Vector3d( 1.0, 0.0, 0.0 );
		Kiwi::Vector3d front = current->position + Kiwi::Vector3d( 0.0, 0.0, 1.0 );
		Kiwi::Vector3d back = current->position - Kiwi::Vector3d( 0.0, 0.0, 1.0 );
		Kiwi::Vector3d above = current->position + Kiwi::Vector3d( 0.0, 1.0, 0.0 );

		Kiwi::Vector3d roomPos( current->position.x * (m_roomSize.x - m_voxelSize) * m_voxelSize, current->position.y * (m_roomSize.y - m_voxelSize) * m_voxelSize, current->position.z * (m_roomSize.z - m_voxelSize) * m_voxelSize );

		//correct position for even sized rooms because there is no center voxel
		if( fmod( m_roomSize.x, 2.0 ) == 0 ) roomPos.x -= m_voxelSize;
		if( fmod( m_roomSize.z, 2.0 ) == 0 ) roomPos.z -= m_voxelSize;

		double halfRoomX = ((m_roomSize.x - 1.0) / 2.0) * m_voxelSize;
		double halfRoomZ = ((m_roomSize.z - 1.0) / 2.0) * m_voxelSize;

		//generate the door position for each wall, if this room has a door(s)
		double doorZPos = std::round( randomGen( e2 ) * (m_roomSize.z - 2.0) ) + 2.0;
		double doorXPos = std::round( randomGen( e2 ) * (m_roomSize.x - 2.0) ) + 2.0;

		//there is a room next to this one, there's a 50% chance to generate a wall with a door between the rooms
		bool wallZ = (randomGen( e2 ) <= 0.5) ? true : false;
		bool wallX = (randomGen( e2 ) <= 0.5) ? true : false;

		//if the room is larger than 4 voxels across, theres a chance of making a wide 'archway' instead of a door between rooms
		bool wideOpeningZ = ( m_roomSize.z >= 5.0 && randomGen( e2 ) <= 0.2) ? true : false;
		bool wideOpeningX = ( m_roomSize.x >= 5.0 && randomGen( e2 ) <= 0.2) ? true : false;

		//check if both interior walls are needed
		if( (m_roomLayout.find( front ) != m_roomLayout.end() || m_roomLayout.find( back ) != m_roomLayout.end()) &&
			(m_roomLayout.find( right ) != m_roomLayout.end() || m_roomLayout.find( left ) != m_roomLayout.end()) )
		{
			if( wallZ || wallX )
			{
				wallZ = wallX = true;
			}
		}

		for( double y = 0.0; y < m_roomSize.y; y += 1.0 )
		{
			if( y == 0.0 )
			{//make floor
				double backZ = roomPos.z - std::floor( halfRoomZ );
				double leftX = roomPos.x - std::floor( halfRoomX );
				for( double z = 1.0; z < m_roomSize.z - 1.0; z += 1.0 )
				{
					for( double x = 1.0; x < m_roomSize.x - 1.0; x += 1.0 )
					{
						if( current->position.y < 0.0 )
						{//generate basement floor
							m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y, backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_FLOOR] ) );

						} else
						{//generate above ground floor
							m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y, backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_FLOOR] ) );
						}
					}
				}

			}

			//right wall
			if( m_roomLayout.find( right ) == m_roomLayout.end() )
			{//no room to the right of this one, generate an exterior wall
				double x = roomPos.x + std::ceil(halfRoomX);//roomPos.x + (((m_roomSize.x - 1.0) / 2.0) * m_voxelSize);
				double backZ = roomPos.z - std::floor( halfRoomZ );
				for( double z = 0.0; z < m_roomSize.z; z += 1.0 )
				{
					if( m_structureVoxels.find( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
					{//dont replace already placed voxels
						if( z == 0.0 || y == 0.0 || z == m_roomSize.z - 1.0 || y == m_roomSize.y - 1.0 )
						{//place trim around the outside edges
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_EXT_TRIM] ) );

							} else
							{
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_EXTERIOR_TRIM] ) );
							}

						} else
						{//regular exterior on the inside parts of the wall
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_EXTERIOR] ) );

							} else
							{//generate above ground wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_EXTERIOR] ) );
							}
						}
					}
				}

			} else
			{//generate an interior wall
				double x = roomPos.x + std::ceil( halfRoomX );//roomPos.x + (((m_roomSize.x - 1.0) / 2.0) * m_voxelSize);
				double backZ = roomPos.z - std::floor( halfRoomZ );

				if( wallZ )
				{
					for( double z = 1.0; z < m_roomSize.z; z += 1.0 )
					{
						if( wallX || z < m_roomSize.z - 1.0 )
						{
							if( m_structureVoxels.find( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
							{//dont replace already placed voxels
								if( y == 0 || y == m_roomSize.y - 1.0 )
								{
									if( current->position.y < 0.0 )
									{//generate basement wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_FLOOR] ) );

									} else
									{//generate above ground wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_FLOOR] ) );
									}

								}else if( (wideOpeningZ && (z == 1.0 || z == m_roomSize.z - 2.0 || y == m_roomSize.y - 2.0)) || (!wideOpeningZ && (z != doorZPos || y > m_doorHeight)) )
								{//regular exterior on the inside parts of the wall
									if( current->position.y < 0.0 )
									{//generate basement wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_INTERIOR] ) );

									} else
									{//generate above ground wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_INTERIOR] ) );
									}
								}
							}
						}
					}

				} else if( y == 0.0 )
				{//generate a floor along the bottom of where the wall would have been
					for( double z = 1.0; z < m_roomSize.z - 1.0; z += 1.0 )
					{
						if( m_structureVoxels.find( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
						{//dont replace already placed voxels
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_FLOOR] ) );

							} else
							{//generate above ground wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_FLOOR] ) );
							}
						}
					}

				} else if( y == m_roomSize.y - 1.0 )
				{//generate a ceiling along the top of where the wall would have been
					for( double z = 1.0; z < m_roomSize.z - 1.0; z += 1.0 )
					{
						if( m_structureVoxels.find( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
						{//dont replace already placed voxels
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_FLOOR] ) );

							} else
							{//generate above ground wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_FLOOR] ) );
							}
						}
					}
				}
			}

			//left wall
			if( m_roomLayout.find( left ) == m_roomLayout.end() )
			{//no room to the right of this one, generate an exterior wall
				double x = roomPos.x - std::floor( halfRoomX );//roomPos.x - (((m_roomSize.x - 1.0) / 2.0) * m_voxelSize);
				double backZ = roomPos.z - std::floor( halfRoomZ );
				for( double z = 0.0; z < m_roomSize.z; z += 1.0 )
				{
					if( m_structureVoxels.find( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
					{//dont replace already placed voxels
						if( z == 0.0 || y == 0.0 || z == m_roomSize.z - 1.0 || y == m_roomSize.y - 1.0 )
						{//place trim around the outside edges
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_EXT_TRIM] ) );

							} else
							{
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_EXTERIOR_TRIM] ) );
							}

						} else
						{//regular exterior on the inside parts of the wall
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[UGRND_EXTERIOR] ) );

							} else
							{//generate above ground wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( x, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[AGRND_EXTERIOR] ) );
							}
						}
					}
				}

			}

			//front wall
			if( m_roomLayout.find( front ) == m_roomLayout.end() )
			{//no room to the front of this one, generate an exterior wall
				double z = roomPos.z + std::ceil(halfRoomZ);// roomPos.z + (((m_roomSize.z - 1.0) / 2.0) * m_voxelSize);
				double leftX = roomPos.x - std::floor( halfRoomX );
				for( double x = 0.0; x < m_roomSize.x; x += 1.0 )
				{
					if( m_structureVoxels.find( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ) ) == m_structureVoxels.end() )
					{//dont replace already placed voxels
						if( x == 0.0 || y == 0.0 || x == m_roomSize.x - 1.0 || y == m_roomSize.y - 1.0 )
						{//place trim around the outside edges
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[UGRND_EXT_TRIM] ) );

							} else
							{
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[AGRND_EXTERIOR_TRIM] ) );
							}

						} else
						{//regular exterior on the inside parts of the wall
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[UGRND_EXTERIOR] ) );

							} else
							{//generate above ground wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[AGRND_EXTERIOR] ) );
							}
						}
					}
				}

			} else
			{
				double z = roomPos.z + std::ceil( halfRoomZ );// roomPos.z + (((m_roomSize.z - 1.0) / 2.0) * m_voxelSize);
				double leftX = roomPos.x - std::floor( halfRoomX );
				if( wallX )
				{//there is a room in front of this one, there's a 50% chance for each room to generate a wall with a door between the rooms
					for( double x = 1.0; x < m_roomSize.x; x += 1.0 )
					{
						if( wallZ || x < m_roomSize.x - 1.0 )
						{
							if( m_structureVoxels.find( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ) ) == m_structureVoxels.end() )
							{//dont replace already placed voxels
								if( y == 0 || y == m_roomSize.y - 1.0 )
								{
									if( current->position.y < 0.0 )
									{//generate basement wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[UGRND_FLOOR] ) );

									} else
									{//generate above ground wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[AGRND_FLOOR] ) );
									}

								} else if( (wideOpeningX && (x == 1.0 || x == m_roomSize.x - 2.0 || y == m_roomSize.y - 2.0 )) || (!wideOpeningX && (x != doorXPos || y > m_doorHeight )) )
								{
									if( current->position.y < 0.0 )
									{//generate basement wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[UGRND_EXTERIOR] ) );

									} else
									{//generate above ground wall
										m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[AGRND_EXTERIOR] ) );
									}
								}
							}
						}
					}

				} else if( y == 0.0 || y == m_roomSize.y - 1.0 )
				{//just add in the floor
					for( double x = 1.0; x < m_roomSize.x - 1.0; x += 1.0 )
					{
						if( m_structureVoxels.find( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ) ) == m_structureVoxels.end() )
						{//dont replace already placed voxels
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[UGRND_FLOOR] ) );

							} else
							{//generate above ground wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[AGRND_FLOOR] ) );
							}
						}
					}
				}
			}

			//back wall
			if( m_roomLayout.find( back ) == m_roomLayout.end() )
			{//no room to the back of this one, generate an exterior wall
				double z = roomPos.z - std::floor( halfRoomZ );//roomPos.z - (((m_roomSize.z - 1.0) / 2.0) * m_voxelSize);
				double leftX =  roomPos.x - std::floor(halfRoomX);
				for( double x = 0.0; x < m_roomSize.x; x += 1.0 )
				{
					if( m_structureVoxels.find( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ) ) == m_structureVoxels.end() )
					{//dont replace already placed voxels
						if( x == 0.0 || y == 0.0 || x == m_roomSize.x - 1.0 || y == m_roomSize.y - 1.0 )
						{//place trim around the outside edges
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[UGRND_EXT_TRIM] ) );

							} else
							{
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[AGRND_EXTERIOR_TRIM] ) );
							}

						} else
						{//regular exterior on the inside parts of the wall
							if( current->position.y < 0.0 )
							{//generate basement wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[UGRND_EXTERIOR] ) );

							} else
							{//generate above ground wall
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), z ), m_houseStyle[AGRND_EXTERIOR] ) );
							}
						}
					}
				}
			}
		}
	}

}

void House::_GenerateRoof()
{

	//generate a roof on top of each room with space above it
	for( auto roomItr = m_roomLayout.begin(); roomItr != m_roomLayout.end(); roomItr++ )
	{
		Room* current = roomItr->second;

		if( current == 0 )
		{
			continue;
		}

		Kiwi::Vector3d right = current->position + Kiwi::Vector3d( 1.0, 0.0, 0.0 );
		Kiwi::Vector3d left = current->position - Kiwi::Vector3d( 1.0, 0.0, 0.0 );
		Kiwi::Vector3d front = current->position + Kiwi::Vector3d( 0.0, 0.0, 1.0 );
		Kiwi::Vector3d back = current->position - Kiwi::Vector3d( 0.0, 0.0, 1.0 );
		Kiwi::Vector3d above = current->position + Kiwi::Vector3d( 0.0, 1.0, 0.0 );

		//local position of the center of the room (y coordinate is the bottom of the room)
		Kiwi::Vector3d roomPos( current->position.x * (m_roomSize.x - m_voxelSize) * m_voxelSize, current->position.y * (m_roomSize.y - m_voxelSize) * m_voxelSize, current->position.z * (m_roomSize.z - m_voxelSize) * m_voxelSize );

		//correct position for even sized rooms because there is no center voxel
		if( fmod( m_roomSize.x, 2.0 ) == 0 ) roomPos.x -= m_voxelSize;
		if( fmod( m_roomSize.z, 2.0 ) == 0 ) roomPos.z -= m_voxelSize;

		double halfRoomX = ((m_roomSize.x - 1.0) / 2.0) * m_voxelSize;
		double halfRoomZ = ((m_roomSize.z - 1.0) / 2.0) * m_voxelSize;

		double roofWidth = m_roomSize.x;
		double roofDepth = m_roomSize.z;

		double overhangDepth = m_voxelSize; //depth of the overhang around the house

		//if the room next to this one has a roof in a different direction, this roof needs to be extended to merge with the other one
		bool mergeLeft = false, mergeRight = false, mergeForward = false, mergeBackward = false;

		//if there's no room on top of this one, make a roof
		if( m_roomLayout.find( above ) == m_roomLayout.end() )
		{
			double backZ = roomPos.z - std::floor( halfRoomZ ) - overhangDepth;
			if( m_roomLayout.find( current->position + Kiwi::Vector3d( 0.0, 1.0, -1.0 ) ) == m_roomLayout.end() )
			{//create an overhang only if it wont collide with another room
				//backZ -= m_voxelSize;
				roofDepth++;

				//check whether to merge with a room behind this one
				auto backItr = m_roomLayout.find( back );
				if( backItr != m_roomLayout.end())
				{
					mergeBackward = true;
				}
			}
			double leftX = roomPos.x - std::floor( halfRoomX ) - overhangDepth;
			if( m_roomLayout.find( current->position + Kiwi::Vector3d( -1.0, 1.0, 0.0 ) ) == m_roomLayout.end() )
			{//create an overhang only if it wont collide with another room
				//leftX -= m_voxelSize;
				roofWidth++;

				//check whether to merge with a room to the left of this one
				auto leftItr = m_roomLayout.find( left );
				if( leftItr != m_roomLayout.end() )
				{
					mergeLeft = true;
				}
			}
			double frontZ = roomPos.z + std::ceil( halfRoomZ ) + overhangDepth;
			if( m_roomLayout.find( current->position + Kiwi::Vector3d( 0.0, 1.0, 1.0 ) ) == m_roomLayout.end() )
			{//create an overhang only if it wont collide with another room
				//frontZ += m_voxelSize;
				roofDepth++;

				//check whether to merge with a room in front of this one
				auto frontItr = m_roomLayout.find( front );
				if( frontItr != m_roomLayout.end() )
				{
					mergeForward = true;
				}
			}
			double rightX = roomPos.x + std::ceil( halfRoomX ) + overhangDepth;
			if( m_roomLayout.find( current->position + Kiwi::Vector3d( 1.0, 1.0, 0.0 ) ) == m_roomLayout.end() )
			{//create an overhang only if it wont collide with another room
				//rightX += m_voxelSize;
				roofWidth++;

				//check whether to merge with a room to the right of this one
				auto rightItr = m_roomLayout.find( right );
				if( rightItr != m_roomLayout.end() )
				{
					mergeRight = true;
				}
			}
			
			//make a ceiling for the room
			double ceilBackZ = roomPos.z - std::floor( halfRoomZ );
			double ceilLeftX = roomPos.x - std::floor( halfRoomX );
			for( double z = 1.0; z < m_roomSize.z - 1.0; z += 1.0 )
			{
				for( double x = 1.0; x < m_roomSize.x - 1.0; x += 1.0 )
				{
					m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( ceilLeftX + (x * m_voxelSize), roomPos.y + ((m_roomSize.y - 1.0) * m_voxelSize), ceilBackZ + (z * m_voxelSize) ), m_houseStyle[AGRND_FLOOR] ) );
				}
			}

			//create the roof
			if( m_roomLayout.find( right ) == m_roomLayout.end() &&
				m_roomLayout.find( left ) == m_roomLayout.end() &&
				m_roomLayout.find( front ) == m_roomLayout.end() &&
				m_roomLayout.find( back ) == m_roomLayout.end() )
			{//this room has no neighbors so generate a pyramid shaped roof

				double maxRoofHeight = m_roomSize.y - 1.0 + min( std::ceil( min( m_roomSize.x, m_roomSize.z ) / 2.0 ), m_roomSize.y );

				for( double y = m_roomSize.y - 1.0, level = 0.0; y < maxRoofHeight; y += 1.0, level += 1.0 )
				{
					if( y == maxRoofHeight - 1.0 )
					{//for the top layer of the roof make a solid flat sheet instead of a ring
						for( double z = 0.0; z < roofDepth - (2 * level); z += 1.0 )
						{
							for( double x = 0.0; x < roofWidth - (2 * level); x += 1.0 )
							{
								if( m_structureVoxels.find( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
								{//dont replace already placed voxels
									m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[ROOF] ) );
								}
							}
						}

					} else
					{
						for( double z = 0.0; z < roofDepth - (2 * level); z += 1.0 )
						{
							if( m_structureVoxels.find( Kiwi::Vector3d( leftX, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
							{//dont replace already placed voxels
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[ROOF] ) );
							}

							if( m_structureVoxels.find( Kiwi::Vector3d( rightX, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ) ) == m_structureVoxels.end() )
							{//dont replace already placed voxels
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( rightX, roomPos.y + (y * m_voxelSize), backZ + (z * m_voxelSize) ), m_houseStyle[ROOF] ) );
							}
						}
						for( double x = 0.0; x < roofWidth - (2 * level); x += 1.0 )
						{
							if( m_structureVoxels.find( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), backZ ) ) == m_structureVoxels.end() )
							{//dont replace already placed voxels
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), backZ ), m_houseStyle[ROOF] ) );
							}

							if( m_structureVoxels.find( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), frontZ ) ) == m_structureVoxels.end() )
							{//dont replace already placed voxels
								m_structureVoxels.insert( std::make_pair( Kiwi::Vector3d( leftX + (x * m_voxelSize), roomPos.y + (y * m_voxelSize), frontZ ), m_houseStyle[ROOF] ) );
							}
						}
					}

					backZ += m_voxelSize;
					leftX += m_voxelSize;
					frontZ -= m_voxelSize;
					rightX -= m_voxelSize;
				}

			} else
			{//make a normal A shaped roof

				double roofHeight = max( std::ceil( m_roomSize.z / 2.0 ), 3.0 );

				if( current->roofDirection == 0 )
				{//roof oriented along the x axis
					//left wall starts from the back left corner
					Kiwi::Vector2d leftWallPos( roomPos.x - (std::floor( halfRoomX ) * m_voxelSize), roomPos.z - (std::floor( halfRoomZ ) * m_voxelSize) );

					for( double level = 0.0, y = (m_roomSize.y - 1.0) * m_voxelSize; level <= roofHeight; level++, y += m_voxelSize )
					{
						if( level == roofHeight )
						{//if this was the last level, add the top layer to complete the roof
							for( double zPos = 0.0; zPos < (m_roomSize.z - ((level - 1.0) * 2.0)) * m_voxelSize; zPos += m_voxelSize )
							{//make the walls under the roof on either side
								for( double xPos = 0.0; xPos < rightX - leftX + m_voxelSize; xPos += m_voxelSize )
								{
									//left
									this->_PlaceVoxel( ROOF, Kiwi::Vector3d( leftX + xPos, roomPos.y + y, backZ + (level * m_voxelSize) + zPos ) );
									//right
									this->_PlaceVoxel( ROOF, Kiwi::Vector3d( leftX + xPos, roomPos.y + y, frontZ - (level * m_voxelSize) + zPos ) );
								}
							}

						} else
						{
							//make both sides of the roof
							for( double xPos = 0.0; xPos < rightX - leftX + m_voxelSize; xPos += m_voxelSize )
							{
								//left
								this->_PlaceVoxel( ROOF, Kiwi::Vector3d( leftX + xPos, roomPos.y + y, backZ + (level * m_voxelSize) ) );
								//right
								this->_PlaceVoxel( ROOF, Kiwi::Vector3d( leftX + xPos, roomPos.y + y, frontZ - (level * m_voxelSize) ) );
							}

							if( level >= 1.0 )
							{
								for( double zPos = m_voxelSize; zPos < (m_roomSize.z - 1.0 - ((level - 1.0) * 2.0)) * m_voxelSize; zPos += m_voxelSize )
								{//make the walls under the roof on either side
									if( !mergeLeft )
									{//dont replace already placed voxels
										//left wall
										this->_PlaceVoxel( AGRND_EXTERIOR, Kiwi::Vector3d( leftWallPos.x, roomPos.y + y, leftWallPos.y + zPos + ((level - 1.0) * m_voxelSize) ) );
									}
									if( !mergeRight )
									{
										//right wall
										this->_PlaceVoxel( AGRND_EXTERIOR, Kiwi::Vector3d( leftWallPos.x + m_roomSize.x - m_voxelSize, roomPos.y + y, leftWallPos.y + zPos + ((level - 1.0) * m_voxelSize) ) );
									}
								}
							}

							if( level > 1.0 )
							{
								if( mergeLeft )
								{
									leftX -= m_voxelSize;
								}
								if( mergeRight )
								{
									rightX += m_voxelSize;
								}
							}
						}
					}


				}else
				{//roof oriented along the z axis
					//left wall starts from the back left corner
					Kiwi::Vector2d leftWallPos( roomPos.x - (std::floor( halfRoomX ) * m_voxelSize), roomPos.z - (std::floor( halfRoomZ ) * m_voxelSize) );

					for( double level = 0.0, y = (m_roomSize.y - 1.0) * m_voxelSize; level <= roofHeight; level++, y += m_voxelSize )
					{
						if( level == roofHeight )
						{//if this was the last level, add the top layer to complete the roof
							//for( double zPos = 0.0; zPos < (m_roomSize.z - ((level - 1.0) * 2.0)) * m_voxelSize; zPos += m_voxelSize )
							//{//make the walls under the roof on either side
							//	for( double xPos = 0.0; xPos < rightX - leftX + m_voxelSize; xPos += m_voxelSize )
							//	{
							//		//left
							//		this->_PlaceVoxel( ROOF, Kiwi::Vector3d( leftX + xPos, roomPos.y + y, backZ + (level * m_voxelSize) + zPos ) );
							//		//right
							//		this->_PlaceVoxel( ROOF, Kiwi::Vector3d( leftX + xPos, roomPos.y + y, frontZ - (level * m_voxelSize) + zPos ) );
							//	}
							//}

						} else
						{
							//make both sides of the roof
							for( double zPos = 0.0; zPos < frontZ - backZ + m_voxelSize; zPos += m_voxelSize )
							{
								//left
								this->_PlaceVoxel( ROOF, Kiwi::Vector3d( leftX + (level * m_voxelSize), roomPos.y + y, backZ + zPos ) );
								//right
								this->_PlaceVoxel( ROOF, Kiwi::Vector3d( rightX - (level * m_voxelSize), roomPos.y + y, frontZ - zPos ) );
							}

							if( level >= 1.0 )
							{
								for( double xPos = m_voxelSize; xPos < (m_roomSize.x - 1.0 - ((level - 1.0) * 2.0)) * m_voxelSize; xPos += m_voxelSize )
								{//make the walls under the roof on either side
									if( !mergeForward )
									{//dont replace already placed voxels
									 //front wall
										this->_PlaceVoxel( AGRND_EXTERIOR, Kiwi::Vector3d( leftWallPos.x + xPos + ((level - 1.0) * m_voxelSize), roomPos.y + y, leftWallPos.y + m_roomSize.z - m_voxelSize ) );
									}
									if( !mergeBackward )
									{
										//rear wall
										this->_PlaceVoxel( AGRND_EXTERIOR, Kiwi::Vector3d( leftWallPos.x + xPos + ((level - 1.0) * m_voxelSize), roomPos.y + y, leftWallPos.y ) );
									}
								}
							}

							if( level > 1.0 )
							{
								if( mergeForward )
								{
									frontZ += m_voxelSize;
								}
								if( mergeBackward )
								{
									backZ -= m_voxelSize;
								}
							}
						}
					}


				}
			}
		}
	}

}

void House::_GenerateRoomLayout( const Kiwi::Vector3d& dimensions )
{

	std::random_device rd;
	std::mt19937 e2( rd() );
	std::uniform_real_distribution<> randomGen( 0.0, 1.0 );

	std::deque<Room*> frontier;
	std::unordered_set<Kiwi::Vector3d, Kiwi::Vector3dHash, Kiwi::Vector3dEquality> explored;
	frontier.push_back( new Room( Kiwi::Vector3d( 0.0, 0.0, 0.0 ), 0 ) );
	explored.insert( Kiwi::Vector3d( 0.0, 0.0, 0.0 ) );

	while( frontier.size() > 0 )
	{
		Room* current = frontier.front();
		frontier.pop_front();

		if( current == 0 )
		{
			continue;
		}

		m_roomLayout.insert( std::make_pair( current->position, current ) );

		Kiwi::Vector3d right = current->position + Kiwi::Vector3d( 1.0, 0.0, 0.0 );
		Kiwi::Vector3d left = current->position - Kiwi::Vector3d( 1.0, 0.0, 0.0 );
		Kiwi::Vector3d front = current->position + Kiwi::Vector3d( 0.0, 0.0, 1.0 );
		Kiwi::Vector3d back = current->position - Kiwi::Vector3d( 0.0, 0.0, 1.0 );

		//generate upper floors
		for( unsigned int i = 0; i < m_upperFloorCount; i++ )
		{
			//odds of generating a room are 50% - (15% * floor)
			if( randomGen( e2 ) <= 0.5 - (0.15 * i) )
			{
				Room* newRoom = new Room( current->position + Kiwi::Vector3d( 0.0, i + 1.0, 0.0 ), (int)current->position.x % 2 );
				m_roomLayout.insert( std::make_pair( newRoom->position, newRoom ) );

			} else
			{//can't build the next room if this one doesnt exist
				break;
			}
		}

		//generate lower floors
		for( unsigned int i = 0; i < m_lowerFloorCount; i++ )
		{
			//odds of generating a room are 10% - (2% * floor)
			if( randomGen( e2 ) <= 0.1 - (0.02 * i) )
			{
				Room* newRoom = new Room( current->position - Kiwi::Vector3d( 0.0, i + 1.0, 0.0 ), (int)current->position.x % 2 );
				m_roomLayout.insert( std::make_pair( newRoom->position, newRoom ) );

			} else
			{//can't build the next room if this one doesnt exist
				break;
			}
		}

		if( current->position.x > -std::floor( (dimensions.x - 1.0) / 2.0 ) && explored.find( left ) == explored.end() )
		{
			//odds of generating a room are 70% - (15% * distance from origin)
			if( randomGen( e2 ) <= 0.7 - (0.15 * std::abs(left.x)) )
			{
				frontier.push_back( new Room( left, (int)current->position.x % 2 ) );
			}
			explored.insert( left );
		}
		if( current->position.x < std::ceil( (dimensions.x - 1.0) / 2.0 ) && explored.find( right ) == explored.end() )
		{
			if( randomGen( e2 ) <= 0.7 - (0.15 * std::abs( right.x )) )
			{
				frontier.push_back( new Room( right, (int)current->position.x % 2 ) );
			}
			explored.insert( right );
		}
		if( current->position.z > -std::floor( (dimensions.z - 1.0) / 2.0 ) && explored.find( back ) == explored.end() )
		{
			if( randomGen( e2 ) <= 0.7 - (0.15 * std::abs( back.z )) )
			{
				frontier.push_back( new Room( back, (int)current->position.z % 2 ) );
			}
			explored.insert( back );
		}
		if( current->position.z < std::ceil( (dimensions.z - 1.0) / 2.0 ) && explored.find( front ) == explored.end() )
		{
			if( randomGen( e2 ) <= 0.7 - (0.15 * std::abs( front.z )) )
			{
				frontier.push_back( new Room( front, (int)current->position.z % 2 ) );
			}
			explored.insert( front );
		}

	}

}

void House::_PlaceVoxel( HOUSE_ELEMENT voxelType, const Kiwi::Vector3d& localPos, bool replaceExisting )
{

	if( replaceExisting || m_structureVoxels.find( localPos ) == m_structureVoxels.end() )
	{//dont replace already placed voxels
		m_structureVoxels.insert( std::make_pair( localPos, m_houseStyle[voxelType] ) );
	}

}

bool House::Generate()
{

	//make sure house is at least the minimum size
	if( m_maxHouseDimensions.y < m_minRoomSize.y ||
		m_maxHouseDimensions.x < m_minRoomSize.x || 
		m_maxHouseDimensions.z < m_minRoomSize.z )
	{
		return false;
	}

	std::random_device rd;
	std::mt19937 e2( rd() );

	//randomly generate the size of each room
	std::uniform_real_distribution<> roomDimX( max(m_minRoomSize.x, std::floor(m_maxHouseDimensions.x / 4.0)), min( m_maxHouseDimensions.x, m_maxRoomSize.x ) );
	std::uniform_real_distribution<> roomDimY( max( m_minRoomSize.y, std::floor( m_maxHouseDimensions.y / 4.0 ) ), min( m_maxHouseDimensions.y, m_maxRoomSize.y ) );
	std::uniform_real_distribution<> roomDimZ( max( m_minRoomSize.y, std::floor( m_maxHouseDimensions.y / 4.0 ) ), min( m_maxHouseDimensions.z, m_maxRoomSize.z ) );
	m_roomSize.Set( std::floor( roomDimX( e2 ) ), std::floor( roomDimY( e2 ) ), std::floor( roomDimZ( e2 ) ) );
	if( fmod( m_roomSize.x, 2.0 ) == 0 )
	{
		if( m_roomSize.x == m_maxHouseDimensions.x )
		{
			m_roomSize.x -= 1.0;

		} else
		{
			m_roomSize.x += 1.0;
		}
	}

	if( fmod( m_roomSize.z, 2.0 ) == 0 )
	{
		if( m_roomSize.z == m_maxHouseDimensions.z )
		{
			m_roomSize.z -= 1.0;

		} else
		{
			m_roomSize.z += 1.0;
		}
	}

	std::uniform_real_distribution<> floorRand(0.0, 3.0);
	m_upperFloorCount = (int)floorRand( e2 );

	if( m_enableLowerFloors )
	{
		m_lowerFloorCount = (int)floorRand( e2 );
	}

	//calculate max number of rooms in the house for each axis
	Kiwi::Vector3d maxRooms( std::floor( (m_maxHouseDimensions.x) / m_roomSize.x ), std::floor( m_maxHouseDimensions.y / m_roomSize.y ), std::floor( (m_maxHouseDimensions.z) / m_roomSize.z ) );

	//randomly generate the rooms
	this->_GenerateRoomLayout( maxRooms );
	this->_GenerateRoomsFromLayout();
	this->_GenerateRoof();

	//find the actual dimensions of the generated house
	Kiwi::Vector3d min;
	Kiwi::Vector3d max;
	for( auto voxelItr = m_structureVoxels.begin(); voxelItr != m_structureVoxels.end(); voxelItr++ )
	{
		Kiwi::Vector3d voxPos = voxelItr->first;

		min.x = (voxPos.x < min.x) ? voxPos.x : min.x;
		max.x = (voxPos.x > max.x) ? voxPos.x : max.x;
		min.y = (voxPos.y < min.y) ? voxPos.y : min.y;
		max.y = (voxPos.y > max.y) ? voxPos.y : max.y;
		min.z = (voxPos.z < min.z) ? voxPos.z : min.z;
		max.z = (voxPos.z > max.z) ? voxPos.z : max.z;
	}

	m_houseDimensions = Kiwi::Vector3d(1.0, 1.0, 1.0) + max - min;

	if( m_yardDimensions == Kiwi::Vector3d( 0.0, 0.0, 0.0 ) )
	{
		m_yardDimensions = m_houseDimensions;
	}

	return true;

}

void House::PlaceVoxels( VoxelChunk* chunk )
{

	if( chunk == 0 || m_terrain == 0 )
	{
		return;
	}

	//first find the average height under the house
	double averageHeight = -1.0;
	double voxelCount = 0.0;
	double voxelSize = m_terrain->GetVoxelSize();

	for( double x = 0.0; x < m_yardDimensions.x; x++ )
	{
		for( double z = 0.0; z < m_yardDimensions.z; z++ )
		{
			double height = m_terrain->FindMaxHeightAtPosition( Kiwi::Vector2d( m_position.x, m_position.z ) ) + voxelSize;
			averageHeight += height;
			voxelCount++;
			continue;
		}
	}

	if( averageHeight >= 0.0 && voxelCount != 0.0 )
	{
		averageHeight /= voxelCount;

	} else
	{
		return;
	}

	//clear out a space for the house starting at the average height
	for( double y = 0.0; y < m_yardDimensions.y; y++ )
	{
		for( double x = -std::floor(m_yardDimensions.x / 2.0); x < std::floor(m_yardDimensions.x / 2.0); x++ )
		{
			for( double z = -std::floor(m_yardDimensions.z / 2.0); z < std::floor(m_yardDimensions.z / 2.0); z++ )
			{
				chunk->ReplaceVoxelAtPosition( Kiwi::Vector3d( m_position.x + x, averageHeight + (y * voxelSize), m_position.z + z ), VoxelType::AIR );
			}
		}
	}

	for( auto voxelItr = m_structureVoxels.begin(); voxelItr != m_structureVoxels.end(); voxelItr++ )
	{
		chunk->ReplaceVoxelAtPosition( voxelItr->first + Kiwi::Vector3d(m_position.x, averageHeight, m_position.z), voxelItr->second );
	}

}