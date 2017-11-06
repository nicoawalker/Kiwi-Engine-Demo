#include "TaskDialogueWindow.h"

#include <Core\Entity.h>
#include <Core\Scene.h>

#include "..\Task.h"

#include "..\Events\TaskEvent.h"

TaskDialogueWindow::TaskDialogueWindow( Kiwi::Entity* overlay, std::wstring name, const Kiwi::Vector3d& dimensions, Task* task ):
	OverlayWindow( overlay, name, dimensions )
{

	m_task = task;

	Kiwi::Scene* scene = overlay->GetScene();
	assert( scene != 0 );

	m_buttonDimensions.Set( 120.0, 32.0 );

	m_acceptButton = scene->CreateEntity( m_objectName + L"/AcceptButton" );
	Kiwi::UIButton* b1 = new Kiwi::UIButton( L"AcceptButton", m_buttonDimensions, L"Accept", L"Lato_20pt", L"DialogueButtonGreen" );
	b1->AddButtonEventListener( this );
	m_acceptButton->AttachComponent( b1 );
	m_acceptButton->FindComponent<Kiwi::Transform>()->SetPosition( m_position + Kiwi::Vector3d( (m_panelDimensions.x / 2.0) - (m_buttonDimensions.x / 2.0) - 10.0, -(dimensions.y / 2.0) + (m_buttonDimensions.y / 2.0) + 10.0, 10.0 ) );

	m_panelEntity->AttachChild( m_acceptButton );

	m_declineButton = 0;

}

void TaskDialogueWindow::_OnButtonRelease( const Kiwi::UIButtonEvent& evt )
{

	if( m_task != 0 )
	{
		if( evt.GetSource()->GetName().compare( L"AcceptButton" ) == 0 )
		{
			Kiwi::Scene* scene = m_overlay->GetScene();
			scene->BroadcastGlobalEvent( std::make_shared<TaskEvent>( m_task, TASK_ACCEPTED, m_panelEntity ) );

			this->Close();

		} else if( evt.GetSource()->GetName().compare( L"DeclineButton" ) == 0 )
		{
			SAFE_DELETE( m_task );
			this->Close();
		}
	}

}