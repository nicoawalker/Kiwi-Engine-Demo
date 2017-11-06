#ifndef _VOXELTERRAINCHUNK_H_
#define _VOXELTERRAINCHUNK_H_

#include "Voxel.h"

#include <Core\Math.h>
#include <Core\Entity.h>
#include <Core\Scene.h>

#include <Graphics\Color.h>
#include <Graphics\InstancedMesh.h>
#include <Graphics\Renderable.h>
#include <Graphics\Renderer.h>

#include <unordered_map>
#include <string>

class VoxelTerrainChunk
{
protected:

	struct hashFunc
	{
		size_t operator()( const Voxel* v ) const
		{
			return 0;
			/*size_t h1 = std::hash<double>()(v->position.x);
			size_t h2 = std::hash<double>()(v->position.y);
			size_t h3 = std::hash<double>()(v->position.z);
			return (h1 ^ (h2 << 1)) ^ h3;*/
		}
	};

	struct equalsFunc
	{
		bool operator()( const Voxel* lhs, const Voxel* rhs ) const
		{
			return false;
			//return (lhs->position.x == rhs->position.x) && (lhs->position.y == rhs->position.y) && (lhs->position.z == rhs->position.z);
		}
	};

	struct Voxels_Y
	{
		std::unordered_map<double, Voxel*> yVoxels;
	};

	struct Voxels_X
	{
		std::unordered_map<double, Voxels_Y> xVoxels;
	};

	struct VoxelMap
	{
		std::unordered_map<double, Voxels_X> zVoxels;
	};

	Kiwi::Scene* m_scene;

	Kiwi::Entity* m_opaqueSurfaces;
	std::vector<Kiwi::Entity*> m_transparentSurfaces;

	std::wstring m_name;

	Kiwi::Vector3d m_position;

	//stores the map
	VoxelMap m_chunk;

	//stores the number of voxels on each side of the chunk
	unsigned int m_chunkSize;
	double m_voxelSize;

	unsigned long m_totalVoxelCount;
	unsigned long m_surfaceMeshVoxelCount;

	//when true, the chunk's meshes will be rebuilt on the next rebuild cycle
	bool m_rebuildMeshes;

	//store pointers to all surrounding chunks
	VoxelTerrainChunk* m_left;
	VoxelTerrainChunk* m_right;
	VoxelTerrainChunk* m_top;
	VoxelTerrainChunk* m_bottom;
	VoxelTerrainChunk* m_front;
	VoxelTerrainChunk* m_back;

protected:

	/*links a voxel with all of its neighbors in the chunk*/
	void _LinkVoxel( Voxel* voxel );

public:

	VoxelTerrainChunk( std::wstring name, Kiwi::Scene& scene, unsigned int chunkSize );
	~VoxelTerrainChunk();

	/*creates a new voxel at the given position
	any existing voxel at the same position will be destroyed*/
	void AddVoxel( Voxel* voxel );

	/*if the given chunk is adjacent to this one, they are linked together*/
	void LinkChunks( VoxelTerrainChunk* chunk );

	/*clears the current chunk, and then recreates it from the given voxels*/
	void BuildChunk( std::vector<Voxel*>& voxels );

	void GenerateWater( double heightCutoff);

	/*links all of the voxels with their neighboring voxels*/
	void LinkVoxels();

	/*rebuilds the entire surface mesh*/
	void RebuildSurfaceMesh();

	/*destroys every voxel in the chunk*/
	void Clear();

	/*destroys any voxel at the given position*/
	void DestroyVoxelAtPosition( const Kiwi::Vector3d& position );

	/*returns the voxel at the given position*/
	Voxel* FindVoxelAtPosition( const Kiwi::Vector3d& position );

	/*if there are any voxels at the given position it will return the maximum height
	if there are no voxels at the position it will return -1*/
	double FindHeightAtPosition( const Kiwi::Vector3d& position );

	double FindHeightUnderPosition( const Kiwi::Vector3d& position );

	void SetPosition( const Kiwi::Vector3d& position );

	Kiwi::Vector3d GetPosition()const { return m_position; }
	long long GetTotalVoxelCount()const { return m_totalVoxelCount; }
	unsigned long GetSurfaceMeshVoxelCount()const { return m_surfaceMeshVoxelCount; }
	long long GetInstanceBufferSize();

};

#endif