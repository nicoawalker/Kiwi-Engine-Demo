#include "OverlayWindow.h"

#include <Core\Entity.h>
#include <Core\Scene.h>
#include <Core\RawInputWrapper.h>

#include <Graphics\RenderWindow.h>

OverlayWindow::OverlayWindow( Kiwi::Entity* overlay, std::wstring name, const Kiwi::Vector3d& dimensions ):
	OverlayPanel( overlay, name, dimensions )
{

	m_titleBarHeight = 24.0;

	Kiwi::Scene* scene = overlay->GetScene();
	assert( scene != 0 );

	m_titleBar = scene->CreateEntity( m_objectName + L"/TitleBar" );
	Kiwi::UIButton* titleBarButton = new Kiwi::UIButton( L"TitleBar", Kiwi::Vector2d( dimensions.x, m_titleBarHeight ), L"OverlayWindowTitle" );
	titleBarButton->AddButtonEventListener( this );
	m_titleBar->AttachComponent( titleBarButton );
	m_titleBar->FindComponent<Kiwi::Transform>()->SetPosition( m_position + Kiwi::Vector3d( 0.0, (dimensions.y / 2.0) + (m_titleBarHeight / 2.0), 0.0 ) );

	m_background = scene->CreateEntity( m_objectName + L"/Background" );
	Kiwi::UISprite* bgSprite = new Kiwi::UISprite( L"Background", L"OverlayWindowBackground" );
	m_background->AttachComponent( bgSprite );
	m_background->FindComponent<Kiwi::Transform>()->SetScale( m_panelDimensions );

	m_panelEntity->AttachChild( m_background );
	m_panelEntity->AttachChild( m_titleBar );

}

OverlayWindow::~OverlayWindow()
{
}

void OverlayWindow::_OnButtonPress( const Kiwi::UIButtonEvent& evt )
{

	if( evt.GetSource()->GetName().compare( L"TitleBar" ) == 0 )
	{
		m_mouseAnchorPoint = evt.GetMousePos();
	}

}

void OverlayWindow::_OnButtonHeld( const Kiwi::UIButtonEvent& evt )
{

	Kiwi::Vector2d relPos = evt.GetMousePos();// -Kiwi::Vector2d( m_position.x, m_position.y );

	if( relPos.y < m_position.y + (m_panelDimensions.y / 2.0) && relPos.y > m_position.y + ( (m_panelDimensions.y / 2.0) + m_titleBarHeight ) &&
		relPos.x > m_position.x - (m_panelDimensions.x / 2.0) && relPos.x < m_position.x + (m_panelDimensions.x / 2.0) )
	{
		Kiwi::Vector2d deltaMouse = (evt.GetMousePos() - m_mouseAnchorPoint);
		this->SetPosition( this->GetPosition() + deltaMouse );
		m_mouseAnchorPoint = m_mouseAnchorPoint + deltaMouse;
	}

}

void OverlayWindow::Close()
{

	m_panelEntity->Shutdown();
	this->Shutdown();

}