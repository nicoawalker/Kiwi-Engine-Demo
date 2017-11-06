#include "CharacterController.h"
#include "DefaultShader.h"
#include "Projectile.h"
#include "TaskManager.h"

#include "Overlay/Selector.h"
#include "Overlay\SelectionPanel.h"

#include <KiwiCore.h>
#include <KiwiGraphics.h>
#include <KiwiPhysics.h>

CharacterController::CharacterController( Kiwi::Entity* overlay ):
	Kiwi::Component( L"CharacterController" )
{

	m_gameEngine = 0;
	m_scene = 0;
	m_camera = 0;
	m_transform = 0;
	m_terrain = 0;
	m_overlay = overlay;
	//m_targetNPC = 0;
	m_targetVoxel = 0;

	m_curMovementSpeed = 0.0;
	m_maxMovementSpeed = 4.0;
	m_movementAccel = 0.5;
	m_movementDecel = 0.5;

	m_playerHeight = 1.9;
	m_characterScale.Set( 0.9, 1.9, 0.8 );

	m_firstPersonCameraPosition.Set( 0.0, (m_playerHeight / 2.0) - 0.2, 0.0 );

	m_maxStepHeight = 0.7;
	m_curStepHeight = 0.7;
	m_minCameraDistance = 1.0;
	m_maxCameraDistance = 10.0;
	m_cameraZoomSpeed = 30.0;
	m_cameraRotation = Kiwi::ToRadians( 45 );
	m_cameraDistance = 0.0;
	m_jumpTime = 1.0;
	m_jumpTimer = 0.0;

	m_maximumHealth = 1000;
	m_currentHealth = 1000;
	m_maxEP = 100;
	m_currentEP = 100;
	m_epRegenPerSecond = 1.5;
	m_hpRegenPerSecond = m_maximumHealth / 300.0;
	m_xpModifier = 1.0;
	m_currentXP = 0.0;

	m_visualTargetNPC = 0;
	m_selectedNPC = 0;
	m_targetVoxel = 0;

	m_maxNPCTargetDepth = 20;
	m_maxNPCInteractionRange = 3.5;
	m_maxVoxelRange = 7.0;

	m_noClip = false;
	m_onTerrain = false;
	m_onGround = false;
	m_firstPersonCamera = false;
	m_buildingMode = false;

}

void CharacterController::_OnAttached()
{

	if( m_entity )
	{
		m_scene = m_entity->GetScene();
		assert( m_scene != 0 );
		m_gameEngine = m_scene->GetEngine();
		assert( m_gameEngine != 0 );
		m_transform = m_entity->FindComponent<Kiwi::Transform>();
		assert( m_transform != 0 );

		/*create the player mesh*/
		m_playerMesh = Kiwi::Mesh::Cube();
		m_entity->AttachComponent( m_playerMesh );
		m_playerMesh->GetSubmesh( 0 )->material.SetColor( L"Diffuse", Kiwi::Color( 0.5, 0.5, 0.5, 1.0 ) );
		m_transform->SetScale( Kiwi::Vector3( 0.9f, m_playerHeight, 0.8f ) );

		m_transform->LockAxis( Kiwi::X_AXIS, true );
		m_transform->LockAxis( Kiwi::Z_AXIS, true );

		/*create the player camera*/
		m_camera = m_scene->CreateCamera( L"MainCam" );
		m_camera->SetFarClipDistance( 1000.0f );
		m_camera->SetNearClipDistance( 0.01f );
		Kiwi::Transform* cameraTransform = m_camera->FindComponent<Kiwi::Transform>();
		if( cameraTransform )
		{
			cameraTransform->SetPosition( *m_transform );
			Kiwi::Quaternion cameraRot( m_transform->GetRight(), m_cameraRotation );
			cameraTransform->SetPosition( m_transform->GetForward() * -5.0f );
			cameraTransform->SetPosition( cameraRot.RotatePoint( cameraTransform->GetPosition() ) );
			cameraTransform->Translate( m_transform->GetPosition() );
			cameraTransform->RotateTowards( *m_transform );
			m_camera->Update();

			m_cameraDistance = m_transform->GetDistance( cameraTransform );
		}
		//m_entity->AttachChild( m_camera );

		Kiwi::Rigidbody* rigidbody = m_entity->AttachComponent( new Kiwi::Rigidbody() );
		if( rigidbody != 0 )
		{
			rigidbody->SetKinematic( false );
			Kiwi::Collider* collider = rigidbody->AttachComponent( new Kiwi::SphereCollider( 1.0 ) );
			collider->AddListener( this );
		}

		TaskManager* tm = new TaskManager();
		m_entity->AttachComponent( tm );

		Kiwi::RenderTarget* renderTarget = m_scene->FindRenderTargetWithName( L"BackBuffer" );
		if( renderTarget == 0 )
		{
			throw Kiwi::Exception( L"PlayerAvatar", L"Could not find render target" );
		}

		if( renderTarget->GetViewportCount() > 0 )
		{
			renderTarget->GetViewport( 0 )->AttachCamera( m_camera );
		}

		m_scene->GetRenderWindow()->AddMouseListener( this );
		m_scene->GetRenderWindow()->AddInputListener( this );

		m_terrain = m_scene->GetTerrain<VoxelTerrain*>();

		this->_SetFirstPersonCamera( true );

		m_entity->AddTag( L"player" );

	}

}

void CharacterController::_OnUpdate()
{
	static int i = 0;
	i++;
	if( i % 2 == 0 )
	{
		if( m_entity != 0 )
		{

			double deltaTime = m_scene->GetEngine()->GetGameTimer()->GetDeltaTime();

			m_hpRegenPerSecond = m_maximumHealth / 500.0;
			if( m_currentHealth != m_maximumHealth )
			{
				m_currentHealth += m_hpRegenPerSecond * deltaTime;
				Kiwi::clamp( m_currentHealth, 0.0, m_maximumHealth );
			}

			if( m_currentEP != m_maxEP )
			{
				m_currentEP += m_epRegenPerSecond * deltaTime;
				Kiwi::clamp( m_currentEP, 0.0, m_maxEP );
			}

			if( m_terrain == 0 )
			{
				m_terrain = m_scene->GetTerrain<VoxelTerrain*>();
				//m_gameEngine->GetConsole()->PrintDebug( L"Player is missing terrain pointer" );
				return;
			}

			Kiwi::RawInputWrapper* inputDevice = &m_gameEngine->GetGameWindow()->GetInput();

			this->_UpdateEntity();
			this->_UpdateCamera();

		}
	}

}

void CharacterController::_OnFixedUpdate()
{

	if( m_entity != 0 && m_terrain != 0 )
	{

		if( m_selectedNPC != 0 && m_selectedNPC->GetEntity()->FindComponent<Kiwi::Transform>()->GetDistance(m_transform) <= m_maxNPCTargetDepth )
		{ //clear the current selection if it is too far away
			m_selectedNPC = 0;
		}

		//get the npc under the crosshair
		m_visualTargetNPC = this->_RaycastVisualTarget( m_maxNPCTargetDepth );

		if( m_buildingMode )
		{
			m_targetVoxel = this->_GetTargetVoxel( m_maxVoxelRange );

		} else
		{
			m_targetVoxel = 0;
		}

		if( m_onTerrain )
		{
			if( m_terrain->FindChunkContainingPosition( m_transform->GetPosition() ) == 0 )
			{
				m_onTerrain = false;
			}

		} else
		{
			double height = m_terrain->FindMaxHeightAtPosition( Kiwi::Vector2d( m_transform->GetPosition().x, m_transform->GetPosition().z ) );
			if( height >= 0.0 )
			{
				m_onTerrain = true;
				m_isActive = true;
				m_transform->SetPosition( Kiwi::Vector3d( m_transform->GetPosition().x, height + (m_playerHeight / 2.0), m_transform->GetPosition().z ) );

			} else
			{
				m_onTerrain = false;
				//m_gameEngine->GetConsole()->PrintDebug( L"Terrain under player has no height" );
			}
		}

	}

}

void CharacterController::_OnTriggerEnter( const Kiwi::CollisionEvent& e )
{

	Kiwi::Collider* collider = e.GetTarget();
	if( collider != 0 && collider->GetEntity() != 0 && collider->GetEntity()->HasTag(L"projectile") )
	{
		Projectile* projectile = collider->GetEntity()->FindComponent<Projectile>();
		if( projectile != 0 && projectile->GetSource() != m_entity )
		{
			m_currentHealth -= projectile->GetForce();

			/*Selector* sel = m_overlay->FindComponent<Selector>();
			if( sel != 0 )
			{
				Kiwi::Entity* source = projectile->GetSource();
				
				INPCController* npc = source->FindComponent<INPCController>();
				if( npc != 0 && m_targetNPC == 0 )
				{
					npc->Target();
					sel->SetTarget( npc );
					m_targetNPC = npc;
				}
			}*/
		}
	}

}

void CharacterController::_SetFirstPersonCamera( bool firstPerson )
{

	if( firstPerson && !m_firstPersonCamera )
	{

		m_firstPersonCamera = true;
		m_playerMesh->SetActive( false );
		m_camera->FindComponent<Kiwi::Transform>()->SetRotation( m_entity->FindComponent<Kiwi::Transform>()->GetRotation() );

	} else
	{
		m_firstPersonCamera = false;
		m_playerMesh->SetActive( true );
	}

}

void CharacterController::OnMousePress( const Kiwi::MouseEvent& e )
{

	if( !m_buildingMode )
	{
		this->_Attack();

	} else if( m_targetVoxel != 0 )
	{
		this->_Build();
	}

}

void CharacterController::_OnKeyPress( Kiwi::KEY key )
{

	switch( key )
	{
		case Kiwi::KEY::C:
			{
				m_buildingMode = !m_buildingMode;
				break;
			}
		case Kiwi::KEY::E:
			{
				if( m_visualTargetNPC != 0 )
				{
					if( m_visualTargetNPC != m_selectedNPC )
					{
						Kiwi::Transform* npcTransform = m_visualTargetNPC->GetEntity()->FindComponent<Kiwi::Transform>();
						if( npcTransform && npcTransform->GetDistance( m_transform ) <= m_maxNPCInteractionRange )
						{
							m_selectedNPC = m_visualTargetNPC;
						}
					}

				} else
				{
					m_selectedNPC = 0;
				}
				
				if( m_selectedNPC != 0 )
				{
					Kiwi::Transform* npcTransform = m_selectedNPC->GetEntity()->FindComponent<Kiwi::Transform>();
					if( npcTransform && npcTransform->GetDistance( m_transform ) <= m_maxNPCInteractionRange )
					{
						m_selectedNPC->Interact();

					} else
					{ //too far away, clear the selection
						m_selectedNPC = 0;
					}
				}
				break;
			}
		case Kiwi::KEY::Tab:
			{
				m_noClip = !m_noClip;
				break;
			}
		case Kiwi::KEY::Add:
			{
				if( m_terrain != 0 )
				{
					m_terrain->SetRenderDistance( m_terrain->GetRenderDistance() + Kiwi::Vector3d( 1.0, 1.0, 1.0 ) );
					break;
				}
			}
		case Kiwi::KEY::Subtract:
			{
				if( m_terrain != 0 )
				{
					m_terrain->SetRenderDistance( m_terrain->GetRenderDistance() - Kiwi::Vector3d( 1.0, 1.0, 1.0 ) );
					break;
				}
			}
		case Kiwi::KEY::RightArrow:
			{
				if( m_terrain != 0 )
				{
					m_terrain->SetCenterPosition( m_terrain->GetCenterPosition() + Kiwi::Vector3d(16.0, 0.0, 0.0) );
					break;
				}
			}
		default:return;
	}

}

void CharacterController::_UpdateEntity()
{

	Kiwi::PhysicsSystem* physics = m_scene->GetPhysicsSystem();
	Kiwi::RawInputWrapper* inputDevice = &m_gameEngine->GetGameWindow()->GetInput();

	if( !m_noClip && m_onTerrain )
	{
		Kiwi::Vector2 mouseDelta = inputDevice->GetMouseDeltaPosition();
		short mouseWheelDelta = inputDevice->GetMouseWheelDelta();

		float mouseXAngle = Kiwi::ToRadians( mouseDelta.x );
		float mouseYAngle = Kiwi::ToRadians( mouseDelta.y );

		if( !m_firstPersonCamera )
		{
			m_transform->Rotate( Kiwi::Vector3::up(), mouseXAngle );
		} else
		{
			m_transform->SetRotation( Kiwi::Quaternion() );
			m_transform->Rotate( m_camera->FindComponent<Kiwi::Transform>()->GetRotation() );
		}

		float deltaTime = m_gameEngine->GetGameTimer()->GetDeltaTime();
		m_jumpTimer -= deltaTime;
		Kiwi::clamp( m_jumpTimer, 0.0, 6000.0 );

		Kiwi::Vector3d curPos = m_transform->GetPosition(); //current position before movement

		double footHeight = m_transform->GetPosition().y - (m_playerHeight / 2.0);
		double voxelSize = m_terrain->GetVoxelSize();
		double halfVoxel = voxelSize / 2.0;
		double moveSpeed = m_maxMovementSpeed;
		m_clearance = (int)std::ceil( m_playerHeight / voxelSize );
		int stepVoxels = (int)(m_curStepHeight / voxelSize);

		Kiwi::Vector3d gravity;
		if( physics != 0 )
		{
			gravity = physics->GetGravity();
			m_gravity = physics->GetGravity();
		}

		if( m_onGround && m_currentEP >= 6.0 && inputDevice->QueryKeyState( Kiwi::KEY::Shift, Kiwi::KEY_ACTIVE ) )
		{
			moveSpeed *= 2.0;
			m_currentEP -= 6.0 * deltaTime;
		}

		Kiwi::Vector3d newMovementVel;
		if( inputDevice->QueryKeyState( Kiwi::KEY::A, Kiwi::KEY_ACTIVE ) )
		{
			newMovementVel = m_transform->GetRight() * -1.0;

		} else if( inputDevice->QueryKeyState( Kiwi::KEY::D, Kiwi::KEY_ACTIVE ) )
		{
			newMovementVel = m_transform->GetRight();

		} 

		if( inputDevice->QueryKeyState( Kiwi::KEY::W, Kiwi::KEY_ACTIVE ) )
		{
			newMovementVel += m_transform->GetForward();

		} else if( inputDevice->QueryKeyState( Kiwi::KEY::S, Kiwi::KEY_ACTIVE ) )
		{
			newMovementVel += m_transform->GetForward() * -1.0;
		}

		newMovementVel = newMovementVel.Normalized() * moveSpeed;

		newMovementVel += gravity * deltaTime;
		newMovementVel.y += m_curVelocity.y;

		if( m_jumpTimer == 0 && m_onGround && (inputDevice->QueryKeyState( Kiwi::KEY::Space, Kiwi::KEY_PRESSED )) )
		{
			newMovementVel.y += 8.0;
			m_curStepHeight = 0.0;
			m_jumpTimer = m_jumpTime;
			m_onGround = false;
		}

		Kiwi::Vector3d newPos = this->_CalculateFinalPosition( Kiwi::Vector3d( curPos.x, footHeight, curPos.z ), newMovementVel * deltaTime, 0.1 );
		newPos.y += curPos.y - footHeight;
		if( m_onGround )
		{
			newMovementVel.y = 0.0;
		}

		m_transform->SetPosition( newPos );
		m_curVelocity = newMovementVel;
	}

}

void CharacterController::_UpdateCamera()
{

	if( m_entity != 0 && m_camera != 0 )
	{

		float deltaTime = m_gameEngine->GetGameTimer()->GetDeltaTime();

		Kiwi::RawInputWrapper* inputDevice = &m_gameEngine->GetGameWindow()->GetInput();

		Kiwi::Vector2 mouseDelta = inputDevice->GetMouseDeltaPosition();
		short mouseWheelDelta = inputDevice->GetMouseWheelDelta();

		float mouseXAngle = Kiwi::ToRadians( mouseDelta.x );
		float mouseYAngle = Kiwi::ToRadians( mouseDelta.y );

		m_cameraRotation += mouseYAngle;
		Kiwi::clamp( m_cameraRotation, (double)Kiwi::ToRadians( -80.0 ), (double)Kiwi::ToRadians( 80.0 ) );

		Kiwi::Transform* cameraTransform = m_camera->FindComponent<Kiwi::Transform>();

		if( !m_noClip )
		{
			if( !m_firstPersonCamera )
			{
				//calculate camera movement and position
				if( mouseWheelDelta != 0 )
				{
					(mouseWheelDelta > 0) ? m_cameraDistance -= deltaTime * m_cameraZoomSpeed : m_cameraDistance += deltaTime * m_cameraZoomSpeed;
					Kiwi::clamp( m_cameraDistance, m_minCameraDistance, m_maxCameraDistance );
				}

				Kiwi::Quaternion cameraRot( m_transform->GetRight(), m_cameraRotation );
				cameraTransform->SetPosition( cameraRot.RotatePoint( m_transform->GetForward() * -m_cameraDistance ) );
				cameraTransform->Translate( m_transform->GetPosition() );

				Kiwi::Vector3d focalPos = m_transform->GetPosition();
				focalPos.y += m_transform->GetScale().y;
				cameraTransform->RotateTowards( focalPos );

			} else
			{
				cameraTransform->Rotate( Kiwi::Vector3d::up(), mouseXAngle);
				cameraTransform->Rotate( cameraTransform->GetRight(), mouseYAngle );
				cameraTransform->SetPosition( m_transform->GetPosition() + m_firstPersonCameraPosition );
			}

		} else
		{
			cameraTransform->Rotate( cameraTransform->GetUp(), mouseXAngle );
			cameraTransform->Rotate( cameraTransform->GetRight(), mouseYAngle );

			Kiwi::Vector3d moveVelocity;

			if( inputDevice->QueryKeyState( Kiwi::KEY::A, Kiwi::KEY_ACTIVE ) )
			{
				moveVelocity = cameraTransform->GetRight() * -1.0;

			} else if( inputDevice->QueryKeyState( Kiwi::KEY::D, Kiwi::KEY_ACTIVE ) )
			{
				moveVelocity = cameraTransform->GetRight();
			}

			if( inputDevice->QueryKeyState( Kiwi::KEY::W, Kiwi::KEY_ACTIVE ) )
			{
				moveVelocity += cameraTransform->GetForward();

			} else if( inputDevice->QueryKeyState( Kiwi::KEY::S, Kiwi::KEY_ACTIVE ) )
			{
				moveVelocity += cameraTransform->GetForward() * -1.0;
			}

			moveVelocity = moveVelocity.Normalized() * 20.0 * deltaTime;

			cameraTransform->Translate( moveVelocity );
			//m_camera->Update();
		}

	}

}

INPCController* CharacterController::_RaycastVisualTarget( double maxDepth )
{

	std::vector<Kiwi::Entity*> rayHits;
	if( m_scene->CastRay( m_camera->FindComponent<Kiwi::Transform>()->GetPosition(), m_camera->FindComponent<Kiwi::Transform>()->GetForward().Normalized(), maxDepth, rayHits ) )
	{
		if( rayHits.size() != 0 && rayHits[0]->HasTag( L"npc" ) )
		{
			return rayHits[0]->FindComponent<INPCController>();

		} else
		{
			return 0;
		}
	}

	return 0;

}

Voxel* CharacterController::_GetTargetVoxel( double maxDepth )
{

	Voxel* newVoxel = 0;
	double voxelSize = m_terrain->GetVoxelSize();
	Kiwi::Transform* cameraTrans = m_camera->FindComponent<Kiwi::Transform>();

	if( m_selectedNPC != 0 || m_visualTargetNPC != 0 )
	{
		return 0;
	}

	//get position of first voxel straight ahead from the player position
	for( double vPos = voxelSize; vPos <= maxDepth; vPos += voxelSize )
	{
		Kiwi::Vector3d newPos = cameraTrans->GetPosition() + (cameraTrans->GetForward() * vPos);

		newVoxel = m_terrain->FindSurfaceVoxelAtPosition( newPos );
		if( newVoxel != 0 )
		{
			break;
		}
	}

	return newVoxel;

}

void CharacterController::_Attack()
{

	if( m_currentEP >= 10.0 )
	{
		static int projNum = 0;
		projNum++;

		double radius = 0.075;

		/*get the position of the current target, if there is one*/
		Kiwi::Vector3d targetPos;
		if( m_visualTargetNPC != 0 )
		{
			Kiwi::Transform* targetTransform = m_visualTargetNPC->GetEntity()->FindComponent<Kiwi::Transform>();
			targetPos = targetTransform->GetPosition();

			if( m_visualTargetNPC != m_selectedNPC )
			{
				m_selectedNPC = m_visualTargetNPC;
			}

		} else
		{
			targetPos = m_transform->GetPosition() + (m_transform->GetForward() * 7.0);
		}

		Kiwi::Vector3d launchPos = m_transform->GetPosition() + (m_transform->GetForward() * m_characterScale.z) + Kiwi::Vector3d( 0.0, 0.2, 0.0 );

		Kiwi::Vector3d relPos = targetPos - launchPos;
		Kiwi::Vector3d relPosXZ = relPos;
		relPosXZ.y = 0.0;

		float y = relPos.y;
		float xz = relPosXZ.Magnitude();

		// calculate starting speeds for xz and y. Physics forumulae deltaX = v0 * t + 1/2 * a * t * t
		// where a is "-gravity" but only on the y plane, and a is 0 in xz plane.
		// so xz = v0xz * t => v0xz = xz / t
		// and y = v0y * t - 1/2 * gravity * t * t => v0y * t = y + 1/2 * gravity * t * t => v0y = y / t + 1/2 * gravity * t
		float time = 0.5;
		float v0y = (y / time) + (0.5 * std::abs( m_gravity.y ) * time);
		float v0xz = xz / time;

		Kiwi::Vector3d trajectory = m_transform->GetForward();//relPosXZ.Normalized();
		trajectory = trajectory * v0xz;
		trajectory.y = v0y;

		Kiwi::Entity* projectile = m_scene->CreateEntity( L"PProj" + Kiwi::ToWString( projNum ) );
		projectile->AttachComponent( Kiwi::Mesh::Primitive( Kiwi::Mesh::CUBE ) );
		projectile->AttachComponent( new Projectile( m_entity, radius, trajectory, radius * 4000 ) );
		projectile->FindComponent<Kiwi::Transform>()->SetPosition( launchPos );

		projectile->AddTag( L"projectile" );

		m_currentEP -= 10.0;
	}

}

void CharacterController::_Build()
{

	m_terrain->RemoveVoxelContainingPosition( m_targetVoxel->GetPosition() );

	m_targetVoxel = 0;

}

Kiwi::Vector3d CharacterController::_CalculateFinalPosition( Kiwi::Vector3d startPos, Kiwi::Vector3d startVelocity, double stepSize )
{

	Kiwi::Vector3d currentPosition = startPos;
	Kiwi::Vector3d currentVelocity = startVelocity;

	double ySign = (currentVelocity.y == 0.0) ? 0.0 : currentVelocity.y / std::abs( currentVelocity.y ); //direction of vertical movement, +1 or -1
	double xSign = (currentVelocity.x == 0.0) ? 0.0 : currentVelocity.x / std::abs( currentVelocity.x ); //direction of x movement, +1 or -1
	double zSign = (currentVelocity.z == 0.0) ? 0.0 : currentVelocity.z / std::abs( currentVelocity.z );

	double voxelSize = m_terrain->GetVoxelSize();
	double halfVoxel = voxelSize / 2.0;
	double width = m_transform->GetScale().x;
	double depth = m_transform->GetScale().z;
	m_clearance = (int)std::ceil( m_playerHeight / voxelSize );
	int stepVoxels = 1;

	unsigned int numPointsX = max( (unsigned int)std::ceil( depth / voxelSize ), 3 ); //number of points to check along the x axis
	unsigned int numPointsZ = max( (unsigned int)std::ceil( width / voxelSize ), 3 );

	m_onGround = false;
	if( currentVelocity.y < 0.0 )
	{
		for( unsigned int x = 0; x < numPointsZ; x++ )
		{
			for( unsigned int z = 0; z < numPointsX; z++ )
			{
				Kiwi::Vector3d pos = currentPosition + Kiwi::Vector3d( (width / 2.0) - ((double)x * (width / (double)(numPointsZ - 1))), -0.01, (depth / 2.0) - ((double)z * (depth / (double)(numPointsX - 1))) );
				Voxel* currentVoxel = m_terrain->FindSurfaceVoxelAtPosition( pos );
				if( currentVoxel != 0 )
				{
					m_onGround = true;
					currentVelocity.y = 0.0;
					currentPosition.y += (currentVoxel->GetPosition().y + halfVoxel) - currentPosition.y;
					break;
				}
				if( m_onGround ) break;
			}
		}
	}

	while( currentVelocity.x != 0 || currentVelocity.y != 0.0 || currentVelocity.z != 0.0 )
	{//loop until velocity is 0

		double xMove = min( stepSize, std::abs( currentVelocity.x ) ) * xSign;
		double yMove = min( stepSize, std::abs( currentVelocity.y ) ) * ySign;
		double zMove = min( stepSize, std::abs( currentVelocity.z ) ) * zSign;

		bool collisionLeft = false;
		bool collisionRight = false;
		bool collisionFront = false;
		bool collisionBack = false;

		double leftTranslation = 0.0; //translation from walking over voxels in the x direction
		double rightTranslation = 0.0;
		double frontTranslation = 0.0;
		double backTranslation = 0.0;

		if( !m_onGround )
		{
			stepVoxels = 0;
		}

		//translate by one step
		currentPosition.x += xMove;
		currentVelocity.x -= xMove;

		//check left side
		for( unsigned int leftVoxel = 0; leftVoxel < numPointsX && collisionLeft == false; leftVoxel++ )
		{
			//first check center of left side
			Kiwi::Vector3d leftSide = currentPosition + Kiwi::Vector3d( width / -2.0, -0.01, (depth / 2.0) - ((double)leftVoxel * (depth / (double)(numPointsX - 1))) );

			std::vector<Voxel*> voxelStack = m_terrain->FindVoxelsAbovePoint( leftSide, m_clearance + stepVoxels );
			for( unsigned int i = 0; i < voxelStack.size(); i++ )
			{
				if( voxelStack[i] == 0 || voxelStack[i]->GetPosition().x > leftSide.x ) continue;

				double vrs = voxelStack[i]->GetPosition().x + halfVoxel;

				if( leftSide.x < vrs && collisionLeft == false )
				{
					bool walkable = false;
					unsigned int step = i + 1;
					for( ; step <= stepVoxels && step < voxelStack.size(); step++ )
					{
						unsigned int verticalCount = 0;
						for( ; verticalCount != m_clearance && step + verticalCount < voxelStack.size(); verticalCount++ )
						{
							if( voxelStack[step + verticalCount] != 0 && voxelStack[step + verticalCount]->GetDefinition()->solidity != 0 )
							{
								break;
							}
						}
						if( verticalCount == m_clearance )
						{
							walkable = true;
							break;
						}
					}
					if( !walkable )
					{
						collisionLeft = true;
						currentVelocity.x = 0.0;
						currentPosition.x += (vrs - leftSide.x) + 0.001;
						leftTranslation = 0.0;

					} else
					{
						double verticalDif = (voxelStack[step - 1]->GetPosition().y + halfVoxel) - currentPosition.y;
						if( verticalDif > leftTranslation ) leftTranslation = verticalDif;
					}
					break;
				}
			}
		}

		//check right side
		for( unsigned int rightVoxel = 0; rightVoxel < numPointsX && collisionRight == false; rightVoxel++ )
		{
			//first check center of left side
			Kiwi::Vector3d rightSide = currentPosition + Kiwi::Vector3d( width / 2.0, -0.01, (depth / 2.0) - ((double)rightVoxel * (depth / (double)(numPointsX - 1))) );

			std::vector<Voxel*> voxelStack = m_terrain->FindVoxelsAbovePoint( rightSide, m_clearance + stepVoxels );
			for( unsigned int i = 0; i < voxelStack.size(); i++ )
			{
				if( voxelStack[i] == 0 || voxelStack[i]->GetPosition().x < rightSide.x ) continue;

				double vls = voxelStack[i]->GetPosition().x - halfVoxel;

				if( rightSide.x > vls && collisionRight == false )
				{
					bool walkable = false;
					unsigned int step = i + 1;
					for( ; step <= stepVoxels && step < voxelStack.size(); step++ )
					{
						unsigned int verticalCount = 0;
						for( ; verticalCount != m_clearance && step + verticalCount < voxelStack.size(); verticalCount++ )
						{
							if( voxelStack[step + verticalCount] != 0 && voxelStack[step + verticalCount]->GetDefinition()->solidity != 0 )
							{
								break;
							}
						}
						if( verticalCount == m_clearance )
						{
							walkable = true;
							break;
						}
					}
					if( !walkable )
					{
						collisionRight = true;
						currentVelocity.x = 0.0;
						currentPosition.x -= (rightSide.x - vls) + 0.001;
						rightTranslation = 0.0;

					} else
					{
						double verticalDif = (voxelStack[step - 1]->GetPosition().y + halfVoxel) - currentPosition.y;
						if( verticalDif > rightTranslation ) rightTranslation = verticalDif;
					}
					break;
				}
			}
		}

		currentVelocity.z -= zMove;
		currentPosition.z += zMove;

		//check front
		for( unsigned int front = 0; front < numPointsZ && collisionFront == false; front++ )
		{
			Kiwi::Vector3d frontSide = currentPosition + Kiwi::Vector3d( (width / 2.0) - ((double)front * (width / (double)(numPointsZ - 1))), -0.01, (depth / 2.0) );

			std::vector<Voxel*> voxelStack = m_terrain->FindVoxelsAbovePoint( frontSide, m_clearance + stepVoxels );
			for( unsigned int i = 0; i < voxelStack.size(); i++ )
			{
				if( voxelStack[i] == 0 || voxelStack[i]->GetPosition().z < frontSide.z ) continue;

				double vbs = voxelStack[i]->GetPosition().z - halfVoxel; //voxel back side

				if( frontSide.z > vbs && collisionFront == false )
				{
					bool walkable = false;
					unsigned int step = i + 1;
					for( ; step <= stepVoxels && step < voxelStack.size(); step++ )
					{
						unsigned int verticalCount = 0;
						for( ; verticalCount != m_clearance && step + verticalCount < voxelStack.size(); verticalCount++ )
						{
							if( voxelStack[step + verticalCount] != 0 && voxelStack[step + verticalCount]->GetDefinition()->solidity != 0 )
							{
								break;
							}
						}
						if( verticalCount == m_clearance )
						{
							walkable = true;
							break;
						}
					}
					if( !walkable )
					{
						collisionFront = true;
						currentVelocity.z = 0.0;
						currentPosition.z -= (frontSide.z - vbs) + 0.001;
						frontTranslation = 0.0;

					} else
					{
						double verticalDif = (voxelStack[step - 1]->GetPosition().y + halfVoxel) - currentPosition.y;
						if( verticalDif > frontTranslation ) frontTranslation = verticalDif;
					}
					break;
				}
			}
		}

		//check back
		for( unsigned int back = 0; back < numPointsZ && collisionBack == false; back++ )
		{
			Kiwi::Vector3d backSide = currentPosition + Kiwi::Vector3d( (width / 2.0) - ((double)back * (width / (double)(numPointsZ - 1))), -0.01, -(depth / 2.0) );

			std::vector<Voxel*> voxelStack = m_terrain->FindVoxelsAbovePoint( backSide, m_clearance + stepVoxels );
			for( unsigned int i = 0; i < voxelStack.size(); i++ )
			{
				if( voxelStack[i] == 0 || voxelStack[i]->GetPosition().z > backSide.z ) continue;

				double vfs = voxelStack[i]->GetPosition().z + halfVoxel; //voxel back side

				if( backSide.z < vfs && collisionBack == false )
				{
					bool walkable = false;
					unsigned int step = i + 1;
					for( ; step <= stepVoxels && step < voxelStack.size(); step++ )
					{
						unsigned int verticalCount = 0;
						for( ; verticalCount != m_clearance && step + verticalCount < voxelStack.size(); verticalCount++ )
						{
							if( voxelStack[step + verticalCount] != 0 && voxelStack[step + verticalCount]->GetDefinition()->solidity != 0 )
							{
								break;
							}
						}
						if( verticalCount == m_clearance )
						{
							walkable = true;
							break;
						}
					}
					if( !walkable )
					{
						collisionBack = true;
						currentVelocity.z = 0.0;
						currentPosition.z += (vfs - backSide.z) + 0.001;
						backTranslation = 0.0;

					} else
					{
						double verticalDif = (voxelStack[step - 1]->GetPosition().y + halfVoxel) - currentPosition.y;
						if( verticalDif > backTranslation ) backTranslation = verticalDif;
					}
					break;
				}
			}
		}

		if( m_onGround )
		{
			currentPosition.y += max( max( leftTranslation, rightTranslation ), max( frontTranslation, backTranslation ) );

		}

		if( currentVelocity.y != 0.0 )
		{
			currentPosition.y += yMove;
			currentVelocity.y -= yMove;

			//check for collision below
			for( unsigned int x = 0; x < numPointsZ; x++ )
			{
				for( unsigned int z = 0; z < numPointsX; z++ )
				{
					Kiwi::Vector3d pos = currentPosition + Kiwi::Vector3d( (width / 2.0) - ((double)x * (width / (double)(numPointsZ - 1))), -0.01, (depth / 2.0) - ((double)z * (depth / (double)(numPointsX - 1))) );
					Voxel* currentVoxel = m_terrain->FindSurfaceVoxelAtPosition( pos );
					if( currentVoxel != 0 )
					{
						m_onGround = true;
						currentVelocity.y = 0.0;
						currentPosition.y += (currentVoxel->GetPosition().y + halfVoxel) - currentPosition.y;
						break;
					}
				}
				if( m_onGround ) break;
			}

			//check for collision above
			bool collisionTop = false;
			for( unsigned int x = 0; x < numPointsZ; x++ )
			{
				for( unsigned int z = 0; z < numPointsX; z++ )
				{
					Kiwi::Vector3d pos = currentPosition + Kiwi::Vector3d( (width / 2.0) - ((double)x * (width / (double)(numPointsZ - 1))), 0.0, (depth / 2.0) - ((double)z * (depth / (double)(numPointsX - 1))) );
					Voxel* currentVoxel = m_terrain->FindSurfaceVoxelAbovePosition( pos );
					if( currentVoxel != 0 )
					{
						double voxelBottom = currentVoxel->GetPosition().y - halfVoxel;

						if( pos.y + m_playerHeight > voxelBottom )
						{
							collisionTop = true;
							currentVelocity.y = 0.0;
							currentPosition.y -= ((pos.y + m_playerHeight) - voxelBottom) + 0.001;
							break;
						}
					}
				}
				if( collisionTop ) break;
			}
		}
	}

	return currentPosition;

}

void CharacterController::AddCombatant( INPCController* combatant )
{

	if( m_selectedNPC == 0 )
	{
		m_selectedNPC = combatant;
	}

	m_combatants.insert( combatant );

}

void CharacterController::RemoveCombatant( INPCController* combatant )
{

	if( m_combatants.size() > 0 )
	{
		if( m_selectedNPC == combatant )
		{
			m_selectedNPC = 0;
		}
		
		m_combatants.erase( combatant );
	}

}