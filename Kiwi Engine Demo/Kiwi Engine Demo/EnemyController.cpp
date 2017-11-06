#include "EnemyController.h"
#include "CharacterController.h"
#include "VoxelTerrain.h"
#include "Pathfinder.h"
#include "Projectile.h"

#include "Overlay\Selector.h"

#include "Events\CombatEvent.h"

#include <Core\Scene.h>
#include <Core\Entity.h>
#include <Core\EngineRoot.h>

#include <Graphics\Mesh.h>

#include <Physics\SphereCollider.h>

#include <vector>
#include <random>

EnemyController::EnemyController( std::wstring name, Kiwi::Entity* overlay ) :
	INPCController( name, L"Villager" )
{

	m_scene = 0;
	m_terrain = 0;
	m_player = 0;
	m_pathfinder = 0;
	m_mesh = 0;
	m_npcID = 1;

	m_hasTask = false;
	m_attackingPlayer = false;

	m_overlay = overlay;

	m_movementSpeed = 3.5;
	m_minPathUpdateTime = 0.5;
	m_pathUpdateCounter = 0.0;
	m_maxVertMoveDist = 1.5;
	m_maxFallDist = 2.5;

	m_maxHealth = 100;
	m_currentHealth = 100;

	m_attackSpeed = 2.0;

}

EnemyController::~EnemyController()
{
	
}

void EnemyController::_OnAttached()
{

	INPCController::_OnAttached();

	m_scene = m_entity->GetScene();

	if( m_scene )
	{
		m_terrain = m_scene->GetTerrain<VoxelTerrain*>();
		m_player = m_scene->GetPlayerEntity();
	}

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
		entTransform->SetScale( Kiwi::Vector3( 0.25, 0.25, 0.25 ) ); 
		entTransform->Translate( Kiwi::Vector3d( 0.0, -1.0, 0.0 ) );
		entTransform->LockAxis( Kiwi::AXIS::X_AXIS, true );
		entTransform->LockAxis( Kiwi::AXIS::Z_AXIS, true );
	}

	Kiwi::Rigidbody* rigidbody = new Kiwi::Rigidbody();
	Kiwi::Collider* collider = rigidbody->AttachComponent( new Kiwi::SphereCollider( 0.45 ) );
	collider->SetTrigger( true );
	collider->AddListener( this );
	rigidbody->SetMass( 60.0 );
	rigidbody->SetKinematic( false );
	m_entity->AttachComponent( rigidbody );

	m_entity->AddTag( L"enemy" );
	m_entity->AddTag( L"hostile" );

	m_npcName = L"Enemy 1";

}

void EnemyController::_Attack()
{

	static int projNum = 0;
	projNum++;

	Kiwi::Transform* entTransform = m_entity->FindComponent<Kiwi::Transform>();

	Kiwi::Entity* projectile = m_scene->CreateEntity( m_objectName + L"Projectile" + Kiwi::ToWString( projNum ) );
	projectile->AttachComponent( Kiwi::Mesh::Primitive( Kiwi::Mesh::CUBE ) );

	std::random_device rd;
	std::mt19937 e2( rd() );
	std::uniform_real_distribution<> dist( 0.0, 1.0 );

	double radius = dist( e2 ) * 0.5;

	projectile->AttachComponent( new Projectile( m_entity, radius, entTransform->GetForward() * 25.0 + Kiwi::Vector3( 0.0, 1.0, 0.0 ), radius * 2000.0 ) );
	projectile->FindComponent<Kiwi::Transform>()->SetPosition( entTransform->GetPosition() + entTransform->GetForward() * 0.5 + Kiwi::Vector3d( 0.0, 0.2, 0.0 ) );

	projectile->AddTag( L"projectile" );

}

void EnemyController::_Die()
{

	Kiwi::Scene* scene = m_entity->GetScene();

	m_attackingPlayer = false;
	m_charController->RemoveCombatant( this );

	std::shared_ptr<CombatEvent> sp = std::make_shared<CombatEvent>( m_entity, NPC_KILLED );
	scene->BroadcastGlobalEvent( sp );

	if( m_isTargeted && m_overlay != 0 )
	{
		Selector* selector = m_overlay->FindComponent<Selector>();
		if( selector != 0 )
		{
			selector->SetTarget( 0 );
		}
	}

	m_currentHealth = 0.0;
	m_entity->Shutdown();

}

void EnemyController::_OnTriggerEnter( const Kiwi::CollisionEvent& e )
{

	Kiwi::Collider* collider = e.GetTarget();
	if( collider != 0 && collider->GetEntity() != 0 && collider->GetEntity()->HasTag( L"projectile" ) )
	{
		Projectile* projectile = collider->GetEntity()->FindComponent<Projectile>();
		if( projectile != 0 && projectile->GetSource() != m_entity )
		{
			if( projectile->GetSource() == m_charController->GetEntity() )
			{
				m_attackingPlayer = true;
				m_charController->AddCombatant( this );
			}
			m_currentHealth -= projectile->GetForce();
		}
	}

}

void EnemyController::_OnFixedUpdate()
{

	static Kiwi::Vector3d lastPlayerPos;

	if( m_currentHealth <= 0.0 )
	{
		this->_Die();
		return;
	}

	if( m_scene )
	{
		if( m_player == 0 )
		{
			m_player = m_scene->GetPlayerEntity();
			m_charController = m_player->FindComponent<CharacterController>();

		} else
		{

			if( m_charController == 0 )
			{
				if( m_player != 0 )
				{
					m_charController = m_player->FindComponent<CharacterController>();
				}
				return;
			}

			Kiwi::Transform* entTransform = m_entity->FindComponent<Kiwi::Transform>();
			Kiwi::Transform* pTransform = m_player->FindComponent<Kiwi::Transform>();

			Kiwi::EngineRoot* engine = m_scene->GetEngine();
			assert( engine != 0 );
			double fdt = engine->GetGameTimer()->GetFixedDeltaTime();

			static double attackTimer = 0.0;

			if( m_attackingPlayer || entTransform->GetSquareDistance( pTransform->GetPosition() ) <= 100.0 )
			{
				m_attackingPlayer = true;
				m_charController->AddCombatant( this );
				attackTimer += fdt;

				entTransform->RotateTowards( pTransform->GetPosition() );

				if( attackTimer >= m_attackSpeed )
				{
					attackTimer -= m_attackSpeed;
					this->_Attack();
				}

			} else
			{
				attackTimer = 0.0;
				if( m_attackingPlayer )
				{
					m_attackingPlayer = false;
					m_charController->RemoveCombatant( this );
				}

				if( m_terrain && m_terrain->GetStatus() == VoxelTerrain::TERRAIN_READY )
				{
					if( m_pathfinder != 0 )
					{
						if( pTransform != 0 )
						{
							m_pathUpdateCounter += fdt;
							if( m_pathUpdateCounter >= m_minPathUpdateTime && (m_pathfinder->GetTargetNode() == 0 || pTransform->GetPosition() != lastPlayerPos) )
							{
								m_pathUpdateCounter = 0.0;
								lastPlayerPos = pTransform->GetPosition();
								Kiwi::Vector3d targetPos = pTransform->GetPosition() - pTransform->GetForward();

								Voxel* targetVoxel = m_terrain->FindSurfaceVoxelAtPosition( targetPos );
								if( targetVoxel != 0 )
								{
									m_pathfinder->GeneratePath( targetVoxel->GetPosition() );

								} else
								{
									targetVoxel = m_terrain->FindWalkableVoxelAbovePoint( targetPos, 1, 0 );
									if( targetVoxel != 0 )
									{
										m_pathfinder->GeneratePath( targetVoxel->GetPosition() );

									} else
									{
										targetVoxel = m_terrain->FindSurfaceVoxelUnderPosition( pTransform->GetPosition() - pTransform->GetForward() );
										if( targetVoxel != 0 )
										{
											m_pathfinder->GeneratePath( targetVoxel->GetPosition() );
										}
									}
								}

							} else if( m_pathfinder->GetTargetNode() != 0 )
							{
								entTransform->RotateTowards( m_pathfinder->GetTargetNode()->position );

								Kiwi::Vector3d velocity = entTransform->GetForward() * m_movementSpeed * fdt;
								velocity.y = 0.0;
								if( entTransform->GetSquareDistance( m_pathfinder->GetTargetNode()->position ) < entTransform->GetSquareDistance( entTransform->GetPosition() + velocity ) )
								{
									velocity = m_pathfinder->GetTargetNode()->position - entTransform->GetPosition();
								}
								entTransform->Translate( velocity );

							} else if( m_pathfinder->GetTargetNode() == 0 )
							{
								entTransform->RotateTowards( pTransform->GetPosition() );
							}
						}

					} else
					{
						m_pathfinder = m_entity->FindComponent<Pathfinder>();
						if( m_pathfinder != 0 )
						{
							m_pathfinder->SetMaxJump( 1 );
							m_pathfinder->SetMaxFall( 4 );
							m_pathfinder->SetMinClearance( 1 );
						}
					}

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
		}

	}

}