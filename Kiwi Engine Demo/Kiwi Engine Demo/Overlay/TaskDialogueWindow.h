#ifndef _TASKDIALOGUEWINDOW_H_
#define _TASKDIALOGUEWINDOW_H_

#include "OverlayWindow.h"

class Task;

class TaskDialogueWindow :
	public OverlayWindow
{
protected:

	Kiwi::Entity* m_acceptButton;
	Kiwi::Entity* m_declineButton;

	Kiwi::Vector2d m_buttonDimensions;

	Task* m_task;

protected:

	void _OnButtonRelease( const Kiwi::UIButtonEvent& evt );

public:

	TaskDialogueWindow( Kiwi::Entity* overlay, std::wstring name, const Kiwi::Vector3d& dimensions, Task* task );

};

#endif