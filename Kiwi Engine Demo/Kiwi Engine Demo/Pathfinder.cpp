#include "Pathfinder.h"
#include "NavMesh.h"
#include "VoxelTerrain.h"
#include "VoxelChunk.h"

#include <Core\Transform.h>
#include <Core\Entity.h>
#include <Core\Scene.h>
#include <Core\Logger.h>

#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>

Pathfinder::Pathfinder()
{

	m_nodePath = 0;
	m_nodeTarget = 0;
	m_pathingEnabled = false;
	m_minNodeEqDist = 0.05;
	m_maxJump = 0;
	m_maxFall = 0;
	m_minClearance = 1;

}

Pathfinder::~Pathfinder()
{

	m_nodeTarget = 0;
	SAFE_DELETE( m_nodePath );

}

void Pathfinder::_OnAttached()
{

	Kiwi::Transform* entTransform = m_entity->FindComponent<Kiwi::Transform>();
	if( entTransform )
	{
		m_pathTarget = entTransform->GetPosition();
	}

}

void Pathfinder::_OnFixedUpdate()
{

	Kiwi::Transform* entTransform = m_entity->FindComponent<Kiwi::Transform>();
	if( entTransform )
	{
		if( m_pathingEnabled )
		{
			if( m_nodeTarget != 0 )
			{
				Kiwi::Vector3d entPosition = entTransform->GetPosition();
				entPosition.y = 0.0;
				Kiwi::Vector3d nodePosition = m_nodeTarget->position;
				nodePosition.y = 0.0;
				if( Kiwi::Vector3d::SquareDistance(entPosition, nodePosition) <= (m_minNodeEqDist * m_minNodeEqDist) )
				{
					m_nodePath->nodes.erase( m_nodePath->nodes.begin() );
					if( m_nodePath->nodes.size() > 0 )
					{
						m_nodeTarget = &m_nodePath->nodes[0];

					} else
					{
						m_nodeTarget = 0;
						SAFE_DELETE( m_nodePath );
					}
				}
			}

		} else
		{
			m_nodeTarget = 0;
			SAFE_DELETE( m_nodePath );
		}
	}

}

double Pathfinder::_Heuristic_AStar( const Kiwi::Vector3d& a, const Kiwi::Vector3d& b )
{

	/*Manhattan Distance, faster/less accurate
	int x1, x2, y1, y2;
	x1 = a.x;
	x2 = b.x;
	y1 = a.z;
	y2 = b.z;

	return std::abs( x1 - x2 ) + std::abs( y1 - y2 );*/

	return Kiwi::Vector3d::SquareDistance( a, b );

}

Pathing::NodePath* Pathfinder::_GenerateNodePath_AStar( const Kiwi::Vector3d& origin, const Kiwi::Vector3d& target, NavMesh* navMesh )
{

	if( navMesh == 0 || origin == target )
	{
		return 0;
	}

	NavMesh::Node* originNode = navMesh->GetNodeAtGlobalPoint( origin );
	NavMesh::Node* targetNode = navMesh->GetNodeAtGlobalPoint( target );

	if( originNode == 0 || targetNode == 0 )
	{
		return 0;
	}

	struct PathNode
	{
		double fCost; //fCost = gCost + hCost;
		double gCost; //shortest distance from origin to this node
		double hCost; //distance to the ending node
		double cost; //movement cost
		PathNode* parent;
		Kiwi::Vector3d position;

		PathNode( const Kiwi::Vector3d& position )
		{
			parent = 0;
			fCost = 0.0;
			gCost = 0.0;
			hCost = 0.0;
			cost = 0.0;
			this->position = position;
		}
	};

	struct PathNodeHash
	{
		size_t operator()( const PathNode* n ) const
		{
			size_t h1 = std::hash<double>()(n->position.x);
			size_t h2 = std::hash<double>()(n->position.y);
			size_t h3 = std::hash<double>()(n->position.z);
			return (h1 ^ (h2 << 1)) ^ h3;
		}
	};

	struct PathNodeEquals
	{
		bool operator() ( const PathNode* n1, const PathNode* n2 )const
		{
			return n1->position == n2->position;
		}
	};

	std::unordered_set<PathNode*, PathNodeHash, PathNodeEquals> openList;
	std::unordered_set<PathNode*, PathNodeHash, PathNodeEquals> closedList;
	Pathing::NodePath* newPath = new Pathing::NodePath();

	PathNode* originPathNode = new PathNode( originNode->position );
	openList.insert( originPathNode );

	while( openList.size() > 0 )
	{

		PathNode* currentNode = 0;
		for( auto itr = openList.begin(); itr != openList.end(); itr++ )
		{
			if( currentNode == 0 )
			{
				currentNode = *itr;

			} else if( currentNode->fCost > (*itr)->fCost )
			{
				currentNode = *itr;
			}
		}

		if( currentNode == 0 )
		{
			return 0;
		}

		if( currentNode->position == targetNode->position )
		{
			newPath->nodes.push_back( Pathing::Node( targetNode->position ) );
			currentNode = currentNode->parent;
			while( currentNode != 0 && currentNode->parent != 0 )
			{
				newPath->nodes.push_back( Pathing::Node( currentNode->position ) );
				currentNode = currentNode->parent;
			}
			std::reverse( newPath->nodes.begin(), newPath->nodes.end() );

			for( auto itr = closedList.begin(); itr != closedList.end();)
			{
				delete (*itr);
				itr = closedList.erase( itr );
			}

			for( auto itr = openList.begin(); itr != openList.end();)
			{
				delete (*itr);
				itr = openList.erase( itr );
			}

			return newPath;
		}

		openList.erase( currentNode );
		closedList.insert( currentNode );

		//2 2 3
		for( auto neighbor : navMesh->GetAdjacentNodeList( currentNode->position, m_minClearance, m_maxJump, m_maxFall ) )
		{

			if( neighbor != 0 )
			{
				PathNode* neighborNode = new PathNode( neighbor->position );
				neighborNode->cost = neighbor->cost;

				if( closedList.find( neighborNode ) != closedList.end() )
				{
					continue;
				}

				double gScore = currentNode->gCost + neighborNode->cost;

				auto itr = openList.find( neighborNode );
				if( itr != openList.end() )
				{
					neighborNode = *itr;

					if( gScore < neighborNode->gCost )
					{
						neighborNode->gCost = gScore;
						neighborNode->fCost = neighborNode->hCost + neighborNode->gCost;
						neighborNode->parent = currentNode;
					}

				} else
				{
					neighborNode->hCost = this->_Heuristic_AStar( neighborNode->position, targetNode->position );
					neighborNode->gCost = gScore;
					neighborNode->fCost = neighborNode->hCost + neighborNode->gCost;
					neighborNode->parent = currentNode;
					openList.insert( neighborNode );
				}
			}
		}
	}

	return 0;

}

void Pathfinder::GeneratePath( const Kiwi::Vector3d& position )
{

	m_pathTarget = position;

	Kiwi::Transform* entTransform = m_entity->FindComponent<Kiwi::Transform>();
	if( entTransform )
	{
		if( m_pathTarget != entTransform->GetPosition() )
		{
			Kiwi::Scene* scene = m_entity->GetScene();
			if( scene == 0 )
			{
				m_nodeTarget = 0;
				SAFE_DELETE( m_nodePath );
				return;
			}

			VoxelTerrain* terrain = scene->GetTerrain<VoxelTerrain*>();
			if( terrain == 0 )
			{
				m_nodeTarget = 0;
				SAFE_DELETE( m_nodePath );
				return;
			}

			VoxelChunk* chunk = terrain->FindChunkContainingPosition( entTransform->GetPosition() );
			if( chunk != 0 )
			{
				NavMesh* navMesh = chunk->GetNavMesh();

				if( entTransform->GetSquareDistance( m_pathTarget ) > (m_minNodeEqDist * m_minNodeEqDist) )
				{//if the entity has not yet arrived at the path target

					Voxel* underVoxel = terrain->FindSurfaceVoxelUnderPosition( entTransform->GetPosition() );
					if( underVoxel )
					{
						m_nodePath = this->_GenerateNodePath_AStar( underVoxel->GetPosition(), m_pathTarget, navMesh );
					}

					if( m_nodePath != 0 )
					{
						if( m_nodePath->nodes.size() == 0 )
						{
							m_nodeTarget = 0;
							SAFE_DELETE( m_nodePath );

						} else
						{
							m_nodeTarget = &m_nodePath->nodes[0];
							m_pathingEnabled = true;
						}

					} else
					{
						m_nodeTarget = 0;
					}

				} else
				{
					m_nodeTarget = 0;
					SAFE_DELETE( m_nodePath );
				}
			}

		} else
		{
			m_nodeTarget = 0;
			SAFE_DELETE( m_nodePath );
		}

	} else
	{
		m_nodeTarget = 0;
		SAFE_DELETE( m_nodePath );
	}

}