#include "PlayerStatusPanel.h"
#include "../CharacterController.h"

#include <Core\Entity.h>

#include <Graphics\Text.h>

PlayerStatusPanel::PlayerStatusPanel( Kiwi::Entity* overlay ):
	OverlayPanel( overlay, L"PlayerStatusPanel", Kiwi::Vector3d(480.0, 64.0, 1.0) )
{

	Kiwi::Scene* scene = m_panelEntity->GetScene();
	assert( scene != 0 );
	Kiwi::Entity* player = scene->FindEntityWithName( L"Player" );
	assert( player != 0 );

	m_characterController = player->FindComponent<CharacterController>();
	assert( m_characterController != 0 );

	m_panelBackground = new Kiwi::UISprite( L"PSPBackground", L"PlayerStatusPanel" );
	m_panelEntity->AttachComponent( m_panelBackground );

	m_hpBar = scene->CreateEntity( L"PSPHPBar" );
	m_epBar = scene->CreateEntity( L"PSPEPBar" );

	m_hpBar->AttachComponent( new Kiwi::UISprite( L"HPBarSprite", L"HPBar" ) );
	m_epBar->AttachComponent( new Kiwi::UISprite( L"EPBarSprite", L"EPBar" ) );
	m_hpBarPosition.Set( 14.0, 13.0, 10.0 );
	m_epBarPosition.Set( 5.0, -15.0, 10.0 );
	m_hpBar->FindComponent<Kiwi::Transform>()->SetPosition( m_hpBarPosition );
	m_epBar->FindComponent<Kiwi::Transform>()->SetPosition( m_epBarPosition );

	m_hpBar->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( 432.0, 24.0, 1.0 ) );
	m_epBar->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( 416.0, 16.0, 1.0 ) );

	m_panelEntity->AttachChild( m_hpBar );
	m_panelEntity->AttachChild( m_epBar );

	Kiwi::Entity* htEntity = scene->CreateEntity( L"PSPHPText" );
	m_healthText = new Kiwi::Text( L"PSPHealthText", L"Lato_20pt_Outline", Kiwi::Vector2d( 300.0, 100.0 ) );
	m_healthText->SetAlignment( Kiwi::Font::ALIGN_CENTRE );
	m_healthText->SetText( L"0/0" );
	htEntity->AttachComponent( m_healthText );
	m_healthText->SetShader( L"Default2DShader" );
	m_panelEntity->AttachChild( htEntity );
	htEntity->FindComponent<Kiwi::Transform>()->SetPosition( m_hpBarPosition + Kiwi::Vector3d(0.0, 0.0, 1.0) );

	Kiwi::Entity* epEntity = scene->CreateEntity( L"PSPEPText" );
	m_epText = new Kiwi::Text( L"PSPEPText", L"Lato_20pt_Outline", Kiwi::Vector2d( 300.0, 100.0 ) );
	m_epText->SetAlignment( Kiwi::Font::ALIGN_CENTRE );
	m_epText->SetText( L"0/0" );
	epEntity->AttachComponent( m_epText );
	m_epText->SetShader( L"Default2DShader" );
	m_panelEntity->AttachChild( epEntity );
	epEntity->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( 0.9, 0.9, 1.0 ) );
	epEntity->FindComponent<Kiwi::Transform>()->SetPosition( m_epBarPosition + Kiwi::Vector3d( 0.0, 0.0, 1.0 ) );

}

PlayerStatusPanel::~PlayerStatusPanel()
{



}

void PlayerStatusPanel::_OnFixedUpdate()
{

	double currentHP = m_characterController->GetCurrentHealth();
	double maxHP = m_characterController->GetMaxHealth();
	double currentEP = m_characterController->GetCurrentEP();
	double maxEP = m_characterController->GetMaxEP();

	if( m_hpBar )
	{
		double xScale = Kiwi::RoundToNearestd( 432.0 * (currentHP / maxHP), 2.0 );
		m_hpBar->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( xScale, 24.0, 1.0 ) );
		m_hpBar->FindComponent<Kiwi::Transform>()->SetPosition( m_hpBarPosition - Kiwi::Vector3d( (432.0 - xScale) / 2.0, 0.0, 0.0) );
	}
	if( m_epBar )
	{
		double xScale = Kiwi::RoundToNearestd( 416.0 * (currentEP / maxEP), 2.0 );
		m_epBar->FindComponent<Kiwi::Transform>()->SetScale( Kiwi::Vector3d( xScale, 16.0, 1.0 ) );
		m_epBar->FindComponent<Kiwi::Transform>()->SetPosition( m_epBarPosition - Kiwi::Vector3d( (416.0 - xScale) / 2.0, 0.0, 0.0 ) );
	}

	if( m_healthText != 0 )
	{
		m_healthText->SetText( Kiwi::ToWString( (int)currentHP ) + L" / " + Kiwi::ToWString( (int)maxHP ) + L" (+" + Kiwi::ToWString(m_characterController->GetCurrentHPRegen()) + L")" );
	}
	if( m_epText != 0 )
	{
		m_epText->SetText( Kiwi::ToWString( (int)currentEP ) + L" / " + Kiwi::ToWString( (int)maxEP ) + L" (+" + Kiwi::ToWString( m_characterController->GetCurrentEPRegen() ) + L")" );
	}

}