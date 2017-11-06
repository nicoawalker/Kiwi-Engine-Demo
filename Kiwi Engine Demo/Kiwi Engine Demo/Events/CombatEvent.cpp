#include "CombatEvent.h"

#include <Core\Entity.h>

CombatEvent::CombatEvent( Kiwi::Entity* source, COMBATEVENTTYPE eventType ) :
	Kiwi::IGlobalEvent( COMBAT_EVENT )
{

	m_source = source;
	m_target = 0;
	m_combatEventType = eventType;

	if( m_source )
	{
		m_source->Reserve();
	}

}

CombatEvent::CombatEvent( Kiwi::Entity* source, Kiwi::Entity* target, COMBATEVENTTYPE eventType ):
	Kiwi::IGlobalEvent( COMBAT_EVENT )
{

	m_source = source;
	m_target = target;
	m_combatEventType = eventType;

	if( m_source )
	{
		m_source->Reserve();
	}
	if( m_target )
	{
		m_target->Reserve();
	}

}

CombatEvent::~CombatEvent()
{

	if( m_source )
	{
		m_source->Free();
	}
	if( m_target )
	{
		m_target->Free();
	}

}