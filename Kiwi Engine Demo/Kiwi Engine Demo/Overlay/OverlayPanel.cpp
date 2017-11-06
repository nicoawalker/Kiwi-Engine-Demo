#include "OverlayPanel.h"

#include <Core\Assert.h>
#include <Core\Entity.h>
#include <Core\Scene.h>

OverlayPanel::OverlayPanel( Kiwi::Entity* overlay, std::wstring name, const Kiwi::Vector3d& dimensions ):
	Kiwi::Component( name ) 
{

	m_panelDimensions = dimensions;

	m_overlay = overlay;
	m_panelBackground = 0;
	assert( m_overlay != 0 );

	Kiwi::Scene* scene = m_overlay->GetScene();

	m_panelEntity = scene->CreateEntity( name );
	m_panelEntity->FindComponent<Kiwi::Transform>()->SetScale( m_panelDimensions );
	m_panelEntity->SetEntityType( Kiwi::Entity::ENTITY_2D );

	m_overlay->AttachChild( m_panelEntity );

}

void OverlayPanel::SetPosition( const Kiwi::Vector2d& position )
{

	Kiwi::Transform* trans = m_panelEntity->FindComponent<Kiwi::Transform>();
	assert( trans != 0 );

	trans->SetPosition( Kiwi::Vector3d( position.x, position.y, trans->GetPosition().z ) );

	m_position.x = position.x;
	m_position.y = position.y;

}

void OverlayPanel::SetDepth( double depth )
{

	Kiwi::Transform* trans = m_panelEntity->FindComponent<Kiwi::Transform>();
	assert( trans != 0 );

	trans->SetPosition( Kiwi::Vector3d( trans->GetPosition().x, trans->GetPosition().y, depth ) );

	m_position.z = depth;

}