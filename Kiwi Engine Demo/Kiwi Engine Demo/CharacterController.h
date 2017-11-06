#ifndef _CHARACTERCONTROLLER_H_
#define _CHARACTERCONTROLLER_H_

#include "VoxelTerrain.h"

#include <Core\EngineRoot.h>
#include <Core\Entity.h>
#include <Core\Scene.h>
#include <Core\Component.h>

#include <Core\Events\IGlobalEventListener.h>

#include <Graphics\Camera.h>

#include <unordered_set>

class Selector;

class INPCController;

class CharacterController:
	public Kiwi::Component,
	public Kiwi::ICollisionEventListener,
	public Kiwi::IMouseEventListener,
	public Kiwi::IKeyboardEventListener,
	public Kiwi::IGlobalEventListener
{
protected:

	Kiwi::EngineRoot* m_gameEngine;
	Kiwi::Scene* m_scene;
	Kiwi::Camera* m_camera;
	Kiwi::Transform* m_transform;
	Kiwi::Mesh* m_playerMesh;
	
	VoxelTerrain* m_terrain;

	Kiwi::Entity* m_overlay;

	Kiwi::Vector3d m_curVelocity;

	Kiwi::Vector3d m_curMoveSpeed;

	Kiwi::Vector3d m_firstPersonCameraPosition;

	Kiwi::Vector3d m_gravity;

	Kiwi::Vector3d m_characterScale;

	//INPCController* m_targetNPC;

	/*the closest voxel currently under the crosshair
	only set if an npc is not being targetted and not in combat*/
	Voxel* m_targetVoxel;

	/*stores the currently selected npc
	a visual target npc becomes selected if the player attacks it or interacts with it*/
	INPCController* m_selectedNPC;

	/*stores the npc that is currently being looked at (the crosshair is over it)
	does not need to be the same as the selected target*/
	INPCController* m_visualTargetNPC;

	//stores the item currently equipped in the main hand slot
	//Item* m_mainEquip;

	//stores the item currently equipped in the offhand slot
	//Item* m_offhandEquip;

	std::unordered_set<INPCController*> m_combatants;

	double m_currentHealth;
	double m_maximumHealth;
	double m_currentEP;
	double m_maxEP;
	double m_currentXP;
	double m_xpModifier; //any added xp will be multiplied by this amount

	double m_epRegenPerSecond;
	double m_hpRegenPerSecond;

	double m_curMovementSpeed;
	double m_maxMovementSpeed;
	double m_movementAccel;
	double m_movementDecel;

	double m_maxStepHeight;
	double m_curStepHeight;

	double m_playerHeight;
	int m_clearance; //how tall, in voxels, the player is

	double m_cameraDistance;
	double m_maxCameraDistance;
	double m_minCameraDistance;
	double m_cameraZoomSpeed;
	double m_cameraRotation;

	double m_jumpTime; //time, in seconds, between jumping and being able to jump again
	double m_jumpTimer; //timer keeping track of jump time

	/*max depth/distance to an npc for it to be targetted when the crosshair is over it*/
	double m_maxNPCTargetDepth;

	/*max distance away from an npc to be able to interact with the npc*/
	double m_maxNPCInteractionRange;

	/*max distance away from a voxel for it to still be reachable*/
	double m_maxVoxelRange;

	bool m_noClip;
	bool m_onGround;
	bool m_onTerrain;
	bool m_firstPersonCamera;
	bool m_buildingMode;

protected:

	void _OnAttached();
	void _OnUpdate();
	void _OnFixedUpdate();

	void _UpdateEntity();
	void _UpdateCamera();

	void _ProcessGlobalEvent( Kiwi::GlobalEventPtr e ) {}

	void OnMousePress( const Kiwi::MouseEvent& e );

	void _OnKeyPress( Kiwi::KEY key );

	void _OnTriggerEnter( const Kiwi::CollisionEvent& e );

	void _SetFirstPersonCamera( bool firstPerson );

	/*performs a raycast from the crosshair and returns the closest hit npc within the target distance*/
	INPCController* _RaycastVisualTarget( double maxDepth );

	/*returns the closest voxel under the crosshair up to a max depth of maxDepth*/
	Voxel* _GetTargetVoxel( double maxDepth );

	void _Attack();
	void _Build();

	Kiwi::Vector3d _CalculateFinalPosition( Kiwi::Vector3d startPos, Kiwi::Vector3d startVelocity, double stepSize );

public:

	CharacterController( Kiwi::Entity* overlay );
	~CharacterController() {}

	void AddCombatant( INPCController* combatant );
	void RemoveCombatant( INPCController* combatant );

	void SetXP( double newXP ) { m_currentXP = newXP; }
	void AddXP( double incrementAmount ) { m_currentXP += incrementAmount * m_xpModifier; }

	Kiwi::Camera* GetCamera()const { return m_camera; }
	double GetMaxHealth()const { return m_maximumHealth; }
	double GetCurrentHealth()const { return m_currentHealth; }
	double GetMaxEP()const { return m_maxEP; }
	double GetCurrentEP()const { return m_currentEP; }
	double GetCurrentEPRegen()const { return m_epRegenPerSecond; }
	double GetCurrentHPRegen()const { return m_hpRegenPerSecond; }

	//INPCController* GetTarget()const { return m_targetNPC; }

	INPCController* GetVisualTargetNPC()const { return m_visualTargetNPC; }
	INPCController* GetSelectedNPC()const { return m_selectedNPC; }

	Voxel* GetTargetVoxel()const { return m_targetVoxel; }

};

#endif