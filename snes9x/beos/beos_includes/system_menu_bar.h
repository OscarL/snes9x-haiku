#ifndef __SYSTEM_MENU_BAR_H__
#define __SYSTEM_MENU_BAR_H__

#include <MenuBar.h>
#include <MenuItem.h>

struct s9x_settings;

class GameMenuBar : public BMenuBar
{
public:
	GameMenuBar(BRect frame, const char *title, s9x_settings s);
	virtual void MakeFocus(bool focus = true);
	virtual void set_overlay(bool what, int index = 0);
	virtual void set_items(bool ch);
private:
	BMenu *mode_menu;
	BMenu *graphics_menu;
};

#endif