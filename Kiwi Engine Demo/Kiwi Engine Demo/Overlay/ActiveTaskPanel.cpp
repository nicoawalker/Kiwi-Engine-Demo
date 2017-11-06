#include "ActiveTaskPanel.h"
#include "ActiveTaskItem.h"

#include "../Task.h"
#include "../Events/TaskEvent.h"

#include <Core\Scene.h>
#include <Core\Entity.h>
#include <Core\Assert.h>

ActiveTaskPanel::ActiveTaskPanel( Kiwi::Entity* overlay, const Kiwi::Vector3d& dimensions ) :
	OverlayPanel( overlay, L"ActiveTaskPanel", dimensions )
{

	Kiwi::Scene* scene = m_panelEntity->GetScene();
	assert( scene != 0 );

	//register the panel to receive only task events
	this->SetRegisteredEventIDs( { TASK_EVENT } );
	scene->AddGlobalEventListener( this );

	m_panelBackground = new Kiwi::UISprite( L"ActiveTaskPanel/Background", L"ActiveTaskPanel" );
	m_panelEntity->AttachComponent( m_panelBackground );
	m_panelEntity->FindComponent<Kiwi::Transform>()->SetScale( m_panelDimensions );

	m_maxTasks = 4;

	m_taskItemPadding = 5.0;

	m_newTaskPosition.Set( 0.0, m_panelDimensions.y / 2.0 );

	Kiwi::Entity* headerEntity = scene->CreateEntity( L"ATPHeader" );
	m_headerText = new Kiwi::Text( L"ATPHeaderText", L"Lato_20pt_Outline", Kiwi::Vector2d( m_panelDimensions.x, 25.0 ) );
	m_headerText->SetAlignment( Kiwi::Font::ALIGN_LEFT );
	m_headerText->SetShader( L"Default2DShader" );
	m_headerText->SetText( L"Active Tasks" );
	headerEntity->AttachComponent( m_headerText );
	headerEntity->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( (m_panelDimensions.x / -2.0) + 10.0, m_panelDimensions.y / 2.0, m_position.z + 1.0 ) );
	m_panelEntity->AttachChild( headerEntity );

}

ActiveTaskPanel::~ActiveTaskPanel()
{

	Kiwi::Scene* scene = m_panelEntity->GetScene();
	scene->RemoveGlobalEventListener( this );

}

void ActiveTaskPanel::_ProcessGlobalEvent( Kiwi::GlobalEventPtr e )
{

	if( e != 0 )
	{
		TaskEvent* taskEvent = dynamic_cast<TaskEvent*>(e.get());
		if( taskEvent != 0 )
		{
			switch( taskEvent->GetEventType() )
			{
				case TASK_RETURNED:
					{//a task has just been completed and turned in, remove it from the list
						auto taskItr = m_activeTasks.find( taskEvent->GetTask()->GetGlobalID() );
						if( taskItr != m_activeTasks.end() )
						{
							taskItr->second->GetEntity()->Shutdown();
							m_activeTasks.erase( taskItr );
						}
						break;
					}
				case TASK_ACCEPTED:
					{//a new task has just been accepted and created, add it to the list
						if( m_activeTasks.size() < m_maxTasks )
						{
							this->_AddNewTask( taskEvent->GetTask() );
						}
						break;
					}
				default: return;
			}
		}
	}

}

void ActiveTaskPanel::_AddNewTask( Task* task )
{

	Kiwi::Scene* scene = m_panelEntity->GetScene();

	ActiveTaskItem* newItem = new ActiveTaskItem( task->GetGlobalID(), this, task, Kiwi::Vector2d( m_panelDimensions.x + 500.0, 80.0 ) );

	m_newTaskPosition.y -= newItem->GetDimensions().y / 2.0;

	Kiwi::Entity* itemEntity = scene->CreateEntity( newItem->GetName() + L"Entity" );
	if( itemEntity != 0 )
	{
		itemEntity->SetEntityType( Kiwi::Entity::ENTITY_2D );
		itemEntity->AttachComponent( newItem );
		m_activeTasks.insert( std::make_pair( task->GetGlobalID(), newItem ) );
		itemEntity->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( m_newTaskPosition.x, m_newTaskPosition.y, 10.0 ) );
		m_newTaskPosition.y -= (newItem->GetDimensions().y / 2.0) + m_taskItemPadding;
		m_panelEntity->AttachChild( itemEntity );
	}

}

void ActiveTaskPanel::_OnFixedUpdate()
{

	while( this->PullNextEvent() );

}