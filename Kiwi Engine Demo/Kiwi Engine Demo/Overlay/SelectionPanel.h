#ifndef _SELECTIONPANEL_H_
#define _SELECTIONPANEL_H_

#include <Core\Component.h>

namespace Kiwi
{
	class Mesh;
	class UISprite;
	class Text;
	class Entity;
	class Scene;
}

class INPCController;
class Selector;
class CharacterController;

class SelectionPanel:
	public Kiwi::Component
{
protected:

	Kiwi::Entity* m_mainEntity;
	Kiwi::Entity* m_nameTextEntity;

	Kiwi::UISprite* m_background;
	Kiwi::Text* m_nameText;
	Kiwi::Scene* m_scene;

	Selector* m_selector;

	INPCController* m_currentSelection;
	CharacterController* m_cc;

protected:

	void _OnFixedUpdate();

public:

	SelectionPanel( Kiwi::Scene* scene, Selector* selector );

	void SetSelection( INPCController* npc );

	Kiwi::Entity* GetMainEntity()const { return m_mainEntity; }

	INPCController* GetSelection()const { return m_currentSelection; }

};

#endif