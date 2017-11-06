#ifndef _OVERLAYWINDOW_H_
#define _OVERLAYWINDOW_H_

#include "OverlayPanel.h"

#include <Graphics\UI\UIButton.h>
#include <Graphics\UI\Events\UIButtonEventListener.h>
#include <Graphics\Text.h>

class OverlayWindow :
	public OverlayPanel,
	public Kiwi::UIButtonEventListener
{
protected:

	Kiwi::Entity* m_titleBar;
	Kiwi::Entity* m_background;

	double m_titleBarHeight;

	Kiwi::Vector2d m_mouseAnchorPoint; //position of the mouse when the title bar is first clicked

protected:

	void _OnButtonPress( const Kiwi::UIButtonEvent& evt );
	void _OnButtonHeld( const Kiwi::UIButtonEvent& evt );
	void _OnButtonRelease( const Kiwi::UIButtonEvent& evt ) {}

	void _OnFixedUpdate() {}

public:

	OverlayWindow( Kiwi::Entity* overlay, std::wstring name, const Kiwi::Vector3d& dimensions );
	virtual ~OverlayWindow();

	void Close();

};

#endif