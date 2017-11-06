#include "Selector.h"

#include "../Voxel.h"
#include "../CharacterController.h"
#include "../VoxelTerrain.h"
#include "../VillagerController.h"
#include "../EnemyController.h"

#include <limits.h>

#include <KiwiCore.h>
#include <KiwiGraphics.h>

Selector::Selector( CharacterController* controller, double maxDistance )
{

	m_circleMesh = 0;
	m_cubeMesh = 0;
	m_controller = controller;
	m_currentVoxel = 0;
	m_selectedNPC = 0;
	m_targetedNPC = 0;
	m_terrain = 0;
	//m_crosshairEntity = 0;
	m_crosshairMesh = 0;
	m_scene = 0;
	m_voxelSelector = 0;
	m_crosshair = 0;

	m_crosshairRadius = 9.0;

	m_circleGrowTime = 0.25;
	m_circleShrinkTime = 0.2;
	m_circleMinRadius = 5.0;
	m_circleMaxRadius = 50.0;
	m_crosshairEnabled = false;
	m_maxTargetDistance = 15.0;

	m_maxDistance = maxDistance;

}

Selector::~Selector()
{

}

void Selector::_OnAttached()
{

	m_scene = m_entity->GetScene();

	Kiwi::RenderWindow* renderWindow = m_scene->GetRenderWindow();
	renderWindow->AddInputListener( this );

	m_terrain = m_entity->GetScene()->GetTerrain<VoxelTerrain*>();

	Kiwi::Camera* camera = m_controller->GetCamera();
	Kiwi::Vector3d cameraForward = camera->FindComponent<Kiwi::Transform>()->GetForward();
	Kiwi::Vector3d cameraPos = camera->FindComponent<Kiwi::Transform>()->GetPosition();

	m_crosshair = m_scene->CreateEntity( L"Crosshair" );
	if( m_crosshair != 0 )
	{
		m_crosshair->FindComponent<Kiwi::Transform>()->SetPosition( cameraPos + (cameraForward * m_maxDistance) );
		m_crosshair->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( m_circleMinRadius, m_circleMinRadius, 1.0 ) );
		m_entity->AttachChild( m_crosshair );
	}

	m_voxelSelector = m_scene->CreateEntity( L"VoxelSelector" );
	if( m_voxelSelector != 0 )
	{
		m_voxelSelector->FindComponent<Kiwi::Transform>()->SetPosition( cameraPos + (cameraForward * m_maxDistance) );
		m_voxelSelector->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( m_circleMinRadius, m_circleMinRadius, 1.0 ) );
		m_entity->AttachChild( m_voxelSelector );
	}

	this->_CreateCrosshair( 30 );
	this->_CreateVoxelSelector( 0.5 );

	m_voxelSelector->SetActive( false );

}

void Selector::_OnFixedUpdate()
{

	if( m_entity != 0 && m_controller != 0 )
	{

		INPCController* selectedNPC = (m_controller->GetSelectedNPC() != 0) ? m_controller->GetSelectedNPC() : m_controller->GetVisualTargetNPC();

		if( selectedNPC != 0 )
		{
			m_voxelSelector->SetActive( false );
			m_crosshair->SetActive( true );

			if( selectedNPC->HasTag( L"hostile" ) )
			{
				m_circleMesh->GetSubmesh( 0 )->material.SetColor( L"Diffuse", Kiwi::Color( 1.0, 0.0, 0.0, 0.3 ) );

			} else
			{
				m_circleMesh->GetSubmesh( 0 )->material.SetColor( L"Diffuse", Kiwi::Color( 0.0, 1.0, 0.1, 0.3 ) );
			}

		} 
		
		if( m_controller->GetTargetVoxel() != 0 )
		{
			m_voxelSelector->SetActive( true );
			m_crosshair->SetActive( false );

			m_voxelSelector->FindComponent<Kiwi::Transform>()->SetPosition( m_controller->GetTargetVoxel()->GetPosition() );

		} else
		{
			m_voxelSelector->SetActive( false );
		}
	}

}

void Selector::_CreateCrosshair( unsigned int pointCount )
{

	if( m_circleMesh == 0 )
	{
		m_circleMesh = new Kiwi::Mesh( L"SelectorCircle" );
	}

	std::vector<Kiwi::Vector3d> vertices;
	std::vector<Kiwi::Vector3d> normals;

	double anglePerVertex = 360.0 / (double)pointCount;
	double currentAngle = 0.0;
	for( unsigned int i = 0; i < pointCount + 1; i++ )
	{
		vertices.push_back( Kiwi::Vector3d( std::cos( Kiwi::ToRadians(currentAngle) ), std::sin( Kiwi::ToRadians( currentAngle ) ), 0.0 ) );
		normals.push_back( Kiwi::Vector3d( 0.0, 0.0, -1.0 ) );
		currentAngle += anglePerVertex;
	}

	m_circleMesh->ClearAll();
	m_circleMesh->SetVertices( vertices );
	m_circleMesh->SetNormals( normals );
	m_circleMesh->SetPrimitiveTopology( Kiwi::LINE_STRIP );
	m_circleMesh->CreateSubmesh( Kiwi::Material( Kiwi::Color( 1.0, 1.0, 1.0, 0.3 ) ), 0, vertices.size() - 1 );
	m_circleMesh->BuildMesh();
	m_circleMesh->SetShader( L"Default2DShader" );

	m_crosshair->AttachComponent( m_circleMesh );
	m_crosshair->FindComponent<Kiwi::Transform>()->SetScale( m_crosshairRadius );

}

void Selector::_CreateVoxelSelector( double radius )
{

	if( m_cubeMesh == 0 )
	{
		m_cubeMesh = new Kiwi::Mesh( L"SelectorCube" );
	}

	double voxelSize = m_terrain->GetVoxelSize();

	m_cubeMesh->ClearAll();
	m_cubeMesh->SetVertices( { Kiwi::Vector3d( -radius, radius, -radius ), //front
							   Kiwi::Vector3d( radius, radius, -radius ),
							   Kiwi::Vector3d( radius, radius, -radius ),
							   Kiwi::Vector3d( radius, -radius, -radius ),
							   Kiwi::Vector3d( radius, -radius, -radius ),
							   Kiwi::Vector3d( -radius, -radius, -radius ),
							   Kiwi::Vector3d( -radius, -radius, -radius ),
							   Kiwi::Vector3d( -radius, radius, -radius ),

							   Kiwi::Vector3d( -radius, radius, radius ), //back
							   Kiwi::Vector3d( -radius, -radius, radius ),
							   Kiwi::Vector3d( -radius, -radius, radius ),
							   Kiwi::Vector3d( radius, -radius, radius ),
							   Kiwi::Vector3d( radius, -radius, radius ),
							   Kiwi::Vector3d( radius, radius, radius ),
							   Kiwi::Vector3d( radius, radius, radius ),
							   Kiwi::Vector3d( -radius, radius, radius ),
	
							   Kiwi::Vector3d( -radius, radius, radius ),
							   Kiwi::Vector3d( -radius, radius, -radius ),
							   Kiwi::Vector3d( -radius, radius, -radius ),
							   Kiwi::Vector3d( -radius, -radius, -radius ),
							   Kiwi::Vector3d( -radius, -radius, -radius ),
							   Kiwi::Vector3d( -radius, -radius, radius ),
							   Kiwi::Vector3d( -radius, -radius, radius ),
							   Kiwi::Vector3d( -radius, radius, radius ), 
	
							   Kiwi::Vector3d( radius, radius, -radius ),
							   Kiwi::Vector3d( radius, radius, radius ),
							   Kiwi::Vector3d( radius, radius, radius ),
							   Kiwi::Vector3d( radius, -radius, radius ),
							   Kiwi::Vector3d( radius, -radius, radius ),
							   Kiwi::Vector3d( radius, -radius, -radius ),
							   Kiwi::Vector3d( radius, -radius, -radius ),
							   Kiwi::Vector3d( radius, radius, -radius ) } );

	m_cubeMesh->SetPrimitiveTopology( Kiwi::LINE_LIST );
	m_cubeMesh->BuildMesh();

	m_voxelSelector->AttachComponent( m_cubeMesh );
	m_voxelSelector->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( voxelSize + 0.002, voxelSize + 0.002, voxelSize + 0.002 ) );

}