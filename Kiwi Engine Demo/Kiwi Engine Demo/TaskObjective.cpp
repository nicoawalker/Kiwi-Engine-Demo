#include "TaskObjective.h"

void TaskObjective::Update()
{

	//process current waiting events
	while( this->PullNextEvent() );

	if( m_isCompleted == false )
	{
		if( m_completionCount == m_totalCount )
		{
			m_isCompleted = true;
		}
	}

}