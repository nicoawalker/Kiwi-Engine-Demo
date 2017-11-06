#ifndef _VILLAGERCONTROLLER_H_
#define _VILLAGERCONTROLLER_H_

#include <string>

#include "INPCController.h"

#include <Physics\ICollisionEventListener.h>

namespace Kiwi
{
	class Scene;
	class Mesh;
	class Text;
}

class VoxelTerrain;
class Pathfinder;

class VillagerController :
	public INPCController,
	public Kiwi::ICollisionEventListener
{
protected:

	Kiwi::Scene* m_scene;
	Kiwi::Entity* m_player;

	Kiwi::Mesh* m_mesh;

	Kiwi::Text* m_nameTag;

	VoxelTerrain* m_terrain;
	Pathfinder* m_pathfinder;

	/*stores the global ids of all tasks the npc has assigned*/
	std::vector<int> m_assignedTasks;

	/*stores the global ids of all assigned tasks that have not yet been completed*/
	std::vector<int> m_activeTasks;

	//maximum height this villager is able to climb or jump to
	double m_maxVertMoveDist;

	//maximum height this villager is able to fall without taking damage
	double m_maxFallDist;

	double m_movementSpeed;

	double m_minPathUpdateTime;
	double m_pathUpdateCounter;

	bool m_knownToPlayer;

protected:

	void _OnAttached();
	void _OnFixedUpdate();

	void _OnPlayerInteraction();

public:

	VillagerController( std::wstring name, std::wstring npcName = L"" );
	~VillagerController();

	std::wstring GetNPCName()const;

};


#endif