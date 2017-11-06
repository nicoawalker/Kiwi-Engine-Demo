#ifndef _TASK_H_
#define _TASK_H_

#include "TaskObjective.h"
#include "TaskReward.h"

#include <Core\IReferencedObject.h>

#include <string>
#include <vector>

class CharacterController;

namespace Kiwi
{
	class Entity;
	class Scene;
}

class Task:
	public Kiwi::IReferencedObject
{
protected:

	static int GlobalTaskID;

	Kiwi::Scene* m_scene;

	TaskReward m_reward;

	//stores the name of the source that assigned the task
	std::wstring m_taskSource;

	int m_globalID; //unique id for each created task
	int m_taskID; //id that identifies what type of task this is

	std::wstring m_taskName;
	std::wstring m_taskDescription;

	std::vector<TaskObjective*> m_objectives;

	bool m_isCompleted;

	bool m_isReturned; //true once the task has been completed and returned for reward

protected:

	virtual void _CheckCompletion();

public:

	Task( int taskID, Kiwi::Scene* scene, std::wstring taskSource );
	virtual ~Task();

	void Update();

	void AddObjective( TaskObjective* objective );
	void AddObjectives( std::vector<TaskObjective*>& objectives );

	void SetReward( TaskReward& reward ) { m_reward = reward; }

	void SetReturned( bool isReturned ) { m_isReturned = isReturned; }

	void SetName( std::wstring taskName ) { m_taskName = taskName; }

	std::vector<TaskObjective*>& GetObjectives() { return m_objectives; }

	TaskReward& GetReward() { return m_reward; }

	std::wstring GetName()const { return m_taskName; }
	std::wstring GetDescription()const { return m_taskDescription; }

	int GetID()const { return m_taskID; }
	int GetGlobalID()const { return m_globalID; }

	std::wstring GetSource()const { return m_taskSource; }

	bool IsCompleted()const { return m_isCompleted; }
	bool IsReturned()const { return m_isReturned; }

};

#endif