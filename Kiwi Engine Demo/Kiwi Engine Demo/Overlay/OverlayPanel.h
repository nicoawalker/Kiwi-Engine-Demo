#ifndef _OVERLAYPANEL_H_
#define _OVERLAYPANEL_H_

#include <Core/Component.h>
#include <Core\Vector3d.h>
#include <Core\Vector2d.h>

#include <Graphics\UI\UISprite.h>

namespace Kiwi
{
	class Entity;
}

class OverlayPanel :
	public Kiwi::Component
{
protected:

	Kiwi::Vector3d m_panelDimensions;
	Kiwi::Vector3d m_position;

	Kiwi::Entity* m_panelEntity;
	Kiwi::Entity* m_overlay;

	Kiwi::UISprite* m_panelBackground;

public:

	OverlayPanel( Kiwi::Entity* overlay, std::wstring name, const Kiwi::Vector3d& dimensions );

	void SetPosition( const Kiwi::Vector2d& position );
	void SetDepth( double depth );

	const Kiwi::Vector3d& GetDimensions()const { return m_panelDimensions; }
	const Kiwi::Vector2d& GetPosition()const { return Kiwi::Vector2d( m_position.x, m_position.y ); }

};

#endif