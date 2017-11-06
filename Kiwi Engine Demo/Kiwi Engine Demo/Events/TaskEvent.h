#ifndef _TASKEVENT_H_
#define _TASKEVENT_H_

#include <Core\Events\IGlobalEvent.h>

#define TASK_EVENT 10001

namespace Kiwi
{
	class Entity;
}

class Task;

enum TASKEVENTTYPE { TASK_UPDATED, TASK_COMPLETED, TASK_ACCEPTED, TASK_RETURNED, TASK_FAILED };

class TaskEvent :
	public Kiwi::IGlobalEvent
{
protected:

	Task* m_task;

	Kiwi::Entity* m_eventSource;

	TASKEVENTTYPE m_taskEventType;

	int m_taskID;

public:

	TaskEvent( Task* task, TASKEVENTTYPE eventType, Kiwi::Entity* eventSource = 0 );

	~TaskEvent();

	void SetTaskID( int taskID ) { m_taskID = taskID; }

	Task* GetTask()const { return m_task; }
	TASKEVENTTYPE GetEventType()const { return m_taskEventType; }
	Kiwi::Entity* GetSource()const { return m_eventSource; }
	int GetTaskID()const { return m_taskID; }

};

#endif