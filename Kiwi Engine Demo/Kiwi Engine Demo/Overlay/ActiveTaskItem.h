#ifndef _ACTIVETASKITEM_H_
#define _ACTIVETASKITEM_H_

#include <Core\Component.h>
#include <Core\Vector2d.h>

#include <Graphics\Text.h>

class ActiveTaskPanel;
class Task;

class ActiveTaskItem:
	public Kiwi::Component
{
protected:

	int m_itemID;

	ActiveTaskPanel* m_panel;
	Task* m_task;

	Kiwi::Text* m_taskTitleText;
	Kiwi::Text* m_taskObjectiveText;

	Kiwi::Vector2d m_dimensions;

protected:

	void _OnAttached();
	void _OnFixedUpdate();

public:

	ActiveTaskItem( int m_itemID, ActiveTaskPanel* panel, Task* task, const Kiwi::Vector2d& dimensions );
	~ActiveTaskItem();

	int GetID()const { return m_itemID; }

	bool IsComplete()const;

	const Kiwi::Vector2d& GetDimensions()const { return m_dimensions; }

};

#endif