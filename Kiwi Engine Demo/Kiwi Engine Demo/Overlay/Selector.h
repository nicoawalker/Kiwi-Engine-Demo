#ifndef _SELECTOR_H_
#define _SELECTOR_H_

#include <Core\Component.h>
#include <Core\Vector3d.h>

#include "Core\Events\IKeyboardEventListener.h"

#include "../INPCController.h"

namespace Kiwi
{
	class Scene;
	class Mesh;
}

class CharacterController;
class VillagerController;
class Voxel;
class VoxelTerrain;
class SelectionPanel;

class Selector :
	public Kiwi::Component,
	public Kiwi::IKeyboardEventListener
{
protected:

	Kiwi::Scene* m_scene;

	//Kiwi::Entity* m_crosshairEntity;

	Kiwi::Entity* m_voxelSelector;
	Kiwi::Entity* m_crosshair;

	Kiwi::Mesh* m_crosshairMesh;
	Kiwi::Mesh* m_circleMesh;
	Kiwi::Mesh* m_cubeMesh;

	VoxelTerrain* m_terrain;

	Kiwi::Vector3d m_circleMaxScale;

	CharacterController* m_controller;

	INPCController* m_selectedNPC; //npc the cursor is hovering over
	INPCController* m_targetedNPC; //npc that has been targeted by pressed e when selected

	Voxel* m_currentVoxel;

	double m_radius;
	double m_circleMinRadius;
	double m_circleMaxRadius;
	double m_circleGrowTime; //how long it takes for the circle to grow to maximum size
	double m_circleShrinkTime;
	double m_maxDistance; // maximum distance from the player
	double m_maxTargetDistance;

	double m_crosshairRadius;

	bool m_crosshairEnabled;

protected:

	void _OnAttached();

	void _OnFixedUpdate();

	void _CreateCrosshair( unsigned int pointCount );
	void _CreateVoxelSelector( double radius );

public:

	Selector( CharacterController* controller, double maxDistance );
	~Selector();

	Voxel* GetSelectedVoxel()const { return m_currentVoxel; }

	void SetTarget( INPCController* target ) { m_targetedNPC = target; }

	INPCController* GetSelectedNPC()const { return m_targetedNPC; }

};

#endif