#ifndef _ACTIVETASKPANEL_H_
#define _ACTIVETASKPANEL_H_

#include "OverlayPanel.h"

#include <Core\Vector2d.h>

#include <Core\Events\IGlobalEventListener.h>

#include <unordered_map>

class Task;
class ActiveTaskItem;

namespace Kiwi
{
	class Text;
};

class ActiveTaskPanel :
	public OverlayPanel,
	public Kiwi::IGlobalEventListener
{
protected:

	std::unordered_map<int, ActiveTaskItem*> m_activeTasks;

	Kiwi::Text* m_headerText;

	//position where the next new task will be placed
	Kiwi::Vector2d m_newTaskPosition;

	//vertical spacing between tasks
	double m_taskItemPadding;

	//maximum number of tasks that can be present in the task panel at once
	int m_maxTasks;

protected:

	void _ProcessGlobalEvent( Kiwi::GlobalEventPtr e );

	void _OnFixedUpdate();

	void _AddNewTask( Task* task );

public:

	ActiveTaskPanel( Kiwi::Entity* overlay, const Kiwi::Vector3d& dimensions );
	virtual ~ActiveTaskPanel();

};

#endif