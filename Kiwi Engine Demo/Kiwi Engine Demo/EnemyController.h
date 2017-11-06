#ifndef _ENEMYCONTROLLER_H_
#define _ENEMYCONTROLLER_H_

#include <string>

#include "INPCController.h"

#include <Physics\ICollisionEventListener.h>

namespace Kiwi
{
	class Scene;
	class Mesh;
}

class VoxelTerrain;
class Pathfinder;

class EnemyController :
	public INPCController,
	public Kiwi::ICollisionEventListener
{
protected:

	Kiwi::Scene* m_scene;
	Kiwi::Entity* m_player;
	Kiwi::Entity* m_overlay;

	Kiwi::Mesh* m_mesh;

	VoxelTerrain* m_terrain;
	Pathfinder* m_pathfinder;

	//maximum height this villager is able to climb or jump to
	double m_maxVertMoveDist;

	//maximum height this villager is able to fall without taking damage
	double m_maxFallDist;

	double m_movementSpeed;

	double m_minPathUpdateTime;
	double m_pathUpdateCounter;

	double m_attackSpeed;

	bool m_attackingPlayer;

protected:

	void _OnAttached();
	void _OnFixedUpdate();

	void _OnTriggerEnter( const Kiwi::CollisionEvent& e );

	void _Attack();

	void _Die();

public:

	EnemyController( std::wstring name, Kiwi::Entity* overlay );
	~EnemyController();

	double GetMaxHealth()const { return m_maxHealth; }
	double GetHealth()const { return m_currentHealth; }

};

#endif