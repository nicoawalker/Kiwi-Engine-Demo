#ifndef _INPCCONTROLLER_H_
#define _INPCCONTROLLER_H_

#include <Core\Component.h>

#include <Core\Events\IGlobalEventListener.h>

#include <unordered_map>

class CharacterController;

class INPCController:
	public Kiwi::Component,
	public Kiwi::IGlobalEventListener
{
protected:

	CharacterController* m_charController;

	std::wstring m_npcType;
	int m_npcID;

	std::wstring m_npcName;

	int m_currentHealth;
	int m_maxHealth;

	//whether the npc is hostile towards the player
	bool m_isHostile;

	//whether the npc is currently targetted by the player
	bool m_isTargeted;

	//true if the npc has a task to give
	bool m_hasTask;

protected:

	virtual void _OnTargeted() {}
	virtual void _OnUntargeted() {}
	virtual void _OnAttached();
	virtual void _OnPlayerInteraction() {}

public:

	INPCController( std::wstring npcType, std::wstring npcName = L"" );

	virtual ~INPCController() = 0;

	void Target();
	void Untarget();
	void Interact() { this->_OnPlayerInteraction(); }

	virtual void SetHostile( bool isHostile ) { m_isHostile = isHostile; }

	virtual bool IsTargeted()const { return m_isTargeted; }
	virtual bool IsHostile()const { return m_isHostile; }

	virtual std::wstring GetNPCType()const { return m_npcType; }
	virtual std::wstring GetNPCName()const { return m_npcName; }
	virtual int GetID()const { return m_npcID; }

};

#endif