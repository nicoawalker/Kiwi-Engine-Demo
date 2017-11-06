#ifndef _SELECTIONMENUITEM_H_
#define _SELECTIONMENUITEM_H_

#include <string>

class SelectionMenuItem
{
protected:

	std::wstring m_itemName;

	std::wstring m_tag;

public:

	SelectionMenuItem() {}
	~SelectionMenuItem() {}

};

#endif