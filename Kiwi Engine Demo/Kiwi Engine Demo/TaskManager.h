#ifndef _TASKMANAGER_H_
#define _TASKMANAGER_H_

#include "Task.h"

#include <Core\Component.h>
#include <Core\Scene.h>

#include <Core\Events\IGlobalEventListener.h>

#include <vector>
#include <unordered_map>

class CharacterController;
class OverlayController;

namespace Kiwi
{
	class Entity;
}

class TaskManager:
	public Kiwi::Component,
	public Kiwi::IGlobalEventListener
{
protected:

	CharacterController* m_cc;
	OverlayController* m_overlay;

	Kiwi::Scene* m_scene;

	std::unordered_map<int, Task*> m_activeTasks;
	std::unordered_map<int, Task*> m_completeTasks;
	std::unordered_map<int, Task*> m_returnedTasks; //once a task has been completed and turned in for reward it is placed here

	std::unordered_map<int, std::wstring> m_objectiveDescriptors;

protected:

	void _OnAttached();

	void _OnFixedUpdate();

	void _ProcessGlobalEvent( Kiwi::GlobalEventPtr e );

public:

	TaskManager();

	void AddTask( Task* task );

	Task* CreateRandomTask( std::wstring taskSource );

	Task* FindTask( int globalTaskID );

};

#endif