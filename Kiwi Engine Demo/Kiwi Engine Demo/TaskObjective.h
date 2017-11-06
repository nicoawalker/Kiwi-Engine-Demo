#ifndef _TASKOBJECTIVE_H_
#define _TASKOBJECTIVE_H_

#include <Core\Events\IGlobalEventListener.h>

#include <string>

class TaskObjective:
	public Kiwi::IGlobalEventListener
{
protected:

	//how many times this objective needs to be completed
	unsigned int m_totalCount;

	//how many times this objective has already been completed
	unsigned int m_completionCount;

	//descriptor that will be displayed for this task
	std::wstring m_objectiveDescriptor;

	bool m_isCompleted;

public:

	TaskObjective( unsigned int totalCount = 0, unsigned int completionCount = 0 ) :
		m_totalCount( totalCount ), m_completionCount( completionCount ), m_isCompleted(false)
	{
	}

	virtual ~TaskObjective() {}

	virtual void Update();

	unsigned int GetTotalCount()const { return m_totalCount; }

	unsigned int GetCompletionCount()const { return m_completionCount; }

	std::wstring GetDescriptor()const { return m_objectiveDescriptor; }

	bool IsCompleted()const { return m_isCompleted; }

	void SetCompletionCount( unsigned int newCount ) { m_completionCount = newCount; }

	void SetCompleted( bool isCompleted ) { m_isCompleted = isCompleted; }

};

#endif