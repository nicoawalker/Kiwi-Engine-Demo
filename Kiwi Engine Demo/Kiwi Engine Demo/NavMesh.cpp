#include "NavMesh.h"

#include <Core\Math.h>
#include <Core\EngineRoot.h>
#include <Core\Logger.h>


NavMesh::NavMesh( const Kiwi::Vector3d& position, unsigned int numCellsX, unsigned int numCellsY, const Kiwi::Vector3d& cellDimensions )
{

	m_numCellsX = numCellsX;
	m_numCellsY = numCellsY;
	m_cellDimensions = cellDimensions;
	m_nodeDimensions = cellDimensions;
	m_position = position;

}

NavMesh::~NavMesh()
{

	for( auto itr = m_layers.begin(); itr != m_layers.end(); itr++ )
	{
		SAFE_DELETE( *itr );
	}
	Kiwi::FreeMemory( m_layers );

}

void NavMesh::AddNodeAtGlobalPoint( const Kiwi::Vector3d& position, NavMesh::Node* node )
{

	if( node != 0 )
	{
		Kiwi::Vector3d rPos = position;
		rPos.x = Kiwi::RoundToNearestd( rPos.x, m_cellDimensions.x );
		rPos.y = Kiwi::RoundToNearestd( rPos.y, m_cellDimensions.y );
		rPos.z = Kiwi::RoundToNearestd( rPos.z, m_cellDimensions.z );

		auto itr = m_mesh.find( rPos );
		if( itr != m_mesh.end() )
		{//erase existing node
			SAFE_DELETE( itr->second );
			m_mesh.erase( itr );
		}
		m_mesh[rPos] = node;
	}

}

void NavMesh::RemoveNodeAtGlobalPoint( const Kiwi::Vector3d& position )
{

	Kiwi::Vector3d rPos = position;
	rPos.x = Kiwi::RoundToNearestd( rPos.x, m_cellDimensions.x );
	rPos.y = Kiwi::RoundToNearestd( rPos.y, m_cellDimensions.y );
	rPos.z = Kiwi::RoundToNearestd( rPos.z, m_cellDimensions.z );

	auto itr = m_mesh.find( rPos );
	if( itr != m_mesh.end() )
	{//erase existing node
		SAFE_DELETE( itr->second );
		m_mesh.erase( itr );
	}

}

NavMesh::Node* NavMesh::GetNodeAtGlobalPoint( const Kiwi::Vector3d& position )
{

	Kiwi::Vector3d rPos = position;
	rPos.x = Kiwi::RoundToNearestd( rPos.x, m_cellDimensions.x );
	rPos.y = Kiwi::RoundToNearestd( rPos.y, m_cellDimensions.y );
	rPos.z = Kiwi::RoundToNearestd( rPos.z, m_cellDimensions.z );

	auto itr = m_mesh.find( rPos );
	if( itr != m_mesh.end() )
	{//erase existing node
		return itr->second;
	}

	return 0;

}

std::vector<NavMesh::Node*> NavMesh::GetAdjacentNodeList( const Kiwi::Vector3d& position, int minClearance, int maxHeight, int maxDrop )
{

	std::vector<NavMesh::Node*> nodeList;

	Kiwi::Vector3d rPos = position;
	rPos.x = Kiwi::RoundToNearestd( rPos.x, m_nodeDimensions.x );
	rPos.y = Kiwi::RoundToNearestd( rPos.y, m_nodeDimensions.y );
	rPos.z = Kiwi::RoundToNearestd( rPos.z, m_nodeDimensions.z );

	auto itr = m_mesh.find( rPos );
	if( itr != m_mesh.end() )
	{

		int row = 1, col = -1;
		for( unsigned int i = 0; i < 9; i++, col++ )
		{//check for nodes surrounding the center
			if( col == 2 )
			{
				col = -1;
				row--;
			}
			if( i == 4 ) continue;

			Kiwi::Vector3d stackPos = rPos + Kiwi::Vector3d( (double)col * m_nodeDimensions.x, 0.0, (double)row * m_nodeDimensions.z );

			bool canGoBelow = true;
			std::vector<NavMesh::Node*> upNodes;
			NavMesh::Node* downNode = 0;
			for( unsigned int h = 0; h <= maxHeight + minClearance; h++ )
			{
				Kiwi::Vector3d nodePos = rPos + Kiwi::Vector3d( (double)col * m_nodeDimensions.x, (double)h * m_nodeDimensions.y, (double)row * m_nodeDimensions.z );
				auto itr = m_mesh.find( nodePos );
				if( itr != m_mesh.end() )
				{
					upNodes.push_back( itr->second );
					if( h < minClearance )
					{//if a voxel is found before we have gone up by at least minClearance nodes, then we cannot get beneath the center node for this stack, so only consider higher nodes
						canGoBelow = false;
					}

				} else
				{
					upNodes.push_back( 0 );
				}
			}

			if( canGoBelow )
			{
				//if we can go down, get the first node below, if it's within maxDrop
				for( int h = -1; h > -maxDrop; h-- )
				{
					Kiwi::Vector3d nodePos = rPos + Kiwi::Vector3d( (double)col * m_nodeDimensions.x, (double)h * m_nodeDimensions.y, (double)row * m_nodeDimensions.z );
					auto itr = m_mesh.find( nodePos );
					if( itr != m_mesh.end() )
					{
						downNode = itr->second;
						break;
					}
				}
			}

			unsigned int spaceCount = 0;
			for( int node = upNodes.size() - 1; node >= 0; node-- )
			{
				if( upNodes[node] == 0 )
				{
					spaceCount++;

				} else
				{
					if( spaceCount >= minClearance )
					{
						nodeList.push_back( upNodes[node] );
					}
					spaceCount = 0;
				}
			}
			if( downNode != 0 )
			{
				nodeList.push_back( downNode );
			}
		}
	}

	return nodeList;

}

void NavMesh::SetLayerCount( unsigned int layerCount )
{

	if( layerCount < 1 )
	{
		layerCount = 1;
	}

	while( layerCount < m_layers.size() )
	{
		auto backItr = m_layers.end()--;
		SAFE_DELETE( *backItr );
	}

	while( layerCount > m_layers.size() )
	{
		m_layers.push_back( new Layer( m_numCellsX, m_numCellsY ) );
	}

}

unsigned int NavMesh::AddLayer( std::vector<std::vector<NavMesh::Cell*>> cells )
{

	m_layers.push_back( new Layer( cells ) );

	return m_layers.size() - 1;

}

NavMesh::Layer* NavMesh::GetLayer( unsigned int layer )
{

	if( layer < m_layers.size() )
	{
		return m_layers[layer];
	}

	return 0;

}

/*returns the cell at the given position in the graph for the specified layer*/
NavMesh::Cell* NavMesh::GetCell( unsigned int x, unsigned int y, unsigned int layer )
{

	if( layer >= m_layers.size() || x >= m_numCellsX || y >= m_numCellsY )
	{
		return 0;
	}

	if( m_layers[layer] != 0 )
	{
		return m_layers[layer]->cellList[y][x];
	}

	return 0;

}

NavMesh::Cell* NavMesh::GetCell( const Kiwi::Vector3d& pos )
{

	Kiwi::Vector3d rPos = pos;
	rPos.x = Kiwi::RoundToNearestd( rPos.x, m_cellDimensions.x );
	rPos.y = Kiwi::RoundToNearestd( rPos.y, m_cellDimensions.y );
	rPos.z = Kiwi::RoundToNearestd( rPos.z, m_cellDimensions.z );

	Kiwi::Vector3d navMeshDim( (double)this->GetLayerWidth() * this->GetCellDimensions().x, (double)this->GetLayerCount() * this->GetCellDimensions().y, (double)this->GetLayerDepth() * this->GetCellDimensions().z );
	if( rPos.x < m_position.x || rPos.x > m_position.x + navMeshDim.x ||
		rPos.y < m_position.y || rPos.y > m_position.y + navMeshDim.y ||
		rPos.z < m_position.z || rPos.z > m_position.z + navMeshDim.z )
	{//position is outside of the navmesh
		return 0;
	}

	unsigned int xPos = (unsigned int)((rPos.x - m_position.x) / m_cellDimensions.x);
	unsigned int zPos = (unsigned int)((rPos.z - m_position.z) / m_cellDimensions.z);
	unsigned int layer = (unsigned int)((rPos.y - m_position.y) / m_cellDimensions.y);

	return this->GetCell( xPos, zPos, layer );

}

/*returns the 8 cells surrounding the given center cell at position (x,y)*/
std::vector<NavMesh::Cell*> NavMesh::GetCellNeighbors( unsigned int x, unsigned int y, unsigned int layer )
{

	std::vector<NavMesh::Cell*> neighbors(8);

	if( layer >= m_layers.size() || x >= m_numCellsX || y >= m_numCellsY )
	{
		return neighbors;
	}

	if( m_layers[layer] != 0 )
	{
		if( x != 0 )
		{
			if( y != m_numCellsY - 1 )
			{
				neighbors[0] = m_layers[layer]->cellList[y + 1][x - 1];
			}
			neighbors[3] = m_layers[layer]->cellList[y][x - 1];
			if( y != 0 )
			{
				neighbors[5] = m_layers[layer]->cellList[y - 1][x - 1];
			}
		}
		if( y != m_numCellsY - 1 )
		{
			neighbors[1] = m_layers[layer]->cellList[y + 1][x];
		}
		if( y != 0 )
		{
			neighbors[6] = m_layers[layer]->cellList[y - 1][x];
		}

		if( x != m_numCellsX - 1 )
		{
			if( y != m_numCellsY - 1 )
			{
				neighbors[2] = m_layers[layer]->cellList[y + 1][x + 1];
			}
			neighbors[4] = m_layers[layer]->cellList[y][x + 1];
			if( y != 0 )
			{
				neighbors[7] = m_layers[layer]->cellList[y - 1][x + 1];
			}
		}
	}

	return neighbors;

}

std::vector<NavMesh::Cell*> NavMesh::GetCellNeighbors( const Kiwi::Vector3d& pos )
{

	Kiwi::Vector3d rPos = pos;
	rPos.x = Kiwi::RoundToNearestd( rPos.x, m_cellDimensions.x );
	rPos.y = Kiwi::RoundToNearestd( rPos.y, m_cellDimensions.y );
	rPos.z = Kiwi::RoundToNearestd( rPos.z, m_cellDimensions.z );

	Kiwi::Vector3d navMeshDim( (double)this->GetLayerWidth() * this->GetCellDimensions().x, (double)this->GetLayerCount() * this->GetCellDimensions().y, (double)this->GetLayerDepth() * this->GetCellDimensions().z );
	if( rPos.x < m_position.x || rPos.x > m_position.x + navMeshDim.x ||
		rPos.y < m_position.y || rPos.y > m_position.y + navMeshDim.y ||
		rPos.z < m_position.z || rPos.z > m_position.z + navMeshDim.z )
	{//position is outside of the navmesh
		std::vector<NavMesh::Cell*> empty;
		return empty;
	}

	unsigned int xPos = (unsigned int)((rPos.x - m_position.x) / m_cellDimensions.x);
	unsigned int zPos = (unsigned int)((rPos.z - m_position.z) / m_cellDimensions.z);
	unsigned int layer = (unsigned int)((rPos.y - m_position.y) / m_cellDimensions.y);

	return this->GetCellNeighbors( xPos, zPos, layer );

}