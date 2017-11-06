#include "TaskKillObjective.h"
#include "INPCController.h"

#include "Events\CombatEvent.h"

#include <Core\Entity.h>

TaskKillObjective::TaskKillObjective( int npcID, std::wstring npcType, int killCount )
{

	m_npcID = npcID;
	m_totalCount = killCount;
	m_completionCount = 0;
	m_npcType = npcType;

	m_objectiveDescriptor = L"Kill " + npcType + L"s";

	this->SetRegisteredEventIDs( { COMBAT_EVENT } );

}

void TaskKillObjective::_ProcessGlobalEvent( Kiwi::GlobalEventPtr e )
{

	if( e != 0 )
	{
		CombatEvent* combatEvent = dynamic_cast<CombatEvent*>(e.get());
		if( combatEvent != 0 && combatEvent->GetEventType() == NPC_KILLED )
		{
			if( combatEvent->GetSource() != 0 )
			{
				INPCController* npc = combatEvent->GetSource()->FindComponent<INPCController>();
				if( npc != 0 && npc->GetID() == m_npcID )
				{
					m_completionCount++;
				}
			}
		}
	}

}