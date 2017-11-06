#ifndef _PLAYERSTATUSPANEL_H_
#define _PLAYERSTATUSPANEL_H_

#include "OverlayPanel.h"

class CharacterController;

namespace Kiwi
{
	class Text;
	class Entity;
}

class PlayerStatusPanel :
	public OverlayPanel
{
protected:

	CharacterController* m_characterController;

	Kiwi::Text* m_healthText;
	Kiwi::Text* m_epText;

	Kiwi::Entity* m_hpBar;
	Kiwi::Vector3d m_hpBarPosition;
	Kiwi::Entity* m_epBar;
	Kiwi::Vector3d m_epBarPosition;

protected:

	void _OnFixedUpdate();

public:

	PlayerStatusPanel( Kiwi::Entity* overlay );
	~PlayerStatusPanel();

};

#endif