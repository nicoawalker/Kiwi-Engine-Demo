#ifndef _COMBATEVENT_H_
#define _COMBATEVENT_H_

#include <Core\Events\IGlobalEvent.h>

#define COMBAT_EVENT 10002

namespace Kiwi
{
	class Entity;
}

class Task;

enum COMBATEVENTTYPE { COMBAT_STARTED, COMBAT_ENDED, NPC_KILLED };

class CombatEvent :
	public Kiwi::IGlobalEvent
{
protected:

	Kiwi::Entity* m_target;
	Kiwi::Entity* m_source;

	COMBATEVENTTYPE m_combatEventType;

public:

	CombatEvent( Kiwi::Entity* source, COMBATEVENTTYPE eventType );
	CombatEvent( Kiwi::Entity* source, Kiwi::Entity* target, COMBATEVENTTYPE eventType );
	virtual ~CombatEvent();

	COMBATEVENTTYPE GetEventType()const { return m_combatEventType; }
	Kiwi::Entity* GetSource()const { return m_source; }
	Kiwi::Entity* GetTarget()const { return m_target; }

};

#endif