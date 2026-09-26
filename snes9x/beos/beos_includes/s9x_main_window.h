#ifndef __SNES9X_MAIN_WINDOW_H__
#define __SNES9X_MAIN_WINDOW_H__

#include <Application.h>
#include <DirectWindow.h>
#include <Screen.h>
#include <Locker.h>
#include <Bitmap.h>
#include <Path.h>
#include <Entry.h>
#include <FilePanel.h>
#include "s9x_view.h"
#include "Gamestick.h"
#include "input_interface.h"
#include "s9x_settings.h"

class s9x_config_window;
class s9x_refresh_window;
class GameMenuBar;

int32 _fThread(void *o);
enum {
	DIRECT, OVERLAY, BITMAP
};

struct direct_info {
	int x;
	int y;
	int w;
	int h;
};

class s9x_main_window : public BDirectWindow
{
public:
	s9x_main_window(BRect);
	virtual ~s9x_main_window();
	virtual bool QuitRequested() { be_app->PostMessage(B_QUIT_REQUESTED); return(true); }
	virtual void MessageReceived(BMessage *m);
	virtual void DirectConnected(direct_buffer_info *info);
	virtual void WindowActivated(bool active);
	virtual void UpdateMouse(uint32 buttons);
	virtual void UpdateMousePosition(int x, int y);
	virtual void S9xMouseOn();
	virtual void ChangeControllers();
	virtual void init_s9x();
	virtual void ScreenChanged(BRect frame, color_space mode);
	virtual void ShowAlert();
	virtual void InitMenuBar();
	virtual void DrawDirect(int Width, int Height);
	virtual void DrawOverlay(int Width, int Height);
	virtual void DrawStandard(int Width, int Height);
	virtual bool start_s9x();
	virtual void s9x_states(int index, bool saving = true);
	virtual void SetGraphicsMode(int g_mode, bool clear = true);
	virtual void SetDrawMode(int d_mode);
	virtual void DirectClear(bool clear);
	virtual void ResizeTo(float width, float height);
	virtual void ClearScreen();
	virtual BBitmap *Screen() { return &off_screen; }
	virtual BBitmap *SubScreen() { return &sub_screen; }
	int32 oThread();
	virtual void DePause();
	const entry_ref *Ref() { return &mRef; }
	virtual void ProcessEvents();
	virtual int check_key(int code);
	virtual void s9x_reset();
	virtual void MenuActivated(bool active);
	static int32 key_thread(void *o);
	virtual void set_flags(int flags);
	virtual void change_item_bool(BMessage *m, uint8 &ch);
	void render_normal();
	void render_2xsai();
	void render_s2xsai();
	void render_seagle();
	void render_multi(int w);
	void render_scan_or_double(const int type, bool scanlines);
	void render_lcd(const int type);
	void set_direct_info();
private:
	int get_height();
	s9x_view *s9x_mview;
	s9x_config_window *config_window;
	s9x_refresh_window *refresh_window;
	GameStick *gamestick;
	int32 menubar_height;
	GameMenuBar *g_menu;
	input_interface key[512];
	display_mode prev_mode, new_mode, cur_mode;
	bool mode_is_valid;
	BFilePanel *s9x_filepanel;
	
	// DirectWindow variables
	BLocker *m_locker;
	int32 m_bitsperpixel;
	int32 m_rowbytes;
	color_space m_format;
	uint32 m_numcliprects;
	clipping_rect *m_cliplist;
	clipping_rect m_bounds;
	uint8 *m_bits;
	bool m_connected;
	
	thread_id thread;
	bool ispaused;
	volatile bool exiting;
	volatile bool needs_clear;
	entry_ref mRef;
	BBitmap off_screen;
	BBitmap sub_screen;
	BBitmap *ov_bitmap;
	thread_id key_id;
	direct_info *direct;
};

#endif
