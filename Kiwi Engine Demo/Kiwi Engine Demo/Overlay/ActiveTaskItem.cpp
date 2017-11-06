#include "ActiveTaskItem.h"
#include "ActiveTaskPanel.h"

#include "../Task.h"

#include <Core\Assert.h>
#include <Core\Entity.h>
#include <Core\Scene.h>

ActiveTaskItem::ActiveTaskItem( int itemID, ActiveTaskPanel* panel, Task* task, const Kiwi::Vector2d& dimensions ):
	Kiwi::Component(L"TaskItem")
{

	m_panel = panel;
	m_task = task;
	m_itemID = itemID;
	m_dimensions = dimensions;
	m_taskTitleText = 0;
	m_taskObjectiveText = 0;

	assert( m_panel != 0 && m_task != 0 );

	m_task->Reserve();

}

ActiveTaskItem::~ActiveTaskItem()
{

	m_task->Free();

}

void ActiveTaskItem::_OnAttached()
{

	Kiwi::Scene* scene = m_entity->GetScene();
	assert( scene != 0 );

	Kiwi::Entity* titleEntity = scene->CreateEntity( L"TaskItem" + Kiwi::ToWString( m_itemID ) + L"/title" );
	if( titleEntity != 0 )
	{
		m_taskTitleText = new Kiwi::Text( L"TitleText", L"Lato_20pt", m_dimensions );
		m_taskTitleText->SetAlignment( Kiwi::Font::ALIGN_CENTRE );
		m_taskTitleText->SetShader( L"Default2DShader" );
		m_taskTitleText->SetText( m_task->GetName() );
		titleEntity->AttachComponent( m_taskTitleText );
	}
	m_entity->AttachChild( titleEntity );

	Kiwi::Entity* objectiveEntity = scene->CreateEntity( L"TaskItem" + Kiwi::ToWString( m_itemID ) + L"/objective" );
	if( objectiveEntity != 0 )
	{
		m_taskObjectiveText = new Kiwi::Text( L"ObjectiveText", L"Lato_20pt", m_dimensions );
		m_taskObjectiveText->SetAlignment( Kiwi::Font::ALIGN_CENTRE );
		m_taskObjectiveText->SetShader( L"Default2DShader" );

		std::wstring objectives;
		for( auto itr = m_task->GetObjectives().begin(); itr != m_task->GetObjectives().end(); itr++ )
		{
			objectives += (*itr)->GetDescriptor() + L" (" + Kiwi::ToWString( (*itr)->GetCompletionCount() ) + L"/" + Kiwi::ToWString( (*itr)->GetTotalCount() ) + L")\n";
		}
		m_taskObjectiveText->SetText( objectives );
		objectiveEntity->AttachComponent( m_taskObjectiveText );

		objectiveEntity->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( 4.0, -((m_taskTitleText->GetCharacterHeight() / 2.0) + 6.0), 0.0 ) );
	}
	m_entity->AttachChild( objectiveEntity );

}

void ActiveTaskItem::_OnFixedUpdate()
{

	if( m_task != 0 )
	{
		if( m_taskObjectiveText != 0 )
		{
			std::wstring objectives;
			for( auto itr = m_task->GetObjectives().begin(); itr != m_task->GetObjectives().end(); itr++ )
			{
				objectives += (*itr)->GetDescriptor();
				if( (*itr)->IsCompleted() )
				{
					objectives += L" (COMPLETE!)";
				} else
				{
					objectives += L" " + Kiwi::ToWString( (*itr)->GetCompletionCount() ) + L"/" + Kiwi::ToWString( (*itr)->GetTotalCount() );
				}
				objectives += L"\n";
			}
			m_taskObjectiveText->SetText( objectives );
		}

		if( m_task->IsCompleted() && m_taskTitleText != 0 )
		{
			m_taskTitleText->SetText( m_task->GetName() + L" (COMPLETE!)" );
		}
	}

}

bool ActiveTaskItem::IsComplete()const
{

	if( m_task != 0 )
	{
		return m_task->IsCompleted();
	}

	return true;

}