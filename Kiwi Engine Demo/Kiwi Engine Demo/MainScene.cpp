#include "MainScene.h"
#include "Default2DShader.h"
#include "DefaultShader.h"
#include "TerrainShader.h"
#include "TextShader.h"
#include "VoxelTerrain.h"
#include "PlayerAvatar.h"
#include "VillagerController.h"
#include "Pathfinder.h"
#include "CharacterController.h"
#include "EnemyController.h"

#include "Overlay/OverlayController.h"

#include <Core\SceneLoader.h>

#include <Graphics\Mesh.h>
#include <Graphics\Font.h>
#include <Graphics\UI\UIScrollBar.h>
#include <Graphics\UI\UISprite.h>
#include <Graphics\UI\UIButton.h>

MainScene::MainScene( Kiwi::EngineRoot* engine, Kiwi::Renderer* renderer ):
	Kiwi::Scene( engine, L"Main Scene", renderer )
{

	m_diffuseDirection = Kiwi::Vector4( 1.0f, -1.0f, 1.0f, 0.0f );
	m_ambientLight = Kiwi::Vector4( 0.7f, 0.7f, 0.7f, 1.0f );

	m_loading = false;

}

MainScene::~MainScene()
{
}

void MainScene::_OnUpdate()
{
	
	if( m_isActive == false )
	{
		if( m_sceneLoader && m_sceneLoader->IsFinished() && m_loading )
		{
			m_loading = false;
			this->_Initialize();
		}
	}

}

void MainScene::_OnFixedUpdate()
{

	if( m_isActive )
	{
		if( m_terrain )
		{
			VoxelTerrain* vTerrain = dynamic_cast<VoxelTerrain*>(m_terrain);
			vTerrain->FixedUpdate();
		}
	}

}

void MainScene::_Initialize()
{

	m_physicsSystem->SetGravity( Kiwi::Vector3d( 0.0, -9.8, 0.0 ) );

	VoxelTerrain* terrain = new VoxelTerrain( this, 32, 1 );

	this->SetTerrain( terrain );

	//terrain->SetWorldSeed( 66184984 );

	terrain->SetCenterPosition( Kiwi::Vector3d( 0.0, 0.0, 0.0 ) );

	Kiwi::Entity* overlay = this->CreateEntity( L"Overlay" );
	Kiwi::Entity* player = this->CreateEntity( L"Player" );
	this->SetPlayerEntity( player );

	player->AttachComponent( new CharacterController( overlay ) );
	overlay->AttachComponent( new OverlayController( player ) );

	terrain->SetTarget( player );

	Kiwi::Entity* villager1 = this->CreateEntity( L"Enemy1" );
	villager1->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( 30.0, 0.0, 20.5 ) );
	villager1->AttachComponent( new EnemyController( L"Enemy1", overlay ) );
	villager1->AttachComponent( new Pathfinder() );

	Kiwi::Entity* vil1 = this->CreateEntity( L"Villager1" );
	vil1->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( 15.0, 0.0, 10.0 ) );
	vil1->AttachComponent( new VillagerController( L"Villager1", L"NPC1" ) );

	Kiwi::Entity* vil2 = this->CreateEntity( L"Villager2" );
	vil2->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( 16.5, 0.0, 10.0 ) );
	vil2->AttachComponent( new VillagerController( L"Villager2", L"NPC2" ) );

	m_isActive = true;

}

void MainScene::Load()
{

	DefaultShader* shader = new DefaultShader( L"default", m_renderer );
	Default2DShader* shader2D = new Default2DShader( m_renderer );
	TerrainShader* terrainShader = new TerrainShader( L"TerrainShader", m_renderer );
	TextShader* textShader = new TextShader( m_renderer );

	this->AddShader( shader );
	this->AddShader( shader2D );
	this->AddShader( terrainShader );
	this->AddShader( textShader );

	m_sceneLoader->LoadFontFromFile( L"Data/Fonts/lato_20pt_outline.fnt" );
	m_sceneLoader->LoadFontFromFile( L"Data/Fonts/lato_20pt.fnt" );
	m_sceneLoader->LoadFontFromFile( L"Data/Fonts/sourcecodepro_20pt.fnt" );
	m_sceneLoader->LoadFontFromFile( L"Data/Fonts/sourcecodepro_20pt_outline.fnt" );
	m_sceneLoader->LoadFontFromFile( L"Data/Fonts/flaemische_24pt.fnt" );
	m_sceneLoader->LoadFontFromFile( L"Data/Fonts/overlock_14pt.fnt" );
	m_sceneLoader->LoadFontFromFile( L"Data/Fonts/overlock_24pt.fnt" );

	m_sceneLoader->LoadTextureFromFile( L"dirt", L"Data/Images/terrain_dirt01.dds", false );
	m_sceneLoader->LoadTextureFromFile( L"ConsoleTextField", L"Data/Images/consoletextfield.dds", false );
	m_sceneLoader->LoadTextureFromFile( L"ConsoleScrollArrow", L"Data/Images/consolescrollbararrow.dds", false );
	m_sceneLoader->LoadTextureFromFile( L"SelectorRing", L"Data/Images/SelectorRing.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"SelectionPanel", L"Data/Images/SelectionPanel.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"ActiveTaskPanel", L"Data/Images/ActiveTaskPanel.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"PlayerStatusPanel", L"Data/Images/PlayerStatusPanel.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"EPBar", L"Data/Images/EPBar.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"HPBar", L"Data/Images/HPBar.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"OverlayWindowTitle", L"Data/Images/OverlayWindowTitle.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"OverlayWindowBackground", L"Data/Images/OverlayWindowBackground.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"DialogueButtonGreen", L"Data/Images/DialogueButtonGreen.dds", true );
	m_sceneLoader->LoadTextureFromFile( L"DialogueButtonRed", L"Data/Images/DialogueButtonRed.dds", true );

	m_sceneLoader->Start();

	m_loading = true;

}