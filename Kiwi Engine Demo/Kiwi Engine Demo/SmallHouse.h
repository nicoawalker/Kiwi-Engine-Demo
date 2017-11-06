#ifndef _SMALLHOUSE_H_
#define _SMALLHOUSE_H_

#include "VoxelInstanceGroup.h"

#include <Core\Vector3L.h>
#include <Core\Vector3d.h>
#include <Core\Entity.h>

#include <string>
#include <unordered_map>

class NPCTown;
class Voxel;

enum StructureNodeType { WALL = 0, ROOF, WINDOW, TRIM };

struct StructureNode
{
	StructureNode( StructureNodeType type ) :
		nodeType( type ), left( 0 ), right( 0 ), up( 0 ), down( 0 ), back( 0 ), front( 0 ), scale( 1.0 )
	{
	}
	StructureNodeType nodeType;
	Kiwi::Vector3d position;
	double scale;
	int left;
	int right;
	int up;
	int down;
	int back;
	int front;
};

class SmallHouse
{
protected:

	struct hashFunc
	{
		size_t operator()( const Kiwi::Vector3d& vec ) const
		{
			size_t h1 = std::hash<double>()(vec.x);
			size_t h2 = std::hash<double>()(vec.y);
			size_t h3 = std::hash<double>()(vec.z);
			return (h1 ^ (h2 << 1)) ^ h3;
		}
	};

	struct equalsFunc
	{
		bool operator()( const Kiwi::Vector3d& lhs, const Kiwi::Vector3d& rhs ) const
		{
			return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
		}
	};

	//pointer to the town in which the structure exists
	NPCTown* m_town;

	std::wstring m_name;

	std::unordered_map<Kiwi::Vector3d, Voxel, hashFunc, equalsFunc> m_voxels;

	//the dimensions of the structure
	Kiwi::Vector3L m_dimensions;
	Kiwi::Vector3d m_position;

	//the number of floors in the structure
	unsigned int m_floorCount;

	//std::vector<Voxel*> m_voxels;

	//entity consisting of all of the opaque voxels
	VoxelInstanceGroup* m_opaqueVoxels;

	//entity consisting of all of the transparent voxels
	VoxelInstanceGroup* m_transparentVoxels;

public:

	SmallHouse( Kiwi::Scene* scene, NPCTown* town, std::wstring name );
	~SmallHouse() {}

	//void Generate();

	//void SetDimensions( const Kiwi::Vector3L& dimensions ) { m_dimensions = dimensions; }

};

#endif