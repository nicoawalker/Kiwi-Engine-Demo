#include "OverlayController.h"
#include "SelectionPanel.h"
#include "Selector.h"
#include "FPSCounter.h"
#include "PlayerStatusPanel.h"
#include "ActiveTaskPanel.h"
#include "OverlayWindow.h"
#include "TaskDialogueWindow.h"

#include "../CharacterController.h"

#include <Core\Scene.h>
#include <Core\Entity.h>

#include <Graphics\RenderWindow.h>

OverlayController::OverlayController( Kiwi::Entity* playerEntity ):
	Kiwi::Component(L"OverlayController")
{

	m_selector = 0;
	m_selectionPanel = 0;
	m_characterController = 0;
	m_fpsCounter = 0;
	m_playerStatusPanel = 0;
	m_activeTaskPanel = 0;

	m_playerEntity = playerEntity;

}

OverlayController::~OverlayController()
{

}

void OverlayController::_OnAttached()
{

	Kiwi::Scene* scene = m_entity->GetScene();
	Kiwi::RenderWindow* renderWindow = scene->GetRenderWindow();

	m_entity->SetEntityType( Kiwi::Entity::ENTITY_2D );

	m_overlayDimensions.Set( renderWindow->GetClientSize().x, renderWindow->GetClientSize().y );

	m_characterController = m_playerEntity->FindComponent<CharacterController>();
	assert( m_characterController != 0 );

	m_selector = new Selector( m_characterController, 3.5 );
	m_entity->AttachComponent( m_selector );

	m_selectionPanel = new SelectionPanel( scene, m_selector );
	Kiwi::Transform* infoTransform = m_selectionPanel->GetMainEntity()->FindComponent<Kiwi::Transform>();
	Kiwi::Vector3d infoPanelPosition( 0.0, m_overlayDimensions.y / 2.0 - (infoTransform->GetScale().y / 2.0) - 10.0, 0.0 );
	infoTransform->SetPosition( infoPanelPosition );
	m_entity->AttachComponent( m_selectionPanel );

	FPSCounter* fpsCounter = new FPSCounter( m_entity );
	m_entity->AttachComponent( fpsCounter );
	fpsCounter->SetPosition( Kiwi::Vector2d( (m_overlayDimensions.x / 2.0) - (fpsCounter->GetDimensions().x / 2.0), (m_overlayDimensions.y / 2.0) - (fpsCounter->GetDimensions().y / 2.0) ) );

	m_playerStatusPanel = new PlayerStatusPanel( m_entity );
	m_playerStatusPanel->SetPosition( Kiwi::Vector2d( -(m_overlayDimensions.x / 2.0) + 240, (m_overlayDimensions.y / 2.0) - 42.0 ) );
	m_entity->AttachComponent( m_playerStatusPanel );

	m_activeTaskPanel = new ActiveTaskPanel( m_entity, Kiwi::Vector3d( 250.0, 350.0, 1.0 ) );
	m_activeTaskPanel->SetPosition( Kiwi::Vector2d( (m_overlayDimensions.x / 2.0) - 130.0, 100.0 ) );
	m_entity->AttachComponent( m_activeTaskPanel );

}

TaskDialogueWindow* OverlayController::CreateTaskDialogue( Task* task )
{

	if( task == 0 )
	{
		return 0;
	}

	TaskDialogueWindow* newWindow = new TaskDialogueWindow( m_entity, L"TaskDialogue", Kiwi::Vector3d( 360.0, 476.0, 1.0 ), task );
	newWindow->SetPosition( m_dialoguePosition );

	m_entity->AttachComponent( newWindow );

	return newWindow;

}