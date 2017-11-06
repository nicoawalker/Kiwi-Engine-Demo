#ifndef _TASKKILLOBJECTIVE_H_
#define _TASKKILLOBJECTIVE_H_

#define TASK_KILL_OBJECTIVE 1

#include "TaskObjective.h"

#include <string>

class TaskKillObjective :
	public TaskObjective
{
protected:

	int m_npcID;
	std::wstring m_npcType;

protected:

	void _ProcessGlobalEvent( Kiwi::GlobalEventPtr e );

public:

	TaskKillObjective( int npcID, std::wstring npcType, int killCount );

	std::wstring GetNPCType()const { return m_npcType; }
	int GetNPCID()const { return m_npcID; }

};

#endif