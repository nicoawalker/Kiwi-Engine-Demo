#ifndef _HOUSE_H_
#define _HOUSE_H_

#include "IStructure.h"
#include "Voxel.h"

#include <Core\Entity.h>
#include <Core\Transform.h>

#include <unordered_set>
#include <string>
#include <random>

class City;
class VoxelChunk;
class VoxelTerrain;

enum HOUSE_ELEMENT { AGRND_FLOOR, AGRND_EXTERIOR, AGRND_INTERIOR, UGRND_EXTERIOR, UGRND_INTERIOR, UGRND_FLOOR, ROOF, AGRND_EXTERIOR_TRIM, UGRND_EXT_TRIM };

typedef std::unordered_map<HOUSE_ELEMENT, int> HouseStyle;

class House :
	public IStructure
{
protected:

	struct Room
	{
		Kiwi::Vector3d position;
		Kiwi::Vector3d dimensions;

		//stores which direction the roof will be oriented
		int roofDirection;

		bool hasNPC;

		Room( const Kiwi::Vector3d& position, int roofDirection )
		{
			this->position = position;
			this->roofDirection = roofDirection;

			std::random_device rd;
			std::mt19937 e2( rd() );
			std::uniform_real_distribution<> randomGen( 0.0, 1.0 );

			//each room has a 20% chance to have an npc spawn
			if( randomGen( e2 ) <= 0.2 )
			{
				hasNPC = true;
			}

		}
	};

	City* m_city;
	VoxelTerrain* m_terrain;

	Kiwi::Vector3d m_doorPosition; //position of the voxel on which the door sits

	//actual dimensions of the generated house
	Kiwi::Vector3d m_houseDimensions;

	//maximum possible size that can be generated
	Kiwi::Vector3d m_maxHouseDimensions;

	//size of the surrounding yard of the house
	Kiwi::Vector3d m_yardDimensions;

	Kiwi::Vector3d m_minRoomSize;
	Kiwi::Vector3d m_maxRoomSize;
	Kiwi::Vector3d m_roomSize;

	std::unordered_map<Kiwi::Vector3d, Room*, Kiwi::Vector3dHash, Kiwi::Vector3dEquality> m_roomLayout;

	int m_upperFloorCount;
	int m_lowerFloorCount;
	int m_npcCount;

	int m_doorHeight;

	//if true then the house can have underground floors
	bool m_enableLowerFloors;

	short m_houseType; //0 if random, otherwise the type of house to create
	
	HouseStyle m_houseStyle;

protected:

	void _GenerateRoomsFromLayout();
	void _GenerateRoof();
	void _GenerateRoomLayout( const Kiwi::Vector3d& dimensions );

	/*takes a position in local coordinates and places a voxel at that position if there is no voxel there already or if replaceExisting is true*/
	void _PlaceVoxel( HOUSE_ELEMENT voxelType, const Kiwi::Vector3d& localPosition, bool replaceExisting = false );

public:

	House( City* city, VoxelTerrain* terrain, int voxelSize, short houseType, const Kiwi::Vector3d position, const Kiwi::Vector3d& dimensions );
	~House() {}

	bool Generate();

	/*places all voxels of the house that are within the chunk on the ground closest to the house position*/
	void PlaceVoxels( VoxelChunk* chunk );

	const Kiwi::Vector3d& GetMaxDimensions()const { return m_maxHouseDimensions; }
	const Kiwi::Vector3d& GetDimensions()const { return m_houseDimensions; }
	const Kiwi::Vector3d& GetYardDimensions()const { return m_yardDimensions; }

};

#endif