#ifndef _CITY_H_
#define _CITY_H_

#include <Core\Component.h>
#include <Core/Vector2d.h>
#include <Core\Vector3d.h>

#include <unordered_set>
#include <deque>

class VoxelTerrain;
class VoxelChunk;
class House;

class City :
	public Kiwi::Component
{
protected:

	struct CityBlock
	{
		Kiwi::Vector2d area;
		Kiwi::Vector2d localPosition;
		int houseCount;

	};

	struct RoadNode
	{
		//stores the length of the road in each direction from this node
		double rl, ll, fl, bl;

		//width of the connected roads 
		double rw, lw, fw, bw;
		Kiwi::Vector3d pos;

		RoadNode( const Kiwi::Vector3d& position, double rl = 0.0, double ll = 0.0, double fl = 0.0, double bl = 0.0 )
		{
			this->rl = rl;
			this->ll = ll;
			this->fl = fl;
			this->bl = bl;

			pos = position;

			rw = lw = fw = bw = 3.0;
		}
	};

	std::unordered_map<Kiwi::Vector3d, int, Kiwi::Vector3dHash, Kiwi::Vector3dEquality> m_cityVoxels;
	std::unordered_map<Kiwi::Vector3d, House*, Kiwi::Vector3dHash, Kiwi::Vector3dEquality> m_houses;

	VoxelTerrain* m_terrain;

	Kiwi::Vector2d m_cityArea;

	Kiwi::Vector3d m_globalPosition;
	Kiwi::Vector3d m_blockSize; //size of a city block
	Kiwi::Vector3d m_minHouseSize;
	Kiwi::Vector3d m_maxHouseSize;

	double m_roadWidth;

	/*stores how much favour the player has with the city, from -100 to 100
	a negative value indicates that the city is actively hostile*/
	int m_playerFavour;

protected:

	void _OnAttached() {}

public:

	City( VoxelTerrain* terrain, const Kiwi::Vector2d& cityArea, const Kiwi::Vector3d& globalPosition );
	~City() {}

	void Generate();

	void PlaceVoxels( VoxelChunk* targetChunk );

};


#endif