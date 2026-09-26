#ifndef __SNES9X_SETTINGS_H__
#define __SNES9X_SETTINGS_H__

struct s9x_ui
{
	uint8 s9x_mode, s9x_mute, s9x_snooze, s9x_inactive, s9x_allow,
		s9x_menuactive, s9x_vsync, s9x_extended, s9x_drawmode;
};

struct s9x_keys
{
	uint32 k_up, k_down, k_left, k_right, k_a, k_b, k_x, k_y, k_l,
		k_r, k_start, k_select;
};

struct s9x_settings
{
	s9x_ui ui;
	s9x_keys s9x_controller1;
	s9x_keys s9x_controller2;
	s9x_keys s9x_controller3;
	s9x_keys s9x_controller4;
	s9x_keys s9x_controller5;
};

void GetSettings(s9x_settings &settings);
void SetSettings(const s9x_settings &settings);

#endif
