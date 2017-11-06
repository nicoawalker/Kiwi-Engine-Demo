#include "GameApplication.h"
#include "DefaultShader.h"
#include "Default2DShader.h"
#include "MainScene.h"
#include "VoxelTerrain.h"
#include "VillagerController.h"
#include "Pathfinder.h"

#include <Graphics\UI\UserInterface.h>
#include <Graphics\UI\UISprite.h>
#include <Graphics\UI\UIButton.h>
#include <Graphics\UI\UIScrollBar.h>
#include <Graphics\UI\UITextBox.h>

#include <Utilities\PerlinNoiseGenerator.h>

#include <Windows.h>
#include <random>

#pragma comment(lib, "Kiwi-Engine.lib")

#include <KiwiCore.h>
#include <KiwiGraphics.h>

#include <Graphics\DirectX.h>

using namespace DirectX;

GameApplication::~GameApplication()
{

	m_terrain = 0;

}

void GameApplication::Launch()
{

	try
	{
		m_mouseRestricted = true;

		Kiwi::RenderWindow* mainWindow = m_graphicsManager->CreateRenderWindow( L"Game Window", Kiwi::Vector2( 0, 0 ), Kiwi::Vector2( 1600.0, 900.0 ), false );

		mainWindow->CenterWindow( GetDesktopWindow() );

		mainWindow->Show( SW_SHOW );

		mainWindow->AddWindowEventListener( this );

		m_engine->Initialize( L"Data/Logs/debug.txt", mainWindow );

		Kiwi::Renderer* renderer = m_graphicsManager->CreateRenderer( L"Main Renderer", mainWindow );

	    m_engine->SetMouseSensitivity( Kiwi::Vector2( 0.04f, 0.04f ) );

		MainScene* scene = new MainScene( m_engine, renderer );
		m_sceneManager->AddScene( scene );

		scene->GetRenderWindow()->AddInputListener( this );
		scene->GetRenderWindow()->RestrictMouse( true );

		m_activeScene = scene;

		renderer->SetPrimitiveTopology( Kiwi::TRIANGLE_LIST );

		Kiwi::RenderTarget* backBuffer = renderer->GetBackBuffer();
		backBuffer->SetClearColor( Kiwi::Color( 0.366f, 0.609f, 0.896f, 1.0f ) );

		Kiwi::D3DRasterStateDesc wireframeDesc;
		wireframeDesc.desc.CullMode = D3D11_CULL_NONE;
		wireframeDesc.desc.FillMode = D3D11_FILL_WIREFRAME;
		renderer->CreateRasterState( L"Wireframe", wireframeDesc );
		Kiwi::D3DRasterStateDesc cDesc;
		cDesc.desc.FillMode = D3D11_FILL_SOLID;
		cDesc.desc.CullMode = D3D11_CULL_BACK;
		cDesc.desc.FrontCounterClockwise = true;
		renderer->CreateRasterState( L"Cull CW", cDesc );
		cDesc.desc.FrontCounterClockwise = false;
		renderer->CreateRasterState( L"Cull CCW", cDesc );

		scene->Load();

	} catch( ... )
	{
		throw;
	}

	//enter the main game loop, wont return until the engine is stopped
	m_engine->Start();

}

void GameApplication::_OnKeyPress( Kiwi::KEY key )
{

	if( key == Kiwi::KEY::Escape )
	{
		m_mouseRestricted = !m_mouseRestricted;
		m_activeScene->GetRenderWindow()->RestrictMouse( m_mouseRestricted );
	}

}

void GameApplication::OnShutdown()
{

	m_engine->Stop();

}

void GameApplication::OnWindowEvent( const Kiwi::WindowEvent& evt )
{

	switch( evt.GetWindowMessage() )
	{
		case WM_CLOSE:
			{
				this->Shutdown();
				break;
			}
		default:break;
	}

}

void GameApplication::OnUpdate()
{


}

void GameApplication::OnFixedUpdate()
{

}





//main Windows entry point
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
{

	GameApplication gameApp;

	srand( time( NULL ) );

	try
	{
		gameApp.Launch();
	}catch(Kiwi::Exception& e)
	{
		MessageBox(NULL, e.GetError().c_str(), e.GetSource().c_str(), MB_OK | MB_ICONEXCLAMATION);
	}

	return 0;

}