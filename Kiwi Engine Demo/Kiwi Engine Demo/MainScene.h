#ifndef _MAINSCENE_H_
#define _MAINSCENE_H_

#include <Core\Scene.h>

class MainScene :
	public Kiwi::Scene
{
protected:

	bool m_loading;

protected:

	void _OnUpdate();
	void _OnFixedUpdate();

	void _Initialize();

public:

	MainScene( Kiwi::EngineRoot* engine, Kiwi::Renderer* renderer );
	~MainScene();

	void Load();

};

#endif