//#include "SmallHouseTypeA.h"
//#include "NPCTown.h"
//
//#include <Core/Vector2d.h>
//
//SmallHouseTypeA::SmallHouseTypeA( NPCTown& town, std::wstring name, const Kiwi::Vector3d& position ):
//	NPCStructure(town, name)
//{
//	//7
//	m_dimensions = Kiwi::Vector3d( 7.5, 6.5, 4.5 );
//	m_position = position;
//
//	double voxelSize = 0.5;
//
//	double roofLength = m_dimensions.x + 1.0;
//	double roofDepth = m_dimensions.z + 1.0;
//	Kiwi::Vector3d roofOrigin = Kiwi::Vector3d( 0.0, m_position.y + 3.5, 0.0 );
//	double roofHeight = (roofDepth + 0.5) / 2.0;
//
//	//create the bottom floor
//	for( double i = -3.5; i <= 3.5; i+= voxelSize )
//	{
//		for( double a = -2.0; a <= 2.0; a+= voxelSize )
//		{
//			if( i == -3.5 || i == 3.5 || a == 2.0 || a == -2.0 )
//			{//make the edges a different color
//				m_voxels[Kiwi::Vector3d( i, m_position.y, a )] = new Voxel( Voxel::WOOD_MAHOGANY );
//
//			} else
//			{
//				m_voxels[Kiwi::Vector3d( i, m_position.y, a )] = new Voxel( Voxel::WOOD_OAK );
//			}
//		}
//	}
//
//	//create the ceiling
//	for( double i = -3.5; i <= 3.5; i += voxelSize )
//	{
//		for( double a = -2.0; a <= 2.0; a += voxelSize )
//		{
//			if( i == -3.5 || i == 3.5 || a == 2.0 || a == -2.0 )
//			{//make the edges a different color
//				m_voxels[Kiwi::Vector3d( i, m_position.y + 3.0, a )] = new Voxel( Voxel::WOOD_MAHOGANY );
//
//			} else
//			{
//				m_voxels[Kiwi::Vector3d( i, m_position.y + 3.0, a )] = new Voxel( Voxel::WOOD_OAK );
//			}
//		}
//	}
//
//	//create the roof
//	for( double i = 0.0; i < roofHeight; i += 0.5 )
//	{
//		for( double a = (roofLength - voxelSize) / -2.0; a < (roofLength - voxelSize) / 2.0; a += 0.5 )
//		{
//			m_voxels[Kiwi::Vector3d( a - roofOrigin.x, i + roofOrigin.y, roofOrigin.z + ((roofDepth - 0.5) / 2.0))] = new Voxel( Voxel::WOOD_MAHOGANY );
//			m_voxels[Kiwi::Vector3d( a - roofOrigin.x, i + roofOrigin.y, roofOrigin.z - ((roofDepth - 0.5) / 2.0) )] = new Voxel( Voxel::WOOD_MAHOGANY );
//		}
//	}
//
//}