#ifndef _ISTRUCTURE_H_
#define _ISTRUCTURE_H_

#include <Core\Component.h>
#include <Core/Vector3d.h>

#include <unordered_map>

typedef std::unordered_map<Kiwi::Vector3d, int, Kiwi::Vector3dHash, Kiwi::Vector3dEquality> StructureVoxels;

class Voxel;
class VoxelChunk;
class VoxelTerrain;

class IStructure :
	public Kiwi::Component
{
protected:

	double m_voxelSize;

	Kiwi::Vector3d m_boundingVolume;

	StructureVoxels m_structureVoxels;

	Kiwi::Vector3d m_position;

public:

	IStructure(double voxelSize);
	~IStructure() = 0;

	void SetPosition( const Kiwi::Vector3d& newPosition ) { m_position = newPosition; }

	StructureVoxels& GetVoxels() { return m_structureVoxels; }

	Kiwi::Vector3d GetBoundingVolume()const { return m_boundingVolume; }

	Kiwi::Vector3d GetPosition()const { return m_position; }

};

#endif