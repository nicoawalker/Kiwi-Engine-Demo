#include "Voxel.h"

Voxel::Voxel( VoxelDefinition* definition, const Kiwi::Vector3d& pos )
{

	m_definition = definition;
	m_position = pos;
	m_state = 0;

}

Voxel::Voxel( VoxelDefinition* definition, const Kiwi::Vector3d& pos, unsigned int state )
{

	m_definition = definition;
	m_position = pos;
	m_state = state;

}

bool Voxel::IsTransparent()const
{

	if( m_definition )
	{
		return (m_definition->color.alpha < 0.999) ? true : false;

	} else
	{
		return false;
	}

}

//void Voxel::_SetType( VoxelType type )
//{
//
//	if( type > VOXEL_TYPE_COUNT )
//	{//custom type
//
//	} else
//	{//built-in type
//		switch( type )
//		{
//			case GRASS:
//				{
//					color = Kiwi::Color( 1.0 / 255.0, 92.0 / 255.0, 0.0 / 255.0, 1.0 );
//					break;
//				}
//			case SAND:
//				{
//					color = Kiwi::Color( 200.0 / 255.0, 190.0 / 255.0, 134.0 / 255.0, 1.0 );
//					break;
//				}
//			case DIRT:
//				{
//					color = Kiwi::Color( 59.0 / 255.0, 34.0 / 255, 2.0 / 255.0, 1.0 );
//					break;
//				}
//			case MUD:
//				{
//					color = Kiwi::Color( 75.0 / 255.0, 49.0 / 255, 23.0 / 255.0, 1.0 );
//					solid = false;
//					break;
//				}
//			case WOOD_OAK:
//				{
//					color = Kiwi::Color( 174.0 / 255.0, 131.0 / 255.0, 75.0 / 255.0, 1.0 );
//					break;
//				}
//			case WOOD_MAHOGANY:
//				{
//					color = Kiwi::Color( 62.0 / 255.0, 37.0 / 255.0, 4.0 / 255.0, 1.0 );
//					break;
//				}
//			case WOOD_MAPLE:
//				{
//					color = Kiwi::Color( 170.0 / 255.0, 150.0 / 255.0, 77.0 / 255.0, 1.0 );
//					break;
//				}
//			case WOOD_WALNUT:
//				{
//					color = Kiwi::Color( 53.0 / 255.0, 23.0 / 255.0, 0.0 / 255.0, 1.0 );
//					break;
//				}
//			case TREE_OAK:
//				{
//					color = Kiwi::Color( 96.0 / 255.0, 72.0 / 255.0, 41.0 / 255.0, 1.0 );
//					break;
//				}
//			case TREE_WALNUT:
//				{
//					color = Kiwi::Color( 89.0 / 255.0, 56.0 / 255.0, 41.0 / 255.0, 1.0 );
//					break;
//				}
//			case TREE_MAPLE:
//				{
//					color = Kiwi::Color( 168.0 / 255.0, 128.0 / 255.0, 93.0 / 255.0, 1.0 );
//					break;
//				}
//			case WATER:
//				{
//					color = Kiwi::Color( 15.0 / 255.0, 160.0 / 255.0, 202.0 / 255.0, 0.92 );
//					solid = false;
//					break;
//				}
//			case AIR:
//				{
//					color = Kiwi::Color( 1.0, 1.0, 1.0, 0.0 );
//					solid = false;
//					break;
//				}
//			default:
//				{
//					color = Kiwi::Color( 0.0, 0.0, 0.0, 1.0 );
//					break;
//				}
//		}
//	}
//}