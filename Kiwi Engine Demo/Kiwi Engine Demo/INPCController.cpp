#include "INPCController.h"
#include "Task.h"
#include "TaskManager.h"
#include "CharacterController.h"

#include "Events\TaskEvent.h"

#include <Core\Scene.h>

INPCController::INPCController( std::wstring npcType, std::wstring npcName ) :
	Kiwi::Component( L"NPCController" )
{
	m_npcType = npcType;
	m_npcName = npcName;
	m_npcID = 0;

	m_charController = 0;

	m_isTargeted = false;
	m_isHostile = false;
	m_hasTask = true;

}

INPCController::~INPCController()
{

}

void INPCController::_OnAttached()
{

	Kiwi::Scene* scene = m_entity->GetScene();
	assert( scene != 0 );

	Kiwi::Entity* player = scene->FindEntityWithName( L"Player" );
	assert( player != 0 );

	m_charController = player->FindComponent<CharacterController>();
	assert( m_charController != 0 );

	if( m_charController == 0 )
	{
		MessageBox( NULL, L"A", L"A", MB_OK );
	}

	m_entity->AddTag( L"npc" );

}

void INPCController::Target()
{

	m_isTargeted = true;

	//if( m_hasTask )
	//{
	//	Kiwi::Scene* scene = m_entity->GetScene();
	//	assert( scene != 0 );

	//	Kiwi::Entity* player = scene->FindEntityWithName( L"Player" );
	//	if( player != 0 )
	//	{
	//		TaskManager* tm = player->FindComponent<TaskManager>();
	//		if( tm != 0 )
	//		{
	//			//attempt to return an outstanding task
	//			if( this->_ReturnActiveTask() )
	//			{//a task was returned

	//			}
	//			Task* task = tm->CreateRandomTask( m_entity );
	//			if( task != 0 )
	//			{
	//				tm->AddTask( task );
	//				m_assignedTasks.insert( std::make_pair( task->GetGlobalID(), task ) );
	//			}
	//		}
	//	}
	//}

	this->_OnTargeted();

}

void INPCController::Untarget()
{

	m_isTargeted = false;

	this->_OnUntargeted();

}