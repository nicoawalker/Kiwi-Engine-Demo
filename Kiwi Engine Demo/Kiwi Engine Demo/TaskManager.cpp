#include "TaskManager.h"
#include "CharacterController.h"
#include "TaskKillObjective.h"

#include "Events\TaskEvent.h"

#include "Overlay\OverlayController.h"

#include <Core\Assert.h>
#include <Core\Entity.h>

#include <random>

TaskManager::TaskManager() :
	Kiwi::Component( L"TaskManager" )
{

	m_cc = 0;
	m_scene = 0;
	m_overlay = 0;

}

void TaskManager::_OnAttached()
{

	m_cc = m_entity->FindComponent<CharacterController>();
	assert( m_cc != 0 );

	m_scene = m_entity->GetScene();

	m_scene->AddGlobalEventListener( this );

}

void TaskManager::_OnFixedUpdate()
{

	while( this->PullNextEvent() );

	if( m_overlay == 0 )
	{
		Kiwi::Entity* overlay = m_scene->FindEntityWithName( L"Overlay" );
		if( overlay )
		{
			m_overlay = overlay->FindComponent<OverlayController>();
		}
	}

	for( auto itr = m_activeTasks.begin(); itr != m_activeTasks.end(); )
	{
		itr->second->Update();
		if( itr->second->IsCompleted() )
		{//move completed tasks out of the active tasks list
			m_scene->BroadcastGlobalEvent( std::make_shared<TaskEvent>( itr->second, TASK_COMPLETED ) );
			m_completeTasks.insert( std::make_pair( itr->second->GetGlobalID(), itr->second ) );
			itr = m_activeTasks.erase( itr );
			continue;
		}

		itr++;
	}

	for( auto itr = m_completeTasks.begin(); itr != m_completeTasks.end(); )
	{
		itr->second->Update();
		if( itr->second->IsReturned() )
		{//move returned tasks out of the completed tasks list
			m_scene->BroadcastGlobalEvent( std::make_shared<TaskEvent>( itr->second, TASK_RETURNED ) );
			m_returnedTasks.insert( std::make_pair( itr->second->GetGlobalID(), itr->second ) );
			itr = m_completeTasks.erase( itr );
			continue;
		}

		itr++;
	}

}

void TaskManager::_ProcessGlobalEvent( Kiwi::GlobalEventPtr e )
{

	if( e != 0 )
	{
		TaskEvent* taskEvent = dynamic_cast<TaskEvent*>(e.get());
		if( taskEvent != 0 )
		{
			switch( taskEvent->GetEventType() )
			{
				case TASK_ACCEPTED:
					{//a new task has just been accepted and created, add it
						this->AddTask( taskEvent->GetTask() );
						break;
					}
				default: return;
			}
		}
	}

}

void TaskManager::AddTask( Task* task )
{

	if( task != 0 )
	{
		Kiwi::Scene* scene = m_entity->GetScene();
		assert( scene != 0 );

		m_activeTasks.insert( std::make_pair( task->GetGlobalID(), task ) );
	}

}

Task* TaskManager::CreateRandomTask( std::wstring taskSource )
{

	Kiwi::Scene* scene = m_entity->GetScene();
	assert( scene != 0 );

	Task* newTask = new Task( 1, scene, taskSource );

	//generate a random number of objectives for this task
	std::random_device rd;
	std::mt19937 e2( rd() );
	std::uniform_real_distribution<> dist( 1, 3 );

	int objectiveCount = (int)dist( e2 );

	for( int i = 0; i < objectiveCount; i++ )
	{
		//generate a random objective
		//int objectiveID = dist( e2 );
		int objectiveID = 1;

		switch( objectiveID )
		{
			case TASK_KILL_OBJECTIVE:
				{
					std::uniform_real_distribution<> killCountDist( 3, 15 );
					newTask->AddObjective( new TaskKillObjective( 1, L"Ranger", 1 ) );
					newTask->SetName( taskSource + L"'s Task: " );
					break;
				}
			default:
				{
					SAFE_DELETE( newTask );
					return 0;
				}
		}
	}

	if( m_overlay != 0 )
	{
		//MessageBox( NULL, L"A", L"A", MB_OK );
		m_overlay->CreateTaskDialogue( newTask );
	}

	return newTask;

}

Task* TaskManager::FindTask( int globalTaskID )
{

	auto taskItr = m_activeTasks.find( globalTaskID );
	if( taskItr != m_activeTasks.end() )
	{
		return taskItr->second;
	}

	taskItr = m_completeTasks.find( globalTaskID );
	if( taskItr != m_completeTasks.end() )
	{
		return taskItr->second;
	}

	taskItr = m_returnedTasks.find( globalTaskID );
	if( taskItr != m_returnedTasks.end() )
	{
		return taskItr->second;
	}

	return 0;

}