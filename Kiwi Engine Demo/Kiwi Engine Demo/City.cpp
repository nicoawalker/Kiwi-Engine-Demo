#include "City.h"
#include "VoxelTerrain.h"
#include "VoxelChunk.h"
#include "House.h"

#include <algorithm>
#include <random>

City::City( VoxelTerrain* terrain, const Kiwi::Vector2d& cityArea, const Kiwi::Vector3d& globalPosition )
{

	m_maxHouseSize.Set( 11.0, 15.0, 11.0 );

	m_terrain = terrain;
	m_cityArea = cityArea;
	m_globalPosition = globalPosition;

	//randomly generate an initial favour value
	std::random_device rd;
	std::mt19937 rGen( rd() );
	std::uniform_real_distribution<> random01( -10.0, 30.0 );
	m_playerFavour = random01( rGen );

}

void City::Generate()
{

	//stores the nodes that have been placed
	std::unordered_map<Kiwi::Vector3d, RoadNode*, Kiwi::Vector3dHash, Kiwi::Vector3dEquality> placedNodes;

	//stores the nodes that haven't yet been checked
	std::deque<RoadNode*> unplacedNodes;

	std::random_device rd;
	std::mt19937 rGen( rd() );
	std::uniform_real_distribution<> random01( 0.0, 1.0 );
	std::uniform_real_distribution<> randomSide( -1.0, 1.0 );

	double voxelSize = m_terrain->GetVoxelSize();

	double m_roadWidth = 3.0;

	//stores the length between any two road nodes
	double roadSegmentLength = max( (m_maxHouseSize.x * 2.0) + 3.0, (m_maxHouseSize.z * 2.0) + 3.0 ) + m_roadWidth;

	Kiwi::Vector2d maxRoadSegments( std::floor( m_cityArea.x / roadSegmentLength ), std::floor( m_cityArea.y / roadSegmentLength ) );

	Kiwi::Vector3d currentPos( 0.0, 0.0, -(std::floor( maxRoadSegments.y / 2.0 ) * roadSegmentLength) );
	RoadNode* currentNode = new RoadNode( currentPos, 0.0, 0.0, roadSegmentLength, 0.0 );
	placedNodes.insert( std::make_pair( currentPos, currentNode ) );
	currentPos.z += roadSegmentLength;
	int xSeg = std::floor( maxRoadSegments.y / 2.0 );
	int zSeg = 0;
	while( zSeg < maxRoadSegments.y )
	{
		RoadNode* newUnplaced = new RoadNode( currentPos );
		newUnplaced->bl = (currentNode->fl > 0.0) ? -1.0 : 0.0;
		newUnplaced->ll = (currentNode->rl > 0.0) ? -1.0 : 0.0;
		newUnplaced->rl = (currentNode->ll > 0.0) ? -1.0 : 0.0;

		//generate a road segment in a random direction
		// -0.5 < fb < 0.5 means generate left/right, else generate forward
		double fb = randomSide( rGen );
		if( std::abs( fb ) >= 0.5 || xSeg == (int)maxRoadSegments.x || xSeg == 0 || zSeg == maxRoadSegments.y - 1 )
		{ //generate forward

			zSeg++;
			currentNode = new RoadNode( currentPos, 0.0, 0.0, roadSegmentLength, -1.0 );
			currentPos.z += roadSegmentLength;
			newUnplaced->fl = -1.0;

		} else
		{ //generate left / right

			if( currentNode->ll < 0.0 )
			{
				fb = 0.1;

			} else if( currentNode->rl < 0.0 )
			{
				fb = -0.1;
			}

			if( fb > 0.0 )
			{//generate right
				xSeg++;

				currentNode = new RoadNode( currentPos, roadSegmentLength, -1.0, 0.0, -1.0 );
				currentPos.x += roadSegmentLength;
				newUnplaced->rl = -1.0;

			} else
			{//generate left
				xSeg--;

				currentNode = new RoadNode( currentPos, -1.0, roadSegmentLength, 0.0, -1.0 );
				currentPos.x -= roadSegmentLength;
				newUnplaced->ll = -1.0;
			}
			
		}

		unplacedNodes.push_back( currentNode );
		//placedNodes.insert( std::make_pair( currentNode->pos, currentNode ) );
	}

	while( unplacedNodes.size() > 0 )
	{
		RoadNode* current = unplacedNodes.front();
		unplacedNodes.pop_front();

		double rand = 0.0;

		if( current == 0 )
		{
			continue;
		}

		if( current->pos.x < -(m_cityArea.x / 2.0) || current->pos.x >( m_cityArea.x / 2.0 ) ||
			current->pos.z < -(m_cityArea.y / 2.0) || current->pos.z >( m_cityArea.y / 2.0 ) )
		{
			SAFE_DELETE( current );
			continue;
		}

		if( placedNodes.find( current->pos ) == placedNodes.end() )
		{
			placedNodes.insert( std::make_pair( current->pos, current ) );

			for( unsigned int i = 0; i < 4; i++ )
			{
				double generateRoad = random01( rGen );
				double generateHouse = random01( rGen );
				if( i == 0 )
				{//left
					if( generateRoad >= 0.25 )
					{ //generate road to the left
						if( current->ll >= 0.0 && placedNodes.find( current->pos - Kiwi::Vector3d( roadSegmentLength, 0.0, 0.0 ) ) == placedNodes.end() )
						{
							RoadNode* newNode = new RoadNode( current->pos - Kiwi::Vector3d( roadSegmentLength, 0.0, 0.0 ), -1.0, 0.0, 0.0, 0.0 );
							unplacedNodes.push_back( newNode );
							current->ll = roadSegmentLength;

							if( generateHouse >= 0.2 )
							{//if there's no road to the left there's a 80% chance to generate a house above the left road
								House* house = new House( this, m_terrain, 1, 0, Kiwi::Vector3d(), m_maxHouseSize );
								house->Generate();
								double roadWidth = std::floor( current->lw / 2.0 );
								house->SetPosition( m_globalPosition + current->pos + Kiwi::Vector3d( (-std::ceil( house->GetDimensions().x / 2.0 ) - 1.0) - roadWidth, 0.0, std::ceil( house->GetDimensions().z / 2.0 ) + 1.0 + roadWidth ) );

								m_houses.insert( std::make_pair( house->GetPosition(), house ) );
							}
						}
					}

				} else if( i == 1 )
				{//right
					if( generateRoad >= 0.25 + (i*0.1) )
					{ //generate road to the right
						if( current->rl >= 0.0 && placedNodes.find( current->pos + Kiwi::Vector3d( roadSegmentLength, 0.0, 0.0 ) ) == placedNodes.end() )
						{
							RoadNode* newNode = new RoadNode( current->pos + Kiwi::Vector3d( roadSegmentLength, 0.0, 0.0 ), 0.0, -1.0, 0.0, 0.0 );
							unplacedNodes.push_back( newNode );
							current->rl = roadSegmentLength;

							if( generateHouse >= 0.2 )
							{//if there's no road to the left there's a 80% chance to generate a house there
								House* house = new House( this, m_terrain, 1, 0, Kiwi::Vector3d(), m_maxHouseSize );
								house->Generate();
								double roadWidth = std::floor( current->rw / 2.0 );
								house->SetPosition( m_globalPosition + current->pos + Kiwi::Vector3d( std::ceil( house->GetDimensions().x / 2.0 ) + 1.0 + roadWidth, 0.0, (-std::ceil( house->GetDimensions().z / 2.0 ) - 1.0) - roadWidth ) );

								m_houses.insert( std::make_pair( house->GetPosition(), house ) );
							}
						}
					}

				} else if( i == 2 )
				{//front
					if( generateRoad >= 0.25 + (i*0.1) )
					{ //generate road in front
						if( current->fl >= 0.0 && placedNodes.find( current->pos + Kiwi::Vector3d( 0.0, 0.0, roadSegmentLength ) ) == placedNodes.end() )
						{
							RoadNode* newNode = new RoadNode( current->pos + Kiwi::Vector3d( 0.0, 0.0, roadSegmentLength ), 0.0, 0.0, 0.0, -1.0 );
							unplacedNodes.push_back( newNode );
							current->fl = roadSegmentLength;

							if( generateHouse >= 0.2 )
							{//if there's no road to the left there's a 80% chance to generate a house there
								House* house = new House( this, m_terrain, 1, 0, Kiwi::Vector3d(), m_maxHouseSize );
								house->Generate();
								double roadWidth = std::floor( current->fw / 2.0 );
								house->SetPosition( m_globalPosition + current->pos + Kiwi::Vector3d( std::ceil( house->GetDimensions().x / 2.0 ) + 1.0 + roadWidth, 0.0, std::ceil( house->GetDimensions().z / 2.0 ) + 1.0 + roadWidth ) );

								m_houses.insert( std::make_pair( house->GetPosition(), house ) );
							}
						}
					}

				} else if( i == 3 )
				{//back
					if( generateRoad >= 0.25 + (i*0.1) )
					{//generate road behind
						if( current->bl >= 0.0 && placedNodes.find( current->pos - Kiwi::Vector3d( 0.0, 0.0, roadSegmentLength ) ) == placedNodes.end() )
						{
							RoadNode* newNode = new RoadNode( current->pos - Kiwi::Vector3d( 0.0, 0.0, roadSegmentLength ), 0.0, 0.0, -1.0, 0.0 );
							unplacedNodes.push_back( newNode );
							current->bl = roadSegmentLength;

							if( generateHouse >= 0.2 )
							{//if there's no road to the left there's a 80% chance to generate a house there
								House* house = new House( this, m_terrain, 1, 0, Kiwi::Vector3d(), m_maxHouseSize );
								house->Generate();
								double roadWidth = std::floor( current->bw / 2.0 );
								house->SetPosition( m_globalPosition + current->pos + Kiwi::Vector3d( (-std::ceil( house->GetDimensions().x / 2.0 ) - 1.0) - roadWidth, 0.0, (-std::ceil( house->GetDimensions().z / 2.0 ) - 1.0) - roadWidth ) );

								m_houses.insert( std::make_pair( house->GetPosition(), house ) );
							}
						}
					}
				}
			}

		} else
		{
			SAFE_DELETE( current );
		}

	}

	for( auto itr = placedNodes.begin(); itr != placedNodes.end(); )
	{

		double maxWidth = max( max( std::ceil( (itr->second->lw - 1.0) / 2.0 ), std::ceil( (itr->second->rw - 1.0) / 2.0 ) ), 1.0 );
		double maxHeight = max( max( std::ceil( (itr->second->fw - 1.0) / 2.0 ), std::ceil( (itr->second->bw - 1.0) / 2.0 ) ), 1.0 );
		for( double width = -maxWidth; width <= maxWidth; width++ )
		{
			for( double height = -maxHeight; height <= maxHeight; height++ )
			{
				m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) + Kiwi::Vector3d(width, 0.0, height), VoxelType::STONE ) );
			}
		}

		if( itr->second->ll > 0.0 )
		{
			for( unsigned int x = voxelSize; x < itr->second->ll * voxelSize; x += voxelSize )
			{
				if( itr->second->lw == 1.0 )
				{//generate a 1 voxel wide road
					m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) - Kiwi::Vector3d( x, 0.0, 0.0 ), VoxelType::STONE ) );

				} else
				{//generate a road wider than 1 voxel
					for( double height = -std::ceil( (itr->second->lw - 1.0) / 2.0 ); height <= std::ceil( (itr->second->lw - 1.0) / 2.0 ); height++ )
					{
						m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) - Kiwi::Vector3d( x, 0.0, height ), VoxelType::STONE ) );
					}
				}
			}
		}

		if( itr->second->rl > 0.0 )
		{
			for( unsigned int x = voxelSize; x < itr->second->rl * voxelSize; x += voxelSize )
			{
				if( itr->second->rw == 1.0 )
				{//generate a 1 voxel wide road
					m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) + Kiwi::Vector3d( x, 0.0, 0.0 ), VoxelType::STONE ) );

				} else
				{//generate a road wider than 1 voxel
					for( double height = -std::ceil( (itr->second->rw - 1.0) / 2.0 ); height <= std::ceil( (itr->second->rw - 1.0) / 2.0 ); height++ )
					{
						m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) + Kiwi::Vector3d( x, 0.0, height ), VoxelType::STONE ) );
					}
				}
			}
		}

		if( itr->second->fl > 0.0 )
		{
			for( unsigned int z = voxelSize; z < itr->second->fl * voxelSize; z += voxelSize )
			{
				if( itr->second->fw == 1.0 )
				{//generate a 1 voxel wide road
					m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) + Kiwi::Vector3d( 0.0, 0.0, z ), VoxelType::STONE ) );

				} else
				{//generate a road wider than 1 voxel
					for( double width = -std::ceil( (itr->second->fw - 1.0) / 2.0 ); width <= std::ceil( (itr->second->fw - 1.0) / 2.0 ); width++ )
					{
						m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) + Kiwi::Vector3d( width, 0.0, z ), VoxelType::STONE ) );
					}
				}
			}
		}

		if( itr->second->bl > 0.0 )
		{
			for( unsigned int z = voxelSize; z < itr->second->bl * voxelSize; z += voxelSize )
			{
				if( itr->second->bw == 1.0 )
				{//generate a 1 voxel wide road
					m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) - Kiwi::Vector3d( 0.0, 0.0, z ), VoxelType::STONE ) );

				} else
				{//generate a road wider than 1 voxel
					for( double width = -std::ceil( (itr->second->bw - 1.0) / 2.0 ); width <= std::ceil( (itr->second->bw - 1.0) / 2.0 ); width++ )
					{
						m_cityVoxels.insert( std::make_pair( (itr->second->pos * voxelSize) - Kiwi::Vector3d( width, 0.0, z ), VoxelType::STONE ) );
					}
				}
			}
		}

		SAFE_DELETE( itr->second );
		itr = placedNodes.erase( itr );
	}

}

void City::PlaceVoxels( VoxelChunk* targetChunk )
{

	if( targetChunk == 0 )
	{
		return;
	}

	for( auto itr = m_cityVoxels.begin(); itr != m_cityVoxels.end(); itr++ )
	{
		Voxel* targetVoxel = m_terrain->FindSurfaceVoxelUnderPosition( m_globalPosition + itr->first );
		if( targetVoxel != 0 )
		{
			targetChunk->ReplaceVoxelAtPosition( targetVoxel->GetPosition(), itr->second );
		}
	}

	for( auto itr = m_houses.begin(); itr != m_houses.end(); itr++ )
	{
		itr->second->PlaceVoxels( targetChunk );
	}

}