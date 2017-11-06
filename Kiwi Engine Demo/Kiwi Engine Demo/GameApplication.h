#ifndef _GAMEAPPLICATION_H_
#define _GAMEAPPLICATION_H_

#define WIN32_LEAN_AND_MEAN

#include <KiwiCore.h>
#include <KiwiGraphics.h>

class House;
class VoxelTerrain;

class GameApplication : 
	public Kiwi::IEngineApp,
	public Kiwi::IKeyboardEventListener
{
protected:

	Kiwi::Scene* m_activeScene;

	VoxelTerrain* m_terrain;

	bool m_mouseRestricted;

protected:

	void OnUpdate();
	void OnFixedUpdate();
	void OnShutdown();

	void _OnKeyPress( Kiwi::KEY key );

public:

	GameApplication(){}
	~GameApplication();

	void Launch();

	void OnWindowEvent( const Kiwi::WindowEvent& evt );

};

#endif