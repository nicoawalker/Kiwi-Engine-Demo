#include "SelectionPanel.h"
#include "../INPCController.h"
#include "Selector.h"

#include "../CharacterController.h"

#include <KiwiCore.h>
#include <KiwiGraphics.h>

#include <Graphics\UI\UISprite.h>

SelectionPanel::SelectionPanel( Kiwi::Scene* scene, Selector* selector ):
	Kiwi::Component(L"SelectionPanel")
{

	assert( scene != 0 );

	m_scene = scene;

	m_selector = selector;

	m_mainEntity = m_scene->CreateEntity( L"SelectionPanel" );
	assert( m_mainEntity != 0 );
	m_nameTextEntity = m_scene->CreateEntity( L"SelectionPanelNameText" );
	assert( m_nameTextEntity != 0 );

	m_mainEntity->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( 252.0, 64.0, 1.0 ) );
	m_mainEntity->SetEntityType( Kiwi::Entity::ENTITY_2D );

	Kiwi::UISprite* sprite = new Kiwi::UISprite( L"SelectionPanelBG", L"SelectionPanel" );
	m_mainEntity->AttachComponent( sprite );

	m_nameText = new Kiwi::Text( L"SelectedNPCText", L"Lato_20pt", Kiwi::Vector2d( 300.0, 100.0 ) );
	m_nameText->SetAlignment( Kiwi::Font::ALIGN_CENTRE );
	m_nameText->SetText( L"No Selection" );
	m_nameTextEntity->AttachComponent( m_nameText );
	m_nameText->SetShader( L"Default2DShader" );
	m_nameTextEntity->FindComponent<Kiwi::Transform>()->SetPosition( Kiwi::Vector3d( 0.0, 0.0, 10.0 ) );

	m_mainEntity->AttachChild( m_nameTextEntity );

	Kiwi::Entity* player = scene->FindEntityWithName( L"Player" );
	assert( player != 0 );
	m_cc = player->FindComponent<CharacterController>();

}

void SelectionPanel::_OnFixedUpdate()
{
	if( m_cc != 0 )
	{
		if( m_cc->GetSelectedNPC() != 0 )
		{
			this->SetSelection( m_cc->GetSelectedNPC() );

		} else
		{
			this->SetSelection( m_cc->GetVisualTargetNPC() );
		}
	}

}

void SelectionPanel::SetSelection( INPCController* npc )
{

	if( npc == 0 )
	{
		m_nameText->SetText( L"No Selection" );
		m_currentSelection = 0;

	} else
	{
		m_currentSelection = npc;
		m_nameText->SetText( m_currentSelection->GetNPCName() );
	}

}