#include "VillagerController.h"
#include "CharacterController.h"
#include "VoxelTerrain.h"
#include "Pathfinder.h"
#include "TaskManager.h"

#include <Core\Scene.h>
#include <Core\Entity.h>
#include <Core\EngineRoot.h>

#include <Graphics\Mesh.h>
#include <Graphics\Text.h>

#include <Physics\SphereCollider.h>

#include <vector>

VillagerController::VillagerController( std::wstring name, std::wstring npcName ):
	INPCController( L"Villager", npcName )
{

	m_scene = 0;
	m_terrain = 0;
	m_player = 0;
	m_pathfinder = 0;
	m_mesh = 0;
	m_nameTag = 0;

	m_movementSpeed = 3.5;
	m_minPathUpdateTime = 0.5;
	m_pathUpdateCounter = 0.0;
	m_maxVertMoveDist = 1.5;
	m_maxFallDist = 2.5;

	m_knownToPlayer = false;

}

VillagerController::~VillagerController()
{

}

void VillagerController::_OnAttached()
{

	INPCController::_OnAttached();

	m_scene = m_entity->GetScene();

	if( m_scene )
	{
		m_terrain = m_scene->GetTerrain<VoxelTerrain*>();
		m_player = m_scene->GetPlayerEntity();
	}

	m_player = m_scene->FindEntityWithName( L"Player" );
	assert( m_player != 0 );

	m_mesh = Kiwi::Mesh::Primitive( Kiwi::Mesh::CUBE, m_objectName + L"/mesh" );
	m_mesh->SetShader( L"TerrainShader" );
	m_entity->AttachComponent( m_mesh );

	if( m_mesh )
	{
		m_mesh->GetSubmesh( 0 )->material.SetColor( L"Diffuse", Kiwi::Color( 0.7, 0.4, 0.5, 1.0 ) );
	}

	Kiwi::Transform* entTransform = m_entity->FindComponent<Kiwi::Transform>();

	if( entTransform != 0 )
	{
		entTransform->SetScale( Kiwi::Vector3( 0.45, 0.65, 0.45 ) );
		entTransform->Translate( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
		entTransform->LockAxis( Kiwi::AXIS::X_AXIS, true );
		entTransform->LockAxis( Kiwi::AXIS::Z_AXIS, true );
	}

	Kiwi::Entity* nameTagEntity = m_scene->CreateEntity( m_objectName + L"/NameTagEntity" );
	if( nameTagEntity != 0 )
	{
		m_nameTag = new Kiwi::Text( L"NameTag", L"Lato_20pt_Outline", Kiwi::Vector2d( 150.0, 150.0 ) );
		if( m_nameTag != 0 )
		{
			m_nameTag->SetAlignment( Kiwi::Font::ALIGN_CENTRE );
			if( m_knownToPlayer )
			{
				m_nameTag->SetText( L"<" + m_npcName + L">" );

			} else
			{
				m_nameTag->SetText( L"<Unknown>" );
			}

			nameTagEntity->AttachComponent( m_nameTag );
			nameTagEntity->SetEntityType( Kiwi::Entity::ENTITY_3D );
			nameTagEntity->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( 0.0, entTransform->GetScale().y + 0.1, 0.0 ) );
			nameTagEntity->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( 1.0/150.0, 1.0/150.0, 1.0) );
			m_entity->AttachChild( nameTagEntity );
			nameTagEntity->FindComponent<Kiwi::Transform>()->LockAxis( Kiwi::Z_AXIS, true );
			nameTagEntity->FindComponent<Kiwi::Transform>()->LockAxis( Kiwi::X_AXIS, true );
		}
	}

	Kiwi::Rigidbody* rigidbody = new Kiwi::Rigidbody();
	Kiwi::Collider* collider = rigidbody->AttachComponent( new Kiwi::SphereCollider( 0.45 ) );
	collider->SetTrigger( true );
	collider->AddListener( this );
	rigidbody->SetMass( 60.0 );
	rigidbody->SetKinematic( false );
	m_entity->AttachComponent( rigidbody );

	m_entity->AddTag( L"villager" );

}

void VillagerController::_OnFixedUpdate()
{

	static Kiwi::Vector3d lastPlayerPos;

	if( m_scene )
	{

		if( m_player == 0 )
		{
			m_player = m_scene->FindEntityWithName( L"Player" );

		} else
		{
			Kiwi::EngineRoot* engine = m_scene->GetEngine();
			assert( engine != 0 );

			Kiwi::Transform* entTransform = m_entity->FindComponent<Kiwi::Transform>();
			Kiwi::Transform* pTransform = m_player->FindComponent<Kiwi::Transform>();
			if( entTransform != 0 )
			{
				double fdt = engine->GetGameTimer()->GetFixedDeltaTime();

				if( m_terrain && m_terrain->GetStatus() == VoxelTerrain::TERRAIN_READY )
				{
					entTransform->RotateTowards( pTransform->GetPosition() );

					if( entTransform->GetPosition().y > 0.0 )
					{
						double terrainHeight = m_terrain->FindHeightUnderPosition( entTransform->GetPosition() );
						terrainHeight += entTransform->GetScale().y / 2.0;
						entTransform->SetPosition( Kiwi::Vector3d( entTransform->GetPosition().x, terrainHeight, entTransform->GetPosition().z ) );

					} else
					{
						double maxHeight = m_terrain->FindMaxHeightAtPosition( Kiwi::Vector2d( entTransform->GetPosition().x, entTransform->GetPosition().z ) );

						if( maxHeight > 0.0 )
						{
							maxHeight += entTransform->GetScale().y / 2.0;
							entTransform->SetPosition( Kiwi::Vector3d( entTransform->GetPosition().x, maxHeight, entTransform->GetPosition().z ) );

						} else
						{
							entTransform->SetPosition( Kiwi::Vector3d( entTransform->GetPosition().x, -1.0, entTransform->GetPosition().z ) );
						}
					}
				}
			}

			if( m_nameTag != 0 && m_nameTag->GetEntity() != 0 )
			{
				float dist = m_nameTag->GetEntity()->FindComponent<Kiwi::Transform>()->GetSquareDistance( pTransform->GetGlobalPosition() );

				if( dist > 80.0 )
				{
					m_nameTag->GetEntity()->SetActive( false );

				}else if( m_nameTag->GetEntity()->IsActive() == false && dist <= 80.0 )
				{
					m_nameTag->GetEntity()->SetActive( true );
				}
			}
		}

	}

}

void VillagerController::_OnPlayerInteraction()
{

	if( m_entity && m_nameTag != 0 )
	{
		if( m_knownToPlayer == false )
		{
			m_nameTag->SetText( m_npcName );
		}
		m_knownToPlayer = true;
	}

	if( m_player != 0 )
	{
		TaskManager* tm = m_player->FindComponent<TaskManager>();
		if( tm != 0 )
		{
			if( m_activeTasks.size() == 0 )
			{
				Task* task = tm->CreateRandomTask( m_npcName );
				if( task != 0 )
				{
					m_activeTasks.push_back( task->GetGlobalID() );
				}

			} else
			{
				//if there are active tasks, check if any have been completed and return them and reward the player
				for( auto taskItr = m_activeTasks.begin(); taskItr != m_activeTasks.end();)
				{
					Task* task = tm->FindTask( *taskItr );
					if( task != 0 )
					{
						if( task->IsCompleted() )
						{
							task->SetReturned( true );
							m_assignedTasks.push_back( *taskItr );
							taskItr = m_activeTasks.erase( taskItr );
							continue;
						}
					}

					taskItr++;
				}
			}
		}
	}

}

std::wstring VillagerController::GetNPCName()const
{

	return (m_knownToPlayer) ? m_npcName : L"Unknown";
}