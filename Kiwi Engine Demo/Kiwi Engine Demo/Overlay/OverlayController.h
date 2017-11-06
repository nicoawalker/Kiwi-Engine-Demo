#ifndef _OVERLAYCONTROLLER_H_
#define _OVERLAYCONTROLLER_H_

#include <Core\Component.h>
#include <Core\Vector2d.h>

class Selector;
class SelectionPanel;
class CharacterController;
class FPSCounter;
class PlayerStatusPanel;
class ActiveTaskPanel;
class TaskDialogueWindow;
class Task;

namespace Kiwi
{
	class Entity;
}

class OverlayController:
	public Kiwi::Component
{
protected:

	Selector* m_selector;
	SelectionPanel* m_selectionPanel;

	ActiveTaskPanel* m_activeTaskPanel;

	Kiwi::Entity* m_playerEntity;
	CharacterController* m_characterController;

	FPSCounter* m_fpsCounter;

	PlayerStatusPanel* m_playerStatusPanel;

	Kiwi::Vector2d m_overlayDimensions;

	//stores the position to place new dialogue windows
	Kiwi::Vector2d m_dialoguePosition;

protected:

	void _OnAttached();

public:

	OverlayController( Kiwi::Entity* playerEntity );
	~OverlayController();

	void SetDialoguePosition( const Kiwi::Vector2d& position ) { m_dialoguePosition = position; }

	TaskDialogueWindow* CreateTaskDialogue( Task* task );

};

#endif