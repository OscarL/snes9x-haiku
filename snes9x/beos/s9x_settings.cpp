#include <Message.h>
#include <File.h>
#include <Path.h>
#include <string.h>
#include <FindDirectory.h>
#include <stdio.h>
#include "s9x_settings.h"
#include "s9x_key.h"

void SetDefault(s9x_settings &settings)
{
	memset(&settings, 0, sizeof(settings));
	// Ui
	settings.ui.s9x_mode = 0;
	settings.ui.s9x_mute = 0;
	settings.ui.s9x_snooze = 1;
	settings.ui.s9x_inactive = 1;
	settings.ui.s9x_allow = 0;
	settings.ui.s9x_menuactive = 0;
	settings.ui.s9x_vsync = 0;
	settings.ui.s9x_extended = 0;
	settings.ui.s9x_drawmode = 0;
	
	// Controller 1
	settings.s9x_controller1.k_up = S9X_UP;
	settings.s9x_controller1.k_down = S9X_DOWN;
	settings.s9x_controller1.k_left = S9X_LEFT;
	settings.s9x_controller1.k_right = S9X_RIGHT;
	settings.s9x_controller1.k_a = S9X_Z;
	settings.s9x_controller1.k_b = S9X_X;
	settings.s9x_controller1.k_x = S9X_C;
	settings.s9x_controller1.k_y = S9X_V;
	settings.s9x_controller1.k_l = S9X_B;
	settings.s9x_controller1.k_r = S9X_LCONTROL;
	settings.s9x_controller1.k_start = S9X_RETURN;
	settings.s9x_controller1.k_select = S9X_SPACE;
	
	// Controller 2
	settings.s9x_controller2.k_up = S9X_Y;
	settings.s9x_controller2.k_down = S9X_B;
	settings.s9x_controller2.k_left = S9X_G;
	settings.s9x_controller2.k_right = S9X_J;
	settings.s9x_controller2.k_a = S9X_B;
	settings.s9x_controller2.k_b = S9X_N;
	settings.s9x_controller2.k_x = S9X_L;
	settings.s9x_controller2.k_y = S9X_LBRACE;
	settings.s9x_controller2.k_l = S9X_RBRACE;
	settings.s9x_controller2.k_r = S9X_HASH;
	settings.s9x_controller2.k_start = S9X_M;
	settings.s9x_controller2.k_select = S9X_RSLASH;
}

void GetSettings(s9x_settings &settings)
{
	BPath path;
	BMessage snes9x;
	if(find_directory(B_USER_SETTINGS_DIRECTORY, &path, true) != B_OK)
	path.SetTo("/boot/home/config/settings");
	path.Append("_s9x_settings");
	
	BFile settings_file(path.Path(), B_READ_ONLY);
	if(settings_file.InitCheck() != B_OK) {
		SetDefault(settings);
		return;
	}
	
	snes9x.Unflatten(&settings_file);
	memset(&settings, 0, sizeof(settings));
	// Ui
	snes9x.FindInt8("s9x_mode", (int8 *)&settings.ui.s9x_mode);
	snes9x.FindInt8("s9x_mute", (int8 *)&settings.ui.s9x_mute);
	snes9x.FindInt8("s9x_snooze", (int8 *)&settings.ui.s9x_snooze);
	snes9x.FindInt8("s9x_inactive", (int8 *)&settings.ui.s9x_inactive);
	snes9x.FindInt8("s9x_allow", (int8 *)&settings.ui.s9x_allow);
	snes9x.FindInt8("s9x_menuactive", (int8 *)&settings.ui.s9x_menuactive);
	snes9x.FindInt8("s9x_vsync", (int8 *)&settings.ui.s9x_vsync);
	snes9x.FindInt8("s9x_extended", (int8 *)&settings.ui.s9x_extended);
	snes9x.FindInt8("s9x_drawmode", (int8 *)&settings.ui.s9x_drawmode);
	
	// Controller 1
	snes9x.FindInt32("k_up0", (int32 *)&settings.s9x_controller1.k_up);
	snes9x.FindInt32("k_down0", (int32 *)&settings.s9x_controller1.k_down);
	snes9x.FindInt32("k_left0", (int32 *)&settings.s9x_controller1.k_left);
	snes9x.FindInt32("k_right0", (int32 *)&settings.s9x_controller1.k_right);
	snes9x.FindInt32("k_a0", (int32 *)&settings.s9x_controller1.k_a);
	snes9x.FindInt32("k_b0", (int32 *)&settings.s9x_controller1.k_b);
	snes9x.FindInt32("k_x0", (int32 *)&settings.s9x_controller1.k_x);
	snes9x.FindInt32("k_y0", (int32 *)&settings.s9x_controller1.k_y);
	snes9x.FindInt32("k_l0", (int32 *)&settings.s9x_controller1.k_l);
	snes9x.FindInt32("k_r0", (int32 *)&settings.s9x_controller1.k_r);
	snes9x.FindInt32("k_start0", (int32 *)&settings.s9x_controller1.k_start);
	snes9x.FindInt32("k_select0", (int32 *)&settings.s9x_controller1.k_select);
	
	// Controller 2
	snes9x.FindInt32("k_up1", (int32 *)&settings.s9x_controller2.k_up);
	snes9x.FindInt32("k_down1", (int32 *)&settings.s9x_controller2.k_down);
	snes9x.FindInt32("k_left1", (int32 *)&settings.s9x_controller2.k_left);
	snes9x.FindInt32("k_right1", (int32 *)&settings.s9x_controller2.k_right);
	snes9x.FindInt32("k_a1", (int32 *)&settings.s9x_controller2.k_a);
	snes9x.FindInt32("k_b1", (int32 *)&settings.s9x_controller2.k_b);
	snes9x.FindInt32("k_x1", (int32 *)&settings.s9x_controller2.k_x);
	snes9x.FindInt32("k_y1", (int32 *)&settings.s9x_controller2.k_y);
	snes9x.FindInt32("k_l1", (int32 *)&settings.s9x_controller2.k_l);
	snes9x.FindInt32("k_r1", (int32 *)&settings.s9x_controller2.k_r);
	snes9x.FindInt32("k_start1", (int32 *)&settings.s9x_controller2.k_start);
	snes9x.FindInt32("k_select1", (int32 *)&settings.s9x_controller2.k_select);
	
	// Controller 3
	snes9x.FindInt32("k_up2", (int32 *)&settings.s9x_controller3.k_up);
	snes9x.FindInt32("k_down2", (int32 *)&settings.s9x_controller3.k_down);
	snes9x.FindInt32("k_left2", (int32 *)&settings.s9x_controller3.k_left);
	snes9x.FindInt32("k_right2", (int32 *)&settings.s9x_controller3.k_right);
	snes9x.FindInt32("k_a2", (int32 *)&settings.s9x_controller3.k_a);
	snes9x.FindInt32("k_b2", (int32 *)&settings.s9x_controller3.k_b);
	snes9x.FindInt32("k_x2", (int32 *)&settings.s9x_controller3.k_x);
	snes9x.FindInt32("k_y2", (int32 *)&settings.s9x_controller3.k_y);
	snes9x.FindInt32("k_l2", (int32 *)&settings.s9x_controller3.k_l);
	snes9x.FindInt32("k_r2", (int32 *)&settings.s9x_controller3.k_r);
	snes9x.FindInt32("k_start2", (int32 *)&settings.s9x_controller3.k_start);
	snes9x.FindInt32("k_select2", (int32 *)&settings.s9x_controller3.k_select);
	
	// Controller 4
	snes9x.FindInt32("k_up3", (int32 *)&settings.s9x_controller4.k_up);
	snes9x.FindInt32("k_down3", (int32 *)&settings.s9x_controller4.k_down);
	snes9x.FindInt32("k_left3", (int32 *)&settings.s9x_controller4.k_left);
	snes9x.FindInt32("k_right3", (int32 *)&settings.s9x_controller4.k_right);
	snes9x.FindInt32("k_a3", (int32 *)&settings.s9x_controller4.k_a);
	snes9x.FindInt32("k_b3", (int32 *)&settings.s9x_controller4.k_b);
	snes9x.FindInt32("k_x3", (int32 *)&settings.s9x_controller4.k_x);
	snes9x.FindInt32("k_y3", (int32 *)&settings.s9x_controller4.k_y);
	snes9x.FindInt32("k_l3", (int32 *)&settings.s9x_controller4.k_l);
	snes9x.FindInt32("k_r3", (int32 *)&settings.s9x_controller4.k_r);
	snes9x.FindInt32("k_start3", (int32 *)&settings.s9x_controller4.k_start);
	snes9x.FindInt32("k_select3", (int32 *)&settings.s9x_controller4.k_select);
	
	// Controller 5
	snes9x.FindInt32("k_up4", (int32 *)&settings.s9x_controller5.k_up);
	snes9x.FindInt32("k_down4", (int32 *)&settings.s9x_controller5.k_down);
	snes9x.FindInt32("k_left4", (int32 *)&settings.s9x_controller5.k_left);
	snes9x.FindInt32("k_right4", (int32 *)&settings.s9x_controller5.k_right);
	snes9x.FindInt32("k_a4", (int32 *)&settings.s9x_controller5.k_a);
	snes9x.FindInt32("k_b4", (int32 *)&settings.s9x_controller5.k_b);
	snes9x.FindInt32("k_x4", (int32 *)&settings.s9x_controller5.k_x);
	snes9x.FindInt32("k_y4", (int32 *)&settings.s9x_controller5.k_y);
	snes9x.FindInt32("k_l4", (int32 *)&settings.s9x_controller5.k_l);
	snes9x.FindInt32("k_r4", (int32 *)&settings.s9x_controller5.k_r);
	snes9x.FindInt32("k_start4", (int32 *)&settings.s9x_controller5.k_start);
	snes9x.FindInt32("k_select4", (int32 *)&settings.s9x_controller5.k_select);
	
	snes9x.Flatten(&settings_file);
}

static inline status_t SaveInt8(BMessage &msg, const char *string, int8 data)
{
	if(msg.ReplaceInt8(string, data) == B_NAME_NOT_FOUND)
		return msg.AddInt8(string, data);
	return msg.ReplaceInt8(string, data);
}

static inline status_t SaveInt32(BMessage &msg, const char *string, int32 data)
{
	if(msg.ReplaceInt32(string, data) == B_NAME_NOT_FOUND)
		return msg.AddInt32(string, data);
	return msg.ReplaceInt32(string, data);
}

void SetSettings(const s9x_settings &settings)
{
	BPath path;
	BMessage snes9x;
	if(find_directory(B_USER_SETTINGS_DIRECTORY, &path, true) != B_OK)
	path.SetTo("/boot/home/config/settings");
	path.Append("_s9x_settings");
	
	BFile settings_file(path.Path(), B_READ_WRITE | B_CREATE_FILE);
	snes9x.Unflatten(&settings_file);
	
	// Ui
	SaveInt8(snes9x, "s9x_mode", settings.ui.s9x_mode);
	SaveInt8(snes9x, "s9x_mute", settings.ui.s9x_mute);
	SaveInt8(snes9x, "s9x_snooze", settings.ui.s9x_snooze);
	SaveInt8(snes9x, "s9x_inactive", settings.ui.s9x_inactive);
	SaveInt8(snes9x, "s9x_allow", settings.ui.s9x_allow);
	SaveInt8(snes9x, "s9x_menuactive", settings.ui.s9x_menuactive);
	SaveInt8(snes9x, "s9x_vsync", settings.ui.s9x_vsync);
	SaveInt8(snes9x, "s9x_extended", settings.ui.s9x_extended);
	SaveInt8(snes9x, "s9x_drawmode", settings.ui.s9x_drawmode);
	
	// Controller 1
	SaveInt32(snes9x, "k_up0", settings.s9x_controller1.k_up);
	SaveInt32(snes9x, "k_down0", settings.s9x_controller1.k_down);
	SaveInt32(snes9x, "k_left0", settings.s9x_controller1.k_left);
	SaveInt32(snes9x, "k_right0", settings.s9x_controller1.k_right);
	SaveInt32(snes9x, "k_a0", settings.s9x_controller1.k_a);
	SaveInt32(snes9x, "k_b0", settings.s9x_controller1.k_b);
	SaveInt32(snes9x, "k_x0", settings.s9x_controller1.k_x);
	SaveInt32(snes9x, "k_y0", settings.s9x_controller1.k_y);
	SaveInt32(snes9x, "k_l0", settings.s9x_controller1.k_l);
	SaveInt32(snes9x, "k_r0", settings.s9x_controller1.k_r);
	SaveInt32(snes9x, "k_start0", settings.s9x_controller1.k_start);
	SaveInt32(snes9x, "k_select0", settings.s9x_controller1.k_select);
	
	// Controller 2
	SaveInt32(snes9x, "k_up1", settings.s9x_controller2.k_up);
	SaveInt32(snes9x, "k_down1", settings.s9x_controller2.k_down);
	SaveInt32(snes9x, "k_left1", settings.s9x_controller2.k_left);
	SaveInt32(snes9x, "k_right1", settings.s9x_controller2.k_right);
	SaveInt32(snes9x, "k_a1", settings.s9x_controller2.k_a);
	SaveInt32(snes9x, "k_b1", settings.s9x_controller2.k_b);
	SaveInt32(snes9x, "k_x1", settings.s9x_controller2.k_x);
	SaveInt32(snes9x, "k_y1", settings.s9x_controller2.k_y);
	SaveInt32(snes9x, "k_l1", settings.s9x_controller2.k_l);
	SaveInt32(snes9x, "k_r1", settings.s9x_controller2.k_r);
	SaveInt32(snes9x, "k_start1", settings.s9x_controller2.k_start);
	SaveInt32(snes9x, "k_select1", settings.s9x_controller2.k_select);
	
	// Controller 3
	SaveInt32(snes9x, "k_up2", settings.s9x_controller3.k_up);
	SaveInt32(snes9x, "k_down2", settings.s9x_controller3.k_down);
	SaveInt32(snes9x, "k_left2", settings.s9x_controller3.k_left);
	SaveInt32(snes9x, "k_right2", settings.s9x_controller3.k_right);
	SaveInt32(snes9x, "k_a2", settings.s9x_controller3.k_a);
	SaveInt32(snes9x, "k_b2", settings.s9x_controller3.k_b);
	SaveInt32(snes9x, "k_x2", settings.s9x_controller3.k_x);
	SaveInt32(snes9x, "k_y2", settings.s9x_controller3.k_y);
	SaveInt32(snes9x, "k_l2", settings.s9x_controller3.k_l);
	SaveInt32(snes9x, "k_r2", settings.s9x_controller3.k_r);
	SaveInt32(snes9x, "k_start2", settings.s9x_controller3.k_start);
	SaveInt32(snes9x, "k_select2", settings.s9x_controller3.k_select);
	
	// Controller 4
	SaveInt32(snes9x, "k_up3", settings.s9x_controller4.k_up);
	SaveInt32(snes9x, "k_down3", settings.s9x_controller4.k_down);
	SaveInt32(snes9x, "k_left3", settings.s9x_controller4.k_left);
	SaveInt32(snes9x, "k_right3", settings.s9x_controller4.k_right);
	SaveInt32(snes9x, "k_a3", settings.s9x_controller4.k_a);
	SaveInt32(snes9x, "k_b3", settings.s9x_controller4.k_b);
	SaveInt32(snes9x, "k_x3", settings.s9x_controller4.k_x);
	SaveInt32(snes9x, "k_y3", settings.s9x_controller4.k_y);
	SaveInt32(snes9x, "k_l3", settings.s9x_controller4.k_l);
	SaveInt32(snes9x, "k_r3", settings.s9x_controller4.k_r);
	SaveInt32(snes9x, "k_start3", settings.s9x_controller4.k_start);
	SaveInt32(snes9x, "k_select3", settings.s9x_controller4.k_select);
	
	// Controller 5
	SaveInt32(snes9x, "k_up4", settings.s9x_controller5.k_up);
	SaveInt32(snes9x, "k_down4", settings.s9x_controller5.k_down);
	SaveInt32(snes9x, "k_left4", settings.s9x_controller5.k_left);
	SaveInt32(snes9x, "k_right4", settings.s9x_controller5.k_right);
	SaveInt32(snes9x, "k_a4", settings.s9x_controller5.k_a);
	SaveInt32(snes9x, "k_b4", settings.s9x_controller5.k_b);
	SaveInt32(snes9x, "k_x4", settings.s9x_controller5.k_x);
	SaveInt32(snes9x, "k_y4", settings.s9x_controller5.k_y);
	SaveInt32(snes9x, "k_l4", settings.s9x_controller5.k_l);
	SaveInt32(snes9x, "k_r4", settings.s9x_controller5.k_r);
	SaveInt32(snes9x, "k_start4", settings.s9x_controller5.k_start);
	SaveInt32(snes9x, "k_select4", settings.s9x_controller5.k_select);
	
	settings_file.SetSize(0);
	settings_file.Seek(0, SEEK_SET);
	snes9x.Flatten(&settings_file);
}

