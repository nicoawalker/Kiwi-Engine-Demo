#include "FPSCounter.h"

#include <Core\Scene.h>
#include <Core\Entity.h>
#include <Core\EngineRoot.h>

#include <Graphics/Text.h>

FPSCounter::FPSCounter( Kiwi::Entity* overlay ):
	OverlayPanel( overlay, L"FPSCounter", Kiwi::Vector3d( 50.0, 24.0, 1.0 ) )
{

	m_fpsText = 0;
	m_updateInterval = 1.0;

}

void FPSCounter::_OnAttached()
{

	Kiwi::Scene* scene = m_entity->GetScene();
	assert( scene != 0 );

	m_fpsText = new Kiwi::Text( L"FPSCounterText", L"Lato_20pt", Kiwi::Vector2d( 50.0, 24.0 ) );
	assert( m_fpsText != 0 );

	Kiwi::Entity* textEntity = scene->CreateEntity( L"FPSCounter/TextEntity" );

	m_fpsText->SetAlignment( Kiwi::Font::ALIGN_RIGHT );
	m_fpsText->SetText( Kiwi::ToWString( scene->GetEngine()->GetGameTimer()->GetFramesPerSecond() ) );
	textEntity->AttachComponent( m_fpsText );
	m_fpsText->SetShader( L"Default2DShader" );

	m_panelEntity->AttachChild( textEntity );
	
}

void FPSCounter::_OnFixedUpdate()
{

	Kiwi::Scene* scene = m_entity->GetScene();

	static double counter = 0.0;

	counter += scene->GetEngine()->GetGameTimer()->GetFixedDeltaTime();

	if( counter >= m_updateInterval )
	{
		counter -= m_updateInterval;
		m_fpsText->SetText( Kiwi::ToWString( scene->GetEngine()->GetGameTimer()->GetFramesPerSecond() ) );
	}

}