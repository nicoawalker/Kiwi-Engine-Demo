#include "TaskEvent.h"

#include "../Task.h"

#include <Core\Assert.h>
#include <Core\Entity.h>

TaskEvent::TaskEvent( Task* task, TASKEVENTTYPE eventType, Kiwi::Entity* eventSource ) :
	Kiwi::IGlobalEvent( 10001 )
{

	assert( task != 0 );

	m_task = task;
	m_taskEventType = eventType;
	m_eventSource = eventSource;

	m_taskID = 0;

	//ensure that the task isn't deleted before this event
	m_task->Reserve();

}

TaskEvent::~TaskEvent()
{

	m_task->Free();

}