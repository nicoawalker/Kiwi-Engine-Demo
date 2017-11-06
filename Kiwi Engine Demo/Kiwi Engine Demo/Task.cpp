#include "Task.h"

#include <Core\Assert.h>
#include <Core\Scene.h>

int Task::GlobalTaskID = 0;

Task::Task( int taskID, Kiwi::Scene* scene, std::wstring taskSource )
{

	assert( scene != 0 );

	m_scene = scene;
	m_taskID = taskID;
	m_isCompleted = false;
	m_isReturned = false;
	m_taskSource = taskSource;

	m_globalID = GlobalTaskID++;

}

Task::~Task()
{

	for( auto itr = m_objectives.begin(); itr != m_objectives.end(); itr++ )
	{
		m_scene->RemoveGlobalEventListener( *itr );
	}

}

void Task::_CheckCompletion()
{

	//check all objectives for completion
	unsigned int numCompleted = 0;
	for( auto itr = m_objectives.begin(); itr != m_objectives.end(); itr++ )
	{
		if( (*itr)->IsCompleted() )
		{
			numCompleted++;
		}
	}

	//if all objectives have been completed, this task is also completed
	if( numCompleted == m_objectives.size() )
	{
		m_isCompleted = true;
	}

}

void Task::Update()
{

	this->_CheckCompletion();

	if( m_isCompleted == false )
	{
		for( auto itr = m_objectives.begin(); itr != m_objectives.end(); itr++ )
		{
			(*itr)->Update();
		}
	}

}

void Task::AddObjective( TaskObjective* objective )
{ 

	m_objectives.push_back( objective ); 
	m_scene->AddGlobalEventListener( objective );

}

void Task::AddObjectives( std::vector<TaskObjective*>& objectives )
{

	for( auto itr = objectives.begin(); itr != objectives.end(); itr++ )
	{
		m_objectives.push_back( *itr );
		m_scene->AddGlobalEventListener( *itr );
	}

}