#ifndef _FPSCOUNTER_H_
#define _FPSCOUNTER_H_

#include "OverlayPanel.h"

namespace Kiwi
{
	class Text;
	class Entity;
}

class FPSCounter:
	public OverlayPanel
{
protected:

	Kiwi::Text* m_fpsText;

	double m_updateInterval;

protected:

	void _OnAttached();
	void _OnFixedUpdate();

public:

	FPSCounter( Kiwi::Entity* overlay );

};

#endif