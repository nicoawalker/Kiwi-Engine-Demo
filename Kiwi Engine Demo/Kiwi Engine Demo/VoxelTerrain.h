#ifndef _VOXELTERRAIN_H_
#define _VOXELTERRAIN_H_

#include "VoxelTerrainChunk.h"

#include <KiwiCore.h>
#include <KiwiGraphics.h>
#include <Core/ITerrain.h>

#include <unordered_map>
#include <string>

class NPCStructure;
class VoxelChunk;
class City;

class VoxelTerrain:
	public Kiwi::ITerrain
{
public:

	enum LoadStatus { TERRAIN_READY, TERRAIN_LOADING, TERRAIN_FAILED, TERRAIN_UNLOADED };

	struct Heightmap
	{
		Kiwi::Vector3d dimensions;

		//stores the height values ( z > x > y )
		std::vector<std::vector<double>> map;
	};

	struct VoxelData
	{
		std::vector<Voxel*> voxels;
		VoxelTerrainChunk* chunk;
	};

	/*parameters used to create each unique biome*/
	struct BiomeParameters
	{
		int biomeType;
		int octaveCount;
		double persistance;
		double frequency;
		double amplitude;

		BiomeParameters()
		{
			biomeType = 0;
			octaveCount = 1;
			persistance = 1.0;
			frequency = 1.0;
			amplitude = 1.0;
		}
	};

protected:

	struct Chunk_Y
	{
		std::unordered_map<double, VoxelTerrainChunk*> yChunks;
	};

	struct Chunk_X
	{
		std::unordered_map<double, Chunk_Y> xChunks;
	};

	struct ChunkMap
	{
		std::unordered_map<double, Chunk_X> zChunks;
	};

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

	Kiwi::Scene* m_scene;

	//key is the position of the chunk
	std::vector<std::vector<std::vector<VoxelChunk*>>> m_chunks;
	//std::unordered_map<Kiwi::Vector3d, VoxelChunk*, hashFunc, equalsFunc> m_chunks;
	std::unordered_map<unsigned int, VoxelDefinition*> m_voxelDefinitions;

	Kiwi::Vector3d m_chunkDimensions;
	Kiwi::Vector3d m_leftBottomPos; //position of the chunk in the bottom left corner
	Kiwi::Vector3d m_renderDistance;
	Kiwi::Vector3d m_centerPosition; //current exact center of the map
	Kiwi::Vector3d m_centerLoadOffset; //how far from the center the target must be before the terrain will re-center
	VoxelChunk* m_centerChunk; //chunk that contains the center position

	City* m_startingCity;

	Kiwi::Entity* m_target; //terrain will load around this entity
	
	VoxelChunk* m_playerChunk; //pointer to the chunk the player is within

	LoadStatus m_loadStatus;

	int m_worldSeed;

	unsigned int m_chunkSize;
	//int m_renderDistance;
	double m_voxelSize;

	double m_seaLevel; //empty space under this level, but above the surface level will be filled with ocean
	double m_surfaceLevel; //height value above which the surface features will be generated

	bool m_doResize;

	ChunkMap m_terrain;

	Heightmap m_heightmap;

	long m_voxelCount;

protected:

	/*function used to initialize a chunk with voxel data*/
	VoxelData* _BuildChunk( VoxelData* vData );
	VoxelTerrainChunk* _GenerateChunkMesh( VoxelData* vData );

	/*clamps the position to the voxel grid*/
	Kiwi::Vector3d _ClampPosition( const Kiwi::Vector3d& position ) {}

	VoxelChunk* _FindChunkContainingPosition( const Kiwi::Vector3d& position );

	VoxelChunk* _GetCenterChunk();

	/*unloads and completely regenerates the map around the current center point*/
	void _GenerateAroundCenter();

	/*regenerates the map around the current center point, loading and unloading chunks as necessary*/
	void _ScrollToCenter();

	void _ClearTerrain();

	void _ResizeTerrain();

public:

	VoxelTerrain( Kiwi::Scene* scene, short chunkSize, unsigned int loadChunkDistance );
	~VoxelTerrain();

	void FixedUpdate();

	void AddVoxelDefinition( VoxelDefinition* definition ) {}

	void OnChunkLoaded( VoxelChunk* chunk );

	void ReplaceVoxelContainingPosition( const Kiwi::Vector3d& globalPosition );
	void RemoveVoxelContainingPosition( const Kiwi::Vector3d& position );

	/*if there are any voxels at the given position it will return the maximum height
	if there are no voxels at the position it will return -1*/
	double FindMaxHeightAtPosition( const Kiwi::Vector3d& position );
	double FindMaxHeightAtPosition( const Kiwi::Vector2d& position );

	/*returns the height of the highest voxel under position*/
	double FindHeightUnderPosition( const Kiwi::Vector3d& position );

	Voxel* FindSurfaceVoxelAtPosition( const Kiwi::Vector3d& position );

	Voxel* FindSurfaceVoxelUnderPosition( const Kiwi::Vector3d& position );

	Voxel* FindSurfaceVoxelAbovePosition( const Kiwi::Vector3d& position );

	/*finds the closest voxel directly above globalPos that is walkable, up to a max of maxHeight
	a voxel is walkable if there are 'minClearance' number of voxels directly above it with a solidity less than maxSolidity*/
	Voxel* FindWalkableVoxelAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int minClearance, unsigned char maxSolidity, unsigned int maxHeight = 0 );

	/*returns a vector containing 'numVoxels' voxel pointers above the given position*/
	std::vector<Voxel*> FindVoxelsAbovePoint( const Kiwi::Vector3d& globalPos, unsigned int numVoxels );

	VoxelChunk* FindChunkContainingPosition( const Kiwi::Vector3d& position );

	void SetTarget( Kiwi::Entity* entity ) { m_target = entity; }

	void SetWorldSeed( int seed ) { m_worldSeed = seed; }
	void SetVoxelSize( double size ) { m_voxelSize = size; }
	void SetSeaLevel( double waterLevel ) { m_seaLevel = waterLevel; }

	void SetRenderDistance( const Kiwi::Vector3d& renderDistance );

	void SetCenterPosition( const Kiwi::Vector3d& position );

	Kiwi::Scene* GetScene()const { return m_scene; }

	VoxelDefinition* GetVoxelDefinition( unsigned int voxelType );

	unsigned int GetWorldSeed()const { return m_worldSeed; }

	long GetGeneratedVoxelCount()const { return m_voxelCount; }
	long GetRenderedVoxelCount();

	LoadStatus GetStatus()const { return m_loadStatus; }

	Kiwi::Entity* GetTarget()const { return m_target; }

	double GetVoxelSize()const { return m_voxelSize; }
	double GetSeaLevel()const { return m_seaLevel; }
	double GetSurfaceLevel()const { return m_surfaceLevel; }

	const Kiwi::Vector3d& GetCenterPosition()const { return m_centerPosition; }

	const Kiwi::Vector3d& GetRenderDistance()const { return m_renderDistance; }

};

#endif