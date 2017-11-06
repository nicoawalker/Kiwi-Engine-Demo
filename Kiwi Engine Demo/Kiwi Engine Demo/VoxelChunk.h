#ifndef _VOXELCHUNK_H_
#define _VOXELCHUNK_H_

#include "Voxel.h"
#include "VoxelTerrain.h"

#include <Core\Math.h>
#include <Core\Entity.h>
#include <Core\Scene.h>
#include <Core\ThreadManager.h>

#include <Graphics\Color.h>
#include <Graphics\InstancedMesh.h>
#include <Graphics\Renderable.h>
#include <Graphics\Renderer.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

class NavMesh;

namespace Kiwi
{
	class EngineRoot;
}

class VoxelChunk
{
public:

	enum ChunkStatus { UNINITIALIZED, FAILED, READY, SHUTDOWN, BUSY };
	enum CHUNK_TASK { NONE, GENERATING_SURFACES, GENERATING_TERRAIN, INITIALIZING };

protected:

	struct hashFunc
	{
		size_t operator()( const Kiwi::Vector3d& v ) const
		{
			size_t h1 = std::hash<double>()(v.x);
			size_t h2 = std::hash<double>()(v.y);
			size_t h3 = std::hash<double>()(v.z);
			return (h1 ^ (h2 << 1)) ^ h3;
		}
	};

	struct equalsFunc
	{
		bool operator()( const Kiwi::Vector3d& lhs, const Kiwi::Vector3d& rhs ) const
		{
			return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
		}
	};

	struct ChunkProperties
	{
		int worldSeed;
		Kiwi::Vector3d dimensions;
	};

	struct SurfaceParam :
		public Kiwi::IThreadParam
	{
		Kiwi::Mesh* solidMesh;
		std::vector<Kiwi::Mesh*> transparentMeshes;

		std::unordered_map<Kiwi::Vector3d, VoxelData*, hashFunc, equalsFunc> surfaceVoxelData;

		SurfaceParam() :
			Kiwi::IThreadParam( 1 )
		{
			solidMesh = 0;
		}
	};

	Kiwi::EngineRoot* m_engine;

	VoxelTerrain* m_terrain;
	Kiwi::Scene* m_scene;

	Kiwi::Entity* m_opaqueSurfaceMesh;
	std::vector<Kiwi::Entity*> m_transparentSurfaceMeshes;

	std::vector<double> m_heightmap; //heightmap storing the height of the topmost non-air voxel at each position

	std::vector<VoxelData> m_voxelData;
	std::unordered_map<Kiwi::Vector3d, Voxel*, hashFunc, equalsFunc> m_surfaceVoxels;

	Kiwi::ThreadManager<int> m_threadManager;
	std::vector<unsigned int> m_runningThreads;

	VoxelTerrain::BiomeParameters m_biomeParams;

	NavMesh* m_navMesh;

	ChunkStatus m_chunkStatus;
	CHUNK_TASK m_currentTask;

	Kiwi::Vector3d m_position;
	//stores the number of voxels on each side of the chunk
	Kiwi::Vector3d m_chunkDimensions;

	double m_verticalScale;
	double m_surfaceFrequency;

	double m_caveFrequency;
	double m_caveSmoothness;

	//when true, the chunk's meshes will be rebuilt on the next rebuild cycle
	bool m_shutdown;
	bool m_active;
	bool m_isSurfaceChunk; //if false, will generate cave terrain rather than surface terrain
	bool m_enableSurfaceVoxels;

	bool m_fullRebuildPending;
	bool m_surfaceRebuildPending;
	bool m_currentlyRebuilding;
	bool m_built;
	bool m_linked;

	bool m_playerModified; //true if the player has modified the contents of the chunk

	int m_biome; //stores the type of biome this chunk will generate

	std::mutex m_voxelDataMutex;
	std::mutex m_surfaceVoxelMutex;
	std::mutex m_meshMutex;

	//store pointers to all surrounding chunks
	VoxelChunk* m_neighborLeft;
	VoxelChunk* m_neighborRight;
	VoxelChunk* m_neighborBottom;
	VoxelChunk* m_neighborTop;
	VoxelChunk* m_neighborFront;
	VoxelChunk* m_neighborBack;

protected:

	/*completely rebuilds the entire chunk*/
	Kiwi::IThreadParam* _RebuildAll( Kiwi::IThreadParam* param );

	/*rebuilds just the surfaces and regenerates the surface meshes*/
	Kiwi::IThreadParam* _RebuildSurfaces( Kiwi::IThreadParam* param );

	/*randomly generates the voxel data for the terrain based on the current position and biome parameters*/
	void _GenerateTerrainData();

	/*uses the voxel data to create all visible surface voxels*/
	void _GenerateSurfaceVoxels( Kiwi::IThreadParam* param );

	/*builds the mesh for the chunk using the currently visible surface voxels*/
	Kiwi::Mesh* _GenerateSurfaceMeshes();

	/*gets pointers to all surrounding chunks and uses this to determine visibility of chunk edges*/
	void _LinkNeighbors();

	/*generates a nav mesh of walkable voxels using the surface voxel data*/
	void _GenerateNavMesh();

	/*intializes the voxel data array and allocates enough space to hold the chunk's voxel data
	returns false if there is insufficient space remaining for the chunk*/
	bool _Allocate();

	/*asynchronously unloads the chunk*/
	void _AsyncUnload() {}

	/*returns the next instance of a surface voxel above the given point in world coordinates
	this function assumes that the passed globalPos is already an even multiple of the voxelSize*/
	Voxel* _FindNextAbovePoint( const Kiwi::Vector3d& globalPos );

public:

	VoxelChunk( VoxelTerrain& terrain, const Kiwi::Vector3d& dimensions );
	~VoxelChunk();

	void Shutdown();

	void FixedUpdate();

	/*destroys every voxel in the chunk*/
	void Unload();

	/*queues a complete build/rebuild of the chunk. anything currently loaded will first be unloaded*/
	void TriggerFullRebuild();

	/*queues a rebuild of all surfaces and meshes*/
	void TriggerSurfaceRebuild();

	void SetBiome( const VoxelTerrain::BiomeParameters& biomeParams );

	/*replaces a the voxel at the given local position
	if a voxel does not exist in that position, or hasn't been loaded, a new voxel is created*/
	void ReplaceVoxelAtPosition( const Kiwi::Vector3d& position, int newVoxelType );

	void RemoveVoxelAtPosition( const Kiwi::Vector3d& position );

	void AddVoxelsAtPosition( const std::vector<Voxel>& voxels ) {}

	/*returns the voxel at the given position*/
	Voxel* FindSurfaceVoxelAtPosition( const Kiwi::Vector3d& globalPos );

	/*returns the first surface voxel under the given position*/
	Voxel* FindSurfaceVoxelUnderPosition( const Kiwi::Vector3d& globalPos );

	/*returns the first surface voxel located above the given position or 0 if there are none in this chunk*/
	Voxel* FindSurfaceVoxelAbovePosition( const Kiwi::Vector3d& globalPos );

	/*finds the closest voxel directly above globalPos that is walkable, up to a max of maxHeight
	a voxel is walkable if there are 'minClearance' number of voxels directly above it with a solidity less than maxSolidity*/
	Voxel* FindWalkableAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int minClearance, unsigned char maxSolidity, unsigned int maxHeight = 0 );

	std::vector<Voxel*> FindVoxelsAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int numVoxels );

	std::vector<Voxel*> FindSurfaceVoxelChunkAroundPosition( const Kiwi::Vector3d& position, const Kiwi::Vector3d& radius ) {}

	/*Takes a local position and returns the local height at that position
	@position: position to check the height for, in world coordinates*/
	double GetMaxHeightAtPosition( const Kiwi::Vector3d& position );

	/*Takes a local position and returns the local height at that position*/
	double GetMaxHeightAtPosition( const Kiwi::Vector2d& position );

	/*takes a local position within the chunk, and returns the y coordinate of the
	highest non-air voxel under that position*/
	double FindHeightUnderPosition( const Kiwi::Vector3d& position );

	VoxelData* GetVoxelDataAtPosition( const Kiwi::Vector3d& position );

	void SetPosition( const Kiwi::Vector3d& position );
	void SetActive( bool active );

	void SetNeighborLeft( VoxelChunk* chunk ) { m_neighborLeft = chunk; }
	void SetNeighborRight( VoxelChunk* chunk ) { m_neighborRight = chunk; }
	void SetNeighborTop( VoxelChunk* chunk ) { m_neighborTop = chunk; }
	void SetNeighborBottom( VoxelChunk* chunk ) { m_neighborBottom = chunk; }
	void SetNeighborFront( VoxelChunk* chunk ) { m_neighborFront = chunk; }
	void SetNeighborBack( VoxelChunk* chunk ) { m_neighborBack = chunk; }

	/*if enabled, all visible voxels will be created and stored from the voxel data*/
	void EnableSurfaceVoxelLoading( bool surfaceVoxelsEnabled ) { m_enableSurfaceVoxels = surfaceVoxelsEnabled; }

	bool IsActive()const { return m_active; }

	bool IsBuilt()const { return m_built; }

	Kiwi::Vector3d GetPosition()const { return m_position; }
	unsigned long GetSurfaceMeshVoxelCount()const { return m_surfaceVoxels.size(); }
	std::unordered_map<Kiwi::Vector3d, Voxel*, hashFunc, equalsFunc>& GetSurfaceVoxels() { return m_surfaceVoxels; }
	VoxelChunk::ChunkStatus GetStatus()const { return m_chunkStatus; }
	VoxelChunk::CHUNK_TASK GetCurrentTask()const { return m_currentTask; }
	NavMesh* GetNavMesh()const { return m_navMesh; }

};

#endif