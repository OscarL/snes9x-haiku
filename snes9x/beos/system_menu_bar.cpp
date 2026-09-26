#include <String.h>
#include "s9x_main_window.h"
#include "system_menu_bar.h"
#include "s9x_constants.h"


GameMenuBar::GameMenuBar(BRect frame, const char *title, s9x_settings s)
	: BMenuBar(frame, title)
{
	// Game Menu
	BMenu *game_menu = new BMenu("Game");
	game_menu->AddItem(new BMenuItem("Load Rom", new BMessage(S9x_LOAD), 'O'));
	game_menu->AddSeparatorItem();
	game_menu->AddItem(new BMenuItem("Reset", new BMessage(S9x_RESET), 'R'));
	game_menu->AddItem(new BMenuItem("Pause Emulation", new BMessage(S9x_PAUSE), 'P'));
	game_menu->AddSeparatorItem();
	game_menu->AddItem(new BMenuItem("Rom Info", new BMessage(S9x_ROMINFO), 'D'));
	game_menu->AddItem(new BMenuItem("Save Spc", new BMessage(S9x_SAVE_SPC), 'S'));
	game_menu->AddSeparatorItem();

	BMenu *save_menu = new BMenu("Save State");
	for(int i = 0; i < 5; i++)
	{
		BMessage *msg = new BMessage(S9x_SAVE_STATE);
		BString item_name;
		
		item_name << "Slot " << i+1;
		msg->AddInt32("save_slot", i);
		
		save_menu->AddItem(new BMenuItem(item_name.String(), msg));
	}
	game_menu->AddItem(save_menu);
	
	BMenu *load_menu = new BMenu("Load State");
	for(int i = 0; i < 5; i++)
	{
		BMessage *msg = new BMessage(S9x_LOAD_STATE);
		BString item_name;
		
		item_name << "Slot " << i+1;
		msg->AddInt32("load_slot", i);
		
		load_menu->AddItem(new BMenuItem(item_name.String(), msg));
	}
	game_menu->AddItem(load_menu);

	game_menu->AddSeparatorItem();
	game_menu->AddItem(new BMenuItem("Exit", new BMessage(S9x_EXIT), 'Q'));
	AddItem(game_menu);
	
	// Options Menu
	BMenu *options = new BMenu("Options");
	AddItem(options);
	
	// Options->Sound
	BMenu *sound_menu = new BMenu("Sound");
	BMenuItem *item;
	sound_menu->AddItem(item = new BMenuItem("Mute", new BMessage(S9x_MUTE), 'M'));
	item->SetMarked(s.ui.s9x_mute);
	options->AddItem(sound_menu);
	
	// options->Graphics
	graphics_menu = new BMenu("Graphics");
	graphics_menu->SetRadioMode(true);
#if 0
	graphics_menu->AddItem(new BMenuItem("Standard", new BMessage(S9x_MODE0)));
#endif
	graphics_menu->AddItem(new BMenuItem("Double", new BMessage(S9x_MODE1)));
	graphics_menu->AddItem(new BMenuItem("Scanlines", new BMessage(S9x_MODE2)));
	graphics_menu->AddItem(new BMenuItem("Lcd", new BMessage(S9x_MODE3)));
	graphics_menu->AddItem(new BMenuItem("2xSaI", new BMessage(S9x_MODE4)));
	graphics_menu->AddItem(new BMenuItem("Super2xSaI", new BMessage(S9x_MODE5)));
	graphics_menu->AddItem(new BMenuItem("SuperEagle", new BMessage(S9x_MODE6)));
	graphics_menu->AddItem(new BMenuItem("Max Scan(FullScreen only)", new BMessage(S9x_MODE7)));
	graphics_menu->ItemAt(s.ui.s9x_mode)->SetMarked(true);
	options->AddItem(graphics_menu);
	
	options->AddSeparatorItem();
	
	// Options->Controls
	BMenu *controls_menu = new BMenu("Controls");
	controls_menu->AddItem(new BMenuItem("Controller 1", new BMessage(S9x_CONTROLLER1)));
	controls_menu->AddItem(new BMenuItem("Controller 2", new BMessage(S9x_CONTROLLER2)));
	controls_menu->AddItem(new BMenuItem("Controller 3", new BMessage(S9x_CONTROLLER3)));
	controls_menu->AddItem(new BMenuItem("Controller 4", new BMessage(S9x_CONTROLLER4)));
	controls_menu->AddItem(new BMenuItem("Controller 5", new BMessage(S9x_CONTROLLER5)));
	controls_menu->AddItem(item = new BMenuItem("Joysticks Enabled", new BMessage(S9x_ALLOW)));
	item->SetMarked(s.ui.s9x_allow);
	options->AddItem(controls_menu);
	options->AddSeparatorItem();
	
	// Options->Other
	BMenu *other_menu = new BMenu("Other");
	options->AddItem(other_menu);
	
	other_menu->AddItem(new BMenuItem("Refresh Rate", new BMessage(S9x_LAUNCH_REFRESH)));
	
	other_menu->AddItem(item = new BMenuItem("Vsync", new BMessage(S9x_VSYNC)));
	item->SetMarked(s.ui.s9x_vsync);
	
	other_menu->AddItem(item = new BMenuItem("Extended Height", new BMessage(S9x_EXTENDED)));
	item->SetMarked(s.ui.s9x_extended);
	
	other_menu->AddItem(item = new BMenuItem("Snooze", new BMessage(S9x_SNOOZE)));
	item->SetMarked(s.ui.s9x_snooze);
	other_menu->AddItem(item = new BMenuItem("Pause On InActive", new BMessage(S9x_INACTIVE)));
	item->SetMarked(s.ui.s9x_inactive);

#if 0
	options->AddItem(item = new BMenuItem("Pause On MenuActive", new BMessage(S9x_MENUACTIVE)));
	item->SetMarked(s.ui.s9x_menuactive);
#endif

	// Mode Menu
	mode_menu = new BMenu("Mode");
	mode_menu->SetRadioMode(true);
	options->AddItem(mode_menu);
	
	mode_menu->AddItem(new BMenuItem("DirectWindow", new BMessage(S9x_DIRECTWINDOW)));
	mode_menu->AddItem(new BMenuItem("Overlay", new BMessage(S9x_OVERLAY)));
	mode_menu->AddItem(new BMenuItem("Bitmap", new BMessage(S9x_BITMAP)));
	mode_menu->ItemAt(s.ui.s9x_drawmode)->SetMarked(true);
	
	
	options->AddItem(new BMenuItem("FullScreen", new BMessage(S9x_FULLSCREEN), 'F'));
	
	BMenu *help = new BMenu("Help");
	help->AddItem(new BMenuItem("About BeS9x", new BMessage(S9x_ABOUT)));
	AddItem(help);
}

void GameMenuBar::set_overlay(bool what, int index)
{
	mode_menu->ItemAt(index)->SetMarked(true);
	mode_menu->ItemAt(1)->SetEnabled(what);
}

void GameMenuBar::set_items(bool ch)
{
	int count = graphics_menu->CountItems();
	for(int i = 1; i < count; i++) {
		BMenuItem *ii = graphics_menu->ItemAt(i);
		if(ii) ii->SetEnabled(ch);
	}
}

void GameMenuBar::MakeFocus(bool focus)
{
	BMenuBar::MakeFocus(focus);
	//dynamic_cast<s9x_main_window *>(Window())->MenuActivated(IsFocus());
}
