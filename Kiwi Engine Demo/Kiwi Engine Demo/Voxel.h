#ifndef _VOXEL_H_
#define _VOXEL_H_

#include <Core\Vector3d.h>
#include <Core\Entity.h>
#include <Core\Scene.h>

#include <Graphics\Color.h>

#include <string>

#define VOXEL_TYPE_COUNT 13 //number of types of voxels

enum VoxelType { AIR, GRASS, DIRT, SAND, WATER, MUD, STONE, WOOD_OAK, WOOD_MAHOGANY, WOOD_MAPLE, WOOD_WALNUT, TREE_OAK, TREE_WALNUT, TREE_MAPLE, TERRACOTTA, WHITE_PLASTER, BLACK_MARBLE };

struct VoxelInstance
{
	VoxelInstance()
	{
		position = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		color = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		world = DirectX::XMMatrixIdentity();
	}

	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMMATRIX world;
};

//stores the parameters for a voxel
struct VoxelDefinition
{
	Kiwi::Color color;
	double friction;
	unsigned short type;
	unsigned short state;
	short moveCost; //cost of walking across the voxel, used for pathfinding
	unsigned char solidity;
	bool isDynamic;

	VoxelDefinition( unsigned int type, Kiwi::Color& color, unsigned char solidity = 254, unsigned short state = 0, short moveCost = 1, bool isDynamic = false )
	{
		this->type = type;
		this->color = color;
		this->solidity = solidity;
		this->state = state;
		this->moveCost = moveCost;
		friction = 1.0;
		this->isDynamic = isDynamic;
	}

};

class Voxel
{
public:

	enum Side { LEFT, RIGHT, TOP, BOTTOM, FRONT, BACK };

protected:

	VoxelDefinition* m_definition;

	Kiwi::Vector3d m_position;

	unsigned short m_state;

public:

	Voxel( VoxelDefinition* definition, const Kiwi::Vector3d& position );
	Voxel( VoxelDefinition* definition, const Kiwi::Vector3d& position, unsigned int state );
	~Voxel() {}

	void SetState( unsigned short newState ) { m_state = newState; }
	void SetDefinition( VoxelDefinition* newDefinition ) { m_definition = newDefinition; m_state = 0; }
	void SetPosition( const Kiwi::Vector3d& newPosition ) { m_position = newPosition; }

	bool IsTransparent()const;

	unsigned int GetState()const { return m_state; }
	Kiwi::Vector3d GetPosition()const { return m_position; }
	VoxelDefinition* GetDefinition()const { return m_definition; }

};

/*stores information about a voxel needed to create that voxel*/
struct VoxelData
{
	VoxelDefinition* definition;

	//bits indicate which faces are visible in the order: front|back|above|below|left|right
	unsigned short visibility;

	VoxelData( VoxelDefinition* definition = 0, unsigned short visibility = 0 )
	{
		this->definition = definition;
		this->visibility = visibility;
	}

	bool GetSideVisibility( Voxel::Side side )
	{

		switch( side )
		{
			case Voxel::LEFT:
				{
					return (visibility & 0x02);
				}
			case Voxel::RIGHT:
				{
					return (visibility & 0x01);
				}
			case Voxel::FRONT:
				{
					return (visibility & 0x20);
				}
			case Voxel::BACK:
				{
					return (visibility & 0x10);
				}
			case Voxel::TOP:
				{
					return (visibility & 0x08);
				}
			case Voxel::BOTTOM:
				{
					return (visibility & 0x04);
				}
			default: return false;
		}

	}

	void SetSideVisibility( Voxel::Side side, bool isVisible )
	{

		switch( side )
		{
			case Voxel::LEFT:
				{
					if( isVisible )
					{
						visibility |= 0x02;

					} else
					{
						visibility &= ~(0x02);
					}
				}
			case Voxel::RIGHT:
				{
					if( isVisible )
					{
						visibility |= 0x01;

					} else
					{
						visibility &= ~(0x01);
					}
				}
			case Voxel::FRONT:
				{
					if( isVisible )
					{
						visibility |= 0x20;

					} else
					{
						visibility &= ~(0x20);
					}
				}
			case Voxel::BACK:
				{
					if( isVisible )
					{
						visibility |= 0x10;

					} else
					{
						visibility &= ~(0x10);
					}
				}
			case Voxel::TOP:
				{
					if( isVisible )
					{
						visibility |= 0x08;

					} else
					{
						visibility &= ~(0x08);
					}
				}
			case Voxel::BOTTOM:
				{
					if( isVisible )
					{
						visibility |= 0x04;

					} else
					{
						visibility &= ~(0x04);
					}
				}
			default: return;
		}

	}
};

#endif