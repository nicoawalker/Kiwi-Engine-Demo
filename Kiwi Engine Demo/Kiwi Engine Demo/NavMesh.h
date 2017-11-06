#ifndef _NAVMESH_H_
#define _NAVMESH_H_

#include <vector>
#include <unordered_map>

#include <Core\Utilities.h>
#include <Core\Vector3d.h>

class NavMesh
{
public:

	struct Cell
	{
		unsigned short layer;
		short cost;
		Kiwi::Vector3d globalPos;

		Cell()
		{
			this->layer = 0;
			this->cost = 0;
		}

		Cell( const Kiwi::Vector3d& globalPos, unsigned short layer, short cost )
		{
			this->layer = layer;
			this->cost = cost;
			this->globalPos = globalPos;
		}
	};

	struct Layer
	{
		std::vector<std::vector<Cell*>> cellList;

		Layer( unsigned int cellsX, unsigned int cellsY )
		{
			cellList.resize( cellsY );
			for( auto itr = cellList.begin(); itr != cellList.end(); itr++ )
			{
				(*itr).resize( cellsX );
			}
		}

		Layer( std::vector<std::vector<Cell*>> cells )
		{
			cellList.resize( cells.size() );
			for( unsigned int i = 0; i < cells.size(); i++ )
			{
				cellList[i] = cells[i];
			}
		}

		~Layer()
		{
			for( auto itr = cellList.begin(); itr != cellList.end(); itr++ )
			{
				for( auto innerItr = (*itr).begin(); innerItr != (*itr).end(); innerItr++ )
				{
					SAFE_DELETE( *innerItr );
				}
			}
			Kiwi::FreeMemory( cellList );
		}
	};

	struct Node
	{
		unsigned short cost;
		Kiwi::Vector3d position;
	};

	struct VectorHash
	{
		size_t operator()( const Kiwi::Vector3d& v ) const
		{
			size_t h1 = std::hash<double>()(v.x);
			size_t h2 = std::hash<double>()(v.y);
			size_t h3 = std::hash<double>()(v.z);
			return (h1 ^ (h2 << 1)) ^ h3;
		}
	};

	struct VectorEquals
	{
		bool operator()( const Kiwi::Vector3d& lhs, const Kiwi::Vector3d& rhs ) const
		{
			return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
		}
	};

protected:

	std::vector<Layer*> m_layers;

	Kiwi::Vector3d m_nodeDimensions;

	Kiwi::Vector3d m_cellDimensions;
	Kiwi::Vector3d m_position;

	unsigned int m_numCellsX;
	unsigned int m_numCellsY;

	std::unordered_map<Kiwi::Vector3d, Node*, VectorHash, VectorEquals> m_mesh;

public:

	NavMesh( const Kiwi::Vector3d& position, unsigned int numCellsX, unsigned int numCellsY, const Kiwi::Vector3d& cellDimensions );
	~NavMesh();

	void AddNodeAtGlobalPoint( const Kiwi::Vector3d& position, NavMesh::Node* node );

	void RemoveNodeAtGlobalPoint( const Kiwi::Vector3d& position );

	NavMesh::Node* GetNodeAtGlobalPoint( const Kiwi::Vector3d& position );

	/*returns a vector containing all of the surrounding nodes from the center position
	height is ignored*/
	//std::vector<NavMesh::Node*> GetAdjacentNodeList( const Kiwi::Vector3d& position );

	/*returns a vector containing all of the reachable nodes from the center position
	@minClearance: two stacked nodes must have this much distance between them vertically for the bottom to be considered walkable
	@maxHeight: maximum vertical distance between two voxels considered walkable
	@maxDrop: maximum negative vetical distance between voxels considered walkable*/
	std::vector<NavMesh::Node*> GetAdjacentNodeList( const Kiwi::Vector3d& globalPoint, int minClearance, int maxHeight, int maxDrop );

	/*alters the number of layers in the navmesh
	if layerCount is less than the current layer count, layers are removed from the top until they are equal*/
	void SetLayerCount( unsigned int layerCount );

	/*creates a new layer and fills it with the cell list*/
	unsigned int AddLayer( std::vector<std::vector<Cell*>> cells );

	NavMesh::Layer* GetLayer( unsigned int layer );

	/*returns the cell at the given position in the graph for the specified layer*/
	NavMesh::Cell* GetCell( unsigned int x, unsigned int y, unsigned int layer = 0 );

	NavMesh::Cell* GetCell( const Kiwi::Vector3d& pos );

	const Kiwi::Vector3d& GetCellDimensions()const { return m_cellDimensions; }

	/*returns the 8 cells surrounding the given center cell at position (x,y) in the mesh
	0 1 2
	3 * 4
	5 6 7*/
	std::vector<Cell*> GetCellNeighbors( unsigned int x, unsigned int y, unsigned int layer = 0 );

	/*returns the 8 cells surrounding the given center cell at the global position
	0 1 2
	3 * 4
	5 6 7*/
	std::vector<Cell*> GetCellNeighbors( const Kiwi::Vector3d& pos );

	const Kiwi::Vector3d& GetPosition()const { return m_position; }

	unsigned int GetLayerCount()const { return m_layers.size(); }
	unsigned int GetLayerWidth()const { return m_numCellsX; }
	unsigned int GetLayerDepth()const { return m_numCellsY; }

};

#endif