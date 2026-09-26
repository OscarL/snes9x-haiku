#include "s9x_includes.h"
#include "s9x_flags.h"

volatile uint32 FrameTimer = 0;
uint8 snes9x_clear_change_log = 0; // used inside 2xsai asm
uint32 snes9x_is_32 = 0; // used inside 2xsai asm
uint32 joypads [5] = { 0 };
static BSoundPlayer *s9x_player = 0;
static s9x_main_window *s9x = 0;
static uint8 *Delta = 0;
static thread_id TimerID = -1;
static int32 s9x_timer(void *o);
s9x_settings settings;
static BLocker sound_locker;
static BLocker main_locker;

s9x_main_window::s9x_main_window(BRect rect)
	: BDirectWindow(rect, "Snes9x", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE),
	config_window(0),
	refresh_window(0),
	gamestick(0),
	menubar_height(0),
	mode_is_valid(false),
	s9x_filepanel(0),
	m_locker(new BLocker()),
	m_bitsperpixel(0),
	m_rowbytes(0),
	m_format(B_NO_COLOR_SPACE),
	m_numcliprects(0),
	m_cliplist(0),
	m_bits(0),
	m_connected(false),
	ispaused(false),
	exiting(true),
	needs_clear(false),
	off_screen(BRect(0, 0, 511, 477), B_RGB16, false, true),
	sub_screen(BRect(0, 0, 511, 477), B_RGB16, false, true),
	ov_bitmap(0),
	direct(0)
{
	GetSettings(settings);
	this->gamestick = new GameStick(this->key);
	
	rect.OffsetTo(B_ORIGIN);
	InitMenuBar();
	
	rect.top = (float)menubar_height + 1;
	s9x_mview = new s9x_view(rect, this);
	s9x_mview->SetViewColor(0, 0, 0);
	AddChild(s9x_mview);
	
	AddShortcut('F', B_COMMAND_KEY, new BMessage(S9x_FULLSCREEN));
	AddShortcut('1', B_COMMAND_KEY, new BMessage(S9x_FRAME_RATE));
	AddShortcut('7', B_COMMAND_KEY, new BMessage(S9x_CHANGE_CONTROLLER));
	
	SetWindowAlignment(B_BYTE_ALIGNMENT, 8);
	
	display_mode *m_mode;
	uint32 m_nummodes;
	
	if(BScreen().GetModeList(&m_mode, &m_nummodes) == B_NO_ERROR)
	{
		for (unsigned int i = 0; i < m_nummodes; i++)
		{
			if((m_mode[i].virtual_width == 640) && (m_mode[i].space == B_RGB16))
			{
				mode_is_valid = true;
				new_mode = m_mode[i];
				break;
			}
		}
	}
	BScreen().GetMode(&prev_mode);
	
	TimerID = spawn_thread(s9x_timer, "FrameTimer",
		B_NORMAL_PRIORITY, this);
	resume_thread(TimerID);
	
	key_id = spawn_thread(key_thread, "let's get keys",
		B_NORMAL_PRIORITY, this);
	resume_thread(key_id);
	
	s9x_filepanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this),
		NULL, B_FILE_NODE, false);

	init_s9x();
	init_palette();
	ov_bitmap = new BBitmap(BRect(0, 0, 511, 223), B_BITMAP_WILL_OVERLAY, B_YCbCr422);
	
	if(ov_bitmap && ov_bitmap->InitCheck() == B_OK)
	{
		if(settings.ui.s9x_drawmode == OVERLAY) SetDrawMode(settings.ui.s9x_drawmode);
		overlay_restrictions r;
		ov_bitmap->GetOverlayRestrictions(&r);
		SetSizeLimits(r.source.max_width - 1, r.source.max_width * r.max_width_scale,
			(r.source.max_height + menubar_height), r.source.max_height * r.max_height_scale);
	} else {
		if(settings.ui.s9x_drawmode == OVERLAY) settings.ui.s9x_drawmode = DIRECT;
		g_menu->set_overlay(false, settings.ui.s9x_drawmode);
	}
	
	float minw, maxw, minh, maxh;
	GetSizeLimits(&minw, &maxw, &minh, &maxh);
	
	SetSizeLimits(511, maxw, (447 + menubar_height), maxh);
	ResizeTo((SNES_WIDTH * 2) - 1, (get_height() * 2) - 1);
}

void s9x_main_window::set_flags(int flags)
{
	::set_flags(this, flags);
}

bool s9x_main_window::start_s9x()
{
	int32 result;
	exiting = true;
	wait_for_thread(thread, &result);
	if(s9x_player) {
		s9x_player->Stop();
		delete s9x_player;
		s9x_player = 0;
	}
	exiting = false;
	DePause();
	thread = spawn_thread(fThread, "Snes9x", B_NORMAL_PRIORITY, (void *)this);
	resume_thread(thread);
	return true;
}

int xx = 0, yy = 0;
uint32 button_d = 0;
void s9x_main_window::UpdateMouse(uint32 buttons)
{
	button_d = buttons;
}

void s9x_main_window::UpdateMousePosition(int x, int y)
{
	static bool ignore_next = false;
	
	if(!ignore_next)
	{
		BPoint p(x, y);
		s9x_mview->ConvertToScreen(&p);
		if(IPPU.Controller == SNES_MOUSE ||
                     IPPU.Controller == SNES_MOUSE_SWAPPED)
        {
			BPoint middle;
			BRect size = Bounds();
			middle.x = (size.right - size.left) / 2;
			middle.y = (size.bottom - size.top) / 2;
			s9x_mview->ConvertToScreen(&middle);
		
			xx += (int)(p.x - middle.x);
			yy += (int)(p.y - middle.y);
		
			set_mouse_position((int)middle.x, (int)middle.y);
			ignore_next = true;
		}
		else
		{
			xx = (int)x / 2;
			yy = (int)y / 2;
		}
	}
	else
		ignore_next = false;
		
	S9xMouseOn();
}

void s9x_main_window::init_s9x()
{
	memset(&Settings, 0, sizeof(Settings));
	Settings.SoundPlaybackRate = 7;
    Settings.Stereo = TRUE;
	Settings.SoundEnvelopeHeightReading = TRUE;
	Settings.InterpolatedSound = TRUE;
    Settings.SoundBufferSize = 2048;
    Settings.CyclesPercentage = 100;
    Settings.DisableSoundEcho = FALSE;
    Settings.APUEnabled = Settings.NextAPUEnabled = true;
    Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
    Settings.SkipFrames = AUTO_FRAMERATE;
    Settings.ShutdownMaster = TRUE;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    Settings.FrameTime = Settings.FrameTimeNTSC;
    Settings.DisableSampleCaching = FALSE;
    Settings.DisableMasterVolume = FALSE;
    Settings.Mouse = TRUE;
    Settings.SuperScope = TRUE;
    Settings.MultiPlayer5 = TRUE;
    Settings.ControllerOption = SNES_MULTIPLAYER5;
    Settings.Transparency = TRUE;
    Settings.ForceTransparency = TRUE;
    Settings.SixteenBit = TRUE;
    Settings.SupportHiRes = TRUE;
    Settings.NetPlay = FALSE;
    Settings.ServerName [0] = 0;
    Settings.ThreadSound = FALSE;
    Settings.AutoSaveDelay = 30;
    Settings.DisplayFrameRate = false;
    Settings.JoystickEnabled = 1;
#ifdef _NETPLAY_SUPPORT
    Settings.Port = NP_DEFAULT_PORT;
#endif
    Settings.ApplyCheats = TRUE;
    Settings.TurboMode = FALSE;
    Settings.TurboSkipFrames = 15;
    Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
}

void s9x_main_window::S9xMouseOn()
{
	if (IPPU.Controller == SNES_MOUSE || IPPU.Controller == SNES_MOUSE_SWAPPED)
    {
    	if(!be_app->IsCursorHidden()) be_app->HideCursor();
    	s9x_mview->ChangeCursor(0);
    }
    else if (IPPU.Controller != SNES_SUPERSCOPE&&IPPU.Controller != SNES_JUSTIFIER&&IPPU.Controller != SNES_JUSTIFIER_2)
    {
    	if(be_app->IsCursorHidden()) be_app->ShowCursor();
    	s9x_mview->ChangeCursor(0);
    }
    else
    {
    	if(be_app->IsCursorHidden()) be_app->ShowCursor();
    	s9x_mview->ChangeCursor(1);
    }
}

void S9xInitDisplay()
{
	Init_2xSaI (565);
	
	memset(s9x->Screen()->Bits(), 0, s9x->Screen()->BitsLength());
	memset(s9x->SubScreen()->Bits(), 0, s9x->SubScreen()->BitsLength());
	
    GFX.Screen = (uint8 *)s9x->Screen()->Bits();
    GFX.Pitch = s9x->Screen()->BytesPerRow();
	Delta = new uint8 [(IMAGE_WIDTH + 2) * IMAGE_HEIGHT * 2];
	GFX.SubScreen = (uint8 *)s9x->SubScreen()->Bits();
	GFX.ZBuffer = new uint8[((int)(SCREEN_WIDTH) * IMAGE_HEIGHT)];
	GFX.SubZBuffer = new uint8[((int)(SCREEN_WIDTH) * IMAGE_HEIGHT)];
}

void OutOfMemory ()
{
	Memory.Deinit ();
    S9xDeinitAPU ();
}

void InitTimer()
{
	FrameTimer = 0;
}

int32 fThread(void *o)
{
	s9x_main_window *w = (s9x_main_window *)o;
	s9x = w;
	return w->oThread();
}

int32 s9x_main_window::oThread()
{
	if(GFX.ZBuffer) S9xExit();
	BEntry entry(Ref(), true);
	BPath path;
	entry.GetPath(&path);
	char *rom_filename = 0;
	
    rom_filename = strdup(path.Path());

    if (!Memory.Init () || !S9xInitAPU()) {
		OutOfMemory ();
		exiting = true;
		return 0;
	}
    uint32 saved_flags = CPU.Flags;

#ifdef GFX_MULTI_FORMAT
    S9xSetRenderPixelFormat (RGB565);
#endif

    if (rom_filename)
    {
	if (!Memory.LoadROM (rom_filename))
	{
		exiting = true;
		return(0);
	}
	Memory.LoadSRAM (S9xGetFilename (".srm"));
    }
    else
    {
		S9xReset ();
		Settings.Paused |= 2;
    }
    CPU.Flags = saved_flags;

    S9xInitDisplay ();
    if (!S9xGraphicsInit ())
    {
    	exiting = true;
		OutOfMemory ();
		return 0;
	}

    (void) S9xInitSound (Settings.SoundPlaybackRate, Settings.Stereo,
		      Settings.SoundBufferSize);
		      
    if (!Settings.APUEnabled) S9xSetSoundMute (TRUE);

	char buf[513];
    sprintf (buf, "\"%s\" %s: %s", Memory.ROMName, TITLE, VERSION);
    S9xSetTitle (buf);
    
    InitTimer ();
    if (!Settings.APUEnabled) S9xSetSoundMute (FALSE);

    while (!exiting)
    {
    if(exiting) break;
    main_locker.Lock();
    
	if (!Settings.Paused
#ifdef DEBUGGER
	    || (CPU.Flags & (DEBUG_MODE_FLAG | SINGLE_STEP_FLAG))
#endif
           )
	    S9xMainLoop ();

	if (Settings.Paused
#ifdef DEBUGGER
	    || (CPU.Flags & DEBUG_MODE_FLAG)
#endif
           )
	{
	    S9xSetSoundMute (TRUE);
	}

#ifdef DEBUGGER
	if (CPU.Flags & DEBUG_MODE_FLAG)
	{
	    S9xDoDebug ();
	}
	else
#endif
	if (Settings.Paused)
	    S9xProcessEvents (TRUE);

	S9xProcessEvents (FALSE);
	
	if (!Settings.Paused
#ifdef DEBUGGER
	    && !(CPU.Flags & DEBUG_MODE_FLAG)
#endif	    
           )
	{
	    S9xSetSoundMute (FALSE);
	}
	main_locker.Unlock();
	if(Settings.Paused) snooze(16000);
    }
    return (0);
}

void s9x_main_window::InitMenuBar()
{
	g_menu = new GameMenuBar(BRect(0, 0, 0, 0), "menu_bar", settings);
	AddChild(g_menu);
	menubar_height = (int32)g_menu->Bounds().Height();
	g_menu->set_items(settings.ui.s9x_drawmode == DIRECT);
}

void s9x_main_window::s9x_reset()
{
	main_locker.Lock();
	sound_locker.Lock();
	DePause();
	S9xReset();
	main_locker.Unlock();
    sound_locker.Unlock();
}

void s9x_main_window::s9x_states(int index, bool saving)
{
	char def [PATH_MAX + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char ext [_MAX_EXT + 1];
	DePause();
    _splitpath (Memory.ROMFilename, drive, dir, def, ext);
    sprintf(ext, ".00%d", index);
    strcat (def, ext);
  
	main_locker.Lock();
	sound_locker.Lock();
	S9xSetSoundMute (TRUE);
	
	if(saving ? Snapshot(def) : S9xLoadSnapshot(def))
	{
		saving ? sprintf (String, "Snapshot saved as: %.255s", def)
			: sprintf (String, "Snapshot Loaded: %.255s", def);
		S9xMessage(0, 0, String);
	} else
	{
		saving ? sprintf (String, "Error, unable to save: %.255s", def)
			: sprintf (String, "Error, unable to load: %.255s", def);
		S9xMessage(0, 0, String);
	}
    
    S9xSetSoundMute (FALSE);
    main_locker.Unlock();
    sound_locker.Unlock();
}

void s9x_main_window::change_item_bool(BMessage *m, uint8 &value)
{
	value = !value;
	BMenuItem *item;
	m->FindPointer("source", (void **)&item);
	item->SetMarked(value);
}

void s9x_main_window::MessageReceived(BMessage *m)
{
	int32 index, slot;
	switch(m->what) {
		case B_KEY_DOWN:
			if(m->FindInt32("key", &index) == B_NO_ERROR) {
				if(index == S9X_ESCAPE) PostMessage(B_QUIT_REQUESTED);
			}
			break;
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
			if(m->FindRef("refs", &mRef) == B_NO_ERROR) start_s9x();
			break;
		case S9x_FULLSCREEN:
			if(!mode_is_valid) break;
			SetFullScreen(!IsFullScreen());
			if(!IsFullScreen()) {
				AddChild(g_menu);
				s9x_mview->MoveTo(0, menubar_height + 1);
				s9x_mview->ResizeBy(0, -menubar_height - 1);
				BScreen().SetMode(&prev_mode);
			}
			if(IsFullScreen()) {
				RemoveChild(g_menu);
				s9x_mview->MoveTo(B_ORIGIN);
				s9x_mview->ResizeBy(0, menubar_height + 1);
				BScreen().SetMode(&new_mode);
				memset(m_bits, 0, ((int32)Bounds().bottom*m_rowbytes));
				if(!needs_clear) needs_clear = true;
			}
			break;
		case S9x_SAVE_SPC:
			if(!exiting) spc_is_dumping = 1;
			break;
		case S9x_SNOOZE:
			change_item_bool(m, settings.ui.s9x_snooze);
			break;
		case S9x_INACTIVE:
			change_item_bool(m, settings.ui.s9x_inactive);
			break;
		case S9x_MENUACTIVE:
			change_item_bool(m, settings.ui.s9x_menuactive);
			break;
		case S9x_ALLOW:
			change_item_bool(m, settings.ui.s9x_allow);
			break;
		case S9x_VSYNC:
			change_item_bool(m, settings.ui.s9x_vsync);
			break;
		case S9x_MUTE:
			change_item_bool(m, settings.ui.s9x_mute);
			break;
		case S9x_EXTENDED:
			change_item_bool(m, settings.ui.s9x_extended);
			SetGraphicsMode(settings.ui.s9x_mode);
			break;
		case S9x_ROMINFO:
			if(exiting) break;
			new s9x_rom_window(BRect(200, 200, 500, 500));
			break;
		case S9x_SCREENSHOT:
			save_screenshot(off_screen, SNES_WIDTH, SNES_HEIGHT);
			break;
		case S9x_LAUNCH_REFRESH:
			if(refresh_window != 0) break;
			refresh_window = new s9x_refresh_window(BRect(200, 200, 380, 300), this, 'redl');
			break;
		case 'redl':
			refresh_window = 0;
			break;
		case 'refr': {
			display_mode *mode;
			if(m->FindPointer("refr_rate", (void **)&mode) == B_NO_ERROR) new_mode = *mode;
		} break;
		case S9x_DIRECTWINDOW:
		case S9x_OVERLAY:
		case S9x_BITMAP:
			m->FindInt32("index", &index);
			SetDrawMode(index);
			break;
		case S9x_CHANGE_CONTROLLER:
			if(exiting) break;
			ChangeControllers();
			break;
		case S9x_FRAME_RATE:
			Settings.DisplayFrameRate = !Settings.DisplayFrameRate;
			break;
		case S9x_PAUSE:
			change_item_bool(m, Settings.Paused);
			ispaused = Settings.Paused;
			break;
		case S9x_LOAD:
			s9x_filepanel->Show();
			break;
		case S9x_CONTROLLER1:
			if(config_window != 0) break;
			config_window = new s9x_config_window("S9x - Controller 1", (uint32 *)&settings.s9x_controller1, this,
				S9x_KEY_QUIT, key, settings.ui.s9x_allow);
			break;
		case S9x_CONTROLLER2:
			if(config_window != 0) break;
			config_window = new s9x_config_window("S9x - Controller 2",(uint32 *)&settings.s9x_controller2, this,
				S9x_KEY_QUIT, key, settings.ui.s9x_allow);
			break;
		case S9x_CONTROLLER3:
			if(config_window != 0) break;
			config_window = new s9x_config_window("S9x - Controller 3",(uint32 *)&settings.s9x_controller3, this,
				S9x_KEY_QUIT, key, settings.ui.s9x_allow);
			break;
		case S9x_CONTROLLER4:
			if(config_window != 0) break;
			config_window = new s9x_config_window("S9x - Controller 4", (uint32 *)&settings.s9x_controller4, this,
				S9x_KEY_QUIT, key, settings.ui.s9x_allow);
			break;
		case S9x_CONTROLLER5:
			if(config_window != 0) break;
			config_window = new s9x_config_window("S9x - Controller 5", (uint32 *)&settings.s9x_controller5, this,
				S9x_KEY_QUIT, key, settings.ui.s9x_allow);
			break;
		case S9x_KEY_QUIT:
			config_window = 0;
			break;
		case S9x_RESET:
			if(!exiting) s9x_reset();
			break;
		case S9x_SAVE_STATE:
			if(exiting) break;
			m->FindInt32("save_slot", &slot);
			s9x_states(slot);
			break;
		case S9x_LOAD_STATE:
			if(exiting) break;
			m->FindInt32("load_slot", &slot);
			s9x_states(slot, false);
			break;
		case S9x_EXIT:
			PostMessage(B_QUIT_REQUESTED);
			break;
		case S9x_MODE0:
		case S9x_MODE1:
		case S9x_MODE2:
		case S9x_MODE3:
		case S9x_MODE4:
		case S9x_MODE5:
		case S9x_MODE6:
		case S9x_MODE7:
			m->FindInt32("index", &index);
			SetGraphicsMode(index);
			break;
		case S9x_ABOUT:
			ShowAlert();
			break;
		default:
			BDirectWindow::MessageReceived(m);
			break;
	}
}

void s9x_main_window::SetDrawMode(int d_mode)
{
	main_locker.Lock();
	sound_locker.Lock();
	settings.ui.s9x_drawmode = d_mode;
	
	s9x_mview->ClearViewOverlay();
	if(settings.ui.s9x_drawmode == OVERLAY) {
		rgb_color col;
		s9x_mview->SetViewOverlay(ov_bitmap, ov_bitmap->Bounds(), s9x_mview->Bounds(), &col,
				B_FOLLOW_ALL, B_OVERLAY_FILTER_HORIZONTAL | B_OVERLAY_FILTER_VERTICAL);
		s9x_mview->SetViewColor(col);
		clear_ycbr((uint8 *)ov_bitmap->Bits(), ov_bitmap->Bounds().IntegerHeight() + 1,
			ov_bitmap->BytesPerRow());
		set_flags(B_NOT_ZOOMABLE);
	} else {
		s9x_mview->SetViewColor(0, 0, 0);
		SetGraphicsMode(settings.ui.s9x_mode, false);
		set_flags(B_NOT_ZOOMABLE | B_NOT_RESIZABLE);
	}
	
	s9x_mview->Invalidate();
	g_menu->set_items(settings.ui.s9x_drawmode == DIRECT);
	main_locker.Unlock();
	sound_locker.Unlock();
}

void s9x_main_window::ShowAlert()
{
	BString text;
	text << "BeS9x " << VERSION"(All-in-Wonder revision)(Super Nes Emulator)" << "\n\n" << "Emulation Core: \nby The Snes9x team(http://www.snes9x.com)\n" << "\nBeOS version: \nby Caz (Carwyn Jones)\n"
	<< "\nThanks to: \nChristian Corona - for his excellent feedback and bug reports, creating the icon for BeS9x and many other interface ideas.\n"
	<< "The Snes9x team for creating a superb Snes emulator.\nDerek Liauw Kie Fa for his excellent 2xSaI graphics routines.\nGoodOldGames - testing and ideas\nJack Burton - testing and ideas\n"
	<< "Jess Tipton - bug reports/feedback\nAlienSoldier - bug reports/feedback";
	(new BAlert("snes9x", text.String(), "Ok", NULL, NULL, B_WIDTH_FROM_LABEL))->Go(NULL);
}

void s9x_main_window::ChangeControllers()
{
	static char *controllers [] = {
		"Multiplayer 5 on #0", "Joypad on #0", "Mouse on #1",
			"Mouse on #0", "Superscope on #1", "Justifier 1 on #1", "Justifier 2 on #1"
	};
	S9xNextController ();
	S9xSetInfoString (controllers [IPPU.Controller]);
	
	bool needs_mouse = (IPPU.Controller == SNES_SUPERSCOPE || IPPU.Controller == SNES_MOUSE ||
			IPPU.Controller == SNES_MOUSE_SWAPPED);
	s9x_mview->SetNeedsMouse(needs_mouse);
	S9xMouseOn();
}

int s9x_main_window::get_height()
{
	if(settings.ui.s9x_extended) return SNES_HEIGHT_EXTENDED;
	return SNES_HEIGHT;
}

void s9x_main_window::DirectConnected(direct_buffer_info *info)
{
	uint32 j;
	
	switch(info->buffer_state & B_DIRECT_MODE_MASK) {
		case B_DIRECT_START:
		case B_DIRECT_MODIFY:
			m_locker->Lock();
			if (m_cliplist) {
				free(m_cliplist);
				m_cliplist = 0;
			}
			if(direct) {
				free(direct);
				direct = 0;
			}
			m_numcliprects = info->clip_list_count; 
			m_cliplist = (clipping_rect *)
				malloc(m_numcliprects * sizeof(clipping_rect));
			direct = (direct_info *)
				malloc(m_numcliprects * sizeof(direct_info));
			if (m_cliplist) {
				memcpy(m_cliplist, info->clip_list,
					m_numcliprects*sizeof(clipping_rect));
				
				m_rowbytes = info->bytes_per_row;
				m_format = info->pixel_format;
				snes9x_is_32 = m_format == B_RGB32 ? 32 : 16;
				m_bitsperpixel = info->bits_per_pixel >> 3;
				m_bounds = info->window_bounds;
				
				if(!IsFullScreen()) {
					m_bounds.top += menubar_height + 1;
					m_bits = (uint8 *)info->bits + (m_bounds.top * m_rowbytes);
					for(j = 0; j < m_numcliprects; j++)
						if (m_cliplist[j].top <= (long)(info->window_bounds.top + menubar_height))
							m_cliplist[j].top = info->window_bounds.top + menubar_height + 1;
				} else {
					switch(settings.ui.s9x_mode) {
					case 0 ... 5:
						for(j = 0; j < (uint32)m_numcliprects; j++)
						{
							m_cliplist[j].left = info->window_bounds.left + ((info->window_bounds.right)/2) - ((int32)off_screen.Bounds().right/2);
							m_cliplist[j].top = info->window_bounds.top + ((info->window_bounds.bottom)/2) - ((int32)get_height());
							m_cliplist[j].bottom = ((m_cliplist[j].top)) + ((int32)((get_height() * 2) - 1));
							m_cliplist[j].right = (m_cliplist[j].left) + ((int32)(off_screen.Bounds().right));
						}
						break;
					}
					m_bits = (uint8 *)info->bits + (m_cliplist[0].top * m_rowbytes);
				}
				set_direct_info();
				m_connected = true;
			}
			m_locker->Unlock();
			break;
		case B_DIRECT_STOP:
			m_locker->Lock();
			m_connected = false;
			m_locker->Unlock();
			break;
	}
}

void s9x_main_window::set_direct_info()
{
	bool mFull = IsFullScreen();
	for(uint32 i = 0; i < m_numcliprects; i++) {
		clipping_rect *cl = &m_cliplist[i];
		direct[i].x = mFull ? 0 : (cl->left - m_bounds.left);
		direct[i].y = mFull ? 0 : (cl->top - m_bounds.top);
		direct[i].w = (cl->right - cl->left);
		direct[i].h = (cl->bottom - m_bounds.top);
	}
}

void s9x_main_window::render_2xsai()
{
	clipping_rect *clip = m_cliplist;
	for(uint32 i = 0; i < m_numcliprects; i++, clip++) {
		int32 width = (clip->right - clip->left)+1;
		int32 height = (clip->bottom - clip->top)+1;
		if(width == 512 && height == get_height() * 2)
			if(m_format == B_RGB16 || m_format == B_RGB32)
				_2xSaI((uint8 *)off_screen.Bits(), off_screen.BytesPerRow(), (uint8 *)Delta, m_bits + clip->left * m_bitsperpixel, m_rowbytes, width >> 1, height >> 1);
	}
}

void s9x_main_window::render_s2xsai()
{
	clipping_rect *clip = m_cliplist;
	for(uint32 i = 0; i < m_numcliprects; i++, clip++) {
		int32 width = (clip->right - clip->left)+1;
		int32 height = (clip->bottom - clip->top)+1;
		if(width == 512 && height == get_height() * 2)
			if(m_format == B_RGB16 || m_format == B_RGB32)
				Super2xSaI((uint8 *)off_screen.Bits(), off_screen.BytesPerRow(), (uint8 *)Delta, m_bits + clip->left * m_bitsperpixel, m_rowbytes, width >> 1, height >> 1);
	}
}

void s9x_main_window::render_seagle()
{
	clipping_rect *clip = m_cliplist;
	for(uint32 i = 0; i < m_numcliprects; i++, clip++) {
		int32 width = (clip->right - clip->left)+1;
		int32 height = (clip->bottom - clip->top)+1;
		if(width == 512 && height == get_height() * 2)
			if(m_format == B_RGB16 || m_format == B_RGB32)
				SuperEagle((uint8 *)off_screen.Bits(), off_screen.BytesPerRow(), (uint8 *)Delta, m_bits + clip->left * m_bitsperpixel, m_rowbytes, width >> 1, height >> 1);
	}
}

void s9x_main_window::render_scan_or_double(const int type, bool scanlines)
{
	clipping_rect *clip = m_cliplist;
	direct_info *di = (direct_info *)direct;
	int32 height = get_height() * 2;
	const int multi = scanlines ? 2 : 1;
	uint8 *data = (uint8 *)off_screen.Bits();
	int32 rbytes = off_screen.BytesPerRow();
	
	for(uint32 i = 0; i < m_numcliprects; i++, clip++, di++)
	for(int32 y = 0; y < height; y += multi)
	if(y >= di->y && y <= di->h) {
		uint8 *dst = (uint8 *)(m_bits + ((y) * (m_rowbytes)) +((clip->left) * m_bitsperpixel));
		uint8 *src = (uint8 *)data + ((y >> 1) * rbytes) + (((di->x) >> 1) * type);
		switch(m_format) {
			case B_CMAP8:
				draw_line16to8double(src, dst, (di->w + 1) >> 1, type);
				break;
			case B_RGB16:
				draw_line16to16double(src, dst, (di->w + 1) >> 1, type);
				break;
			case B_RGB32:
				draw_line16to32double(src, dst, (di->w + 1) >> 1, type);
				break;
			default:
				break;
		}
	}
}

void s9x_main_window::render_lcd(const int type)
{
	clipping_rect *clip = m_cliplist;
	direct_info *di = (direct_info *)direct;
	int32 height = get_height() * 2;
	uint8 *data = (uint8 *)off_screen.Bits();
	int32 rbytes = off_screen.BytesPerRow();
	
	for(uint32 i = 0; i < m_numcliprects; i++, clip++, di++)
	for(int32 y = 0; y < height; y += 2)
	if(y >= di->y && y <= di->h) {
		uint8 *dst = (uint8 *)(m_bits + ((y) * (m_rowbytes)) +((clip->left) * m_bitsperpixel));
		uint8 *src = (uint8 *)data + ((y >> 1) * rbytes) + (((di->x) >> 1) * type);
		switch(m_format) {
			case B_CMAP8:
				draw_line16to8lcd(src, dst, (di->w + 1) >> 1, type);
				break;
			case B_RGB16:
				draw_line16to16lcd(src, dst, (di->w + 1) >> 1, type);
				break;
			case B_RGB32:
				draw_line16to32lcd(src, dst, (di->w + 1) >> 1, type);
				break;
			default:
				break;
		}
	}
}

void s9x_main_window::render_multi(int w)
{
	const int type = w > 256 ? 8 : 4;
	clipping_rect *clip = m_cliplist;
	for(uint32 i = 0; i < m_numcliprects; i++, clip++) {
		uint8 *src = (uint8 *)off_screen.Bits();
		uint8 *dst = (uint8 *)m_bits;
		draw_16_to_16_multi(src, dst, type, 0, 512/2, get_height(), 640, 480, off_screen.BytesPerRow(), m_rowbytes);
	}
}

void s9x_main_window::render_normal()
{
	clipping_rect *clip = m_cliplist;
	direct_info *di = (direct_info *)direct;
	int32 height = get_height();
	
	for(uint32 i = 0; i < m_numcliprects; i++, clip++, di++)
	for(int32 y = 0; y < height; y += 1)
	if(y >= di->y && y <= di->h) {
		uint8 *dst = (uint8 *)(m_bits + ((y) * (m_rowbytes)) +((clip->left) * m_bitsperpixel));
		uint8 *src = (uint8 *)off_screen.Bits() + ((y) * off_screen.BytesPerRow()) + (((di->x)) * 2);
		switch(m_format) {
			case B_CMAP8:
				draw_line16to8(src, dst, (di->w + 1));
				break;
			case B_RGB16:
				draw_line16to16(src, dst, (di->w + 1));
				break;
			case B_RGB32:
				draw_line16to32(src, dst, (di->w + 1));
				break;
			default:
				break;
		}
	}
}

void s9x_main_window::DrawDirect(int Width, int Height)
{
	const int type = Width > 256 ? 4 : 2;
	if(needs_clear) { ClearScreen(); needs_clear = false; }
	m_locker->Lock();
	if(m_connected) {
		switch(settings.ui.s9x_mode) {
			case 0:
				render_scan_or_double(type, 0);
				break;
			case 1:
				render_scan_or_double(type, 1);
				break;
			case 2:
				render_lcd(type);
				break;
			case 3:
				render_2xsai();
				break;
			case 4:
				render_s2xsai();
				break;
			case 5:
				render_seagle();
				break;
			case 6:
				if(IsFullScreen()) render_multi(Width);
				break;
		}
	}
	m_locker->Unlock();
}

void s9x_main_window::DirectClear(bool clear)
{
	if(!clear) return;
	#include "system_clear_direct.h"
}

void s9x_main_window::ResizeTo(float width, float height)
{
	height += menubar_height + 1;
	BDirectWindow::ResizeTo(width, height);
}

void s9x_main_window::WindowActivated(bool active)
{
	if(active) needs_clear = true;
	if(!exiting && settings.ui.s9x_inactive && !ispaused) Settings.Paused = !active;
	for(int i = 0; i < 128; i++) key[i].pressed = 0;
	BDirectWindow::WindowActivated(active);
}

void s9x_main_window::MenuActivated(bool active)
{
	if(!exiting && settings.ui.s9x_menuactive && !ispaused && IsActive()) Settings.Paused = active; // whew!!
}

void s9x_main_window::DePause()
{
	ispaused = false;
	BMenuItem *item = g_menu->FindItem(S9x_PAUSE);
	if(item) item->SetMarked(false);
}

void s9x_main_window::ClearScreen()
{
	if(Delta) {
		int delta_len = (IMAGE_WIDTH + 2) * IMAGE_HEIGHT * 2;
		uint32 *start = (uint32 *)Delta;
		uint32 *end = (uint32 *)Delta + (delta_len >> 2);
		while(start < end)
			*start++ ^= ~0;
	}
	needs_clear = false;
}

void s9x_main_window::SetGraphicsMode(int g_mode, bool clear)
{
	settings.ui.s9x_mode = g_mode;
	ResizeTo((SNES_WIDTH * 2) - 1, (get_height() * 2) - 1);
	if(settings.ui.s9x_mode > 0 && m_connected && settings.ui.s9x_drawmode != OVERLAY) DirectClear(clear);
}

void s9x_main_window::ScreenChanged(BRect frame, color_space mode)
{
	if(!IsFullScreen()) BScreen().GetMode(&prev_mode);
}

static void sleep_until()
{
	static bigtime_t prev_tick = 0;
	static const bigtime_t frame_snooze = 1000000 / 60;
	bigtime_t snooze_time = frame_snooze - (system_time() - prev_tick);
	if (snooze_time > 0) snooze(snooze_time);
	prev_tick = system_time();
}

void s9x_main_window::DrawOverlay(int Width, int Height)
{
	if(LockWithTimeout(50000) == B_OK) {
		draw_16_to_ycbr_multi((uint8 *)off_screen.Bits(), (uint8 *)ov_bitmap->Bits(),
			Width, Height, 512, 224, off_screen.BytesPerRow(), ov_bitmap->BytesPerRow());
		Unlock();
	}
}

void s9x_main_window::DrawStandard(int Width, int Height)
{
	if(LockWithTimeout(50000) == B_OK) {
		s9x_mview->DrawBitmap(&off_screen, BRect(0, 0, Width - 1, Height - 1), s9x_mview->Bounds());
		Unlock();
	}
}

s9x_main_window::~s9x_main_window()
{
	int32 result;
	exiting = true;
	wait_for_thread(thread, &result);
	if(s9x_player) {
		s9x_player->Stop();
		delete s9x_player;
		s9x_player = 0;
	}
	delete m_locker;
	delete gamestick;
	delete s9x_filepanel;
	SetSettings(settings);
	S9xExit();
	kill_thread(TimerID);
	kill_thread(key_id);
	if(IsFullScreen()) BScreen().SetMode(&prev_mode);
}

bool8 S9xInitUpdate ()
{
    return (TRUE);
}

bool8 S9xDeinitUpdate (int Width, int Height, bool8 sixteen_bit)
{
	if(settings.ui.s9x_vsync) BScreen().WaitForRetrace();
	switch(settings.ui.s9x_drawmode) {
		case DIRECT:
			s9x->DrawDirect(Width, Height);
			break;
		case OVERLAY:
			s9x->DrawOverlay(Width, Height);
			break;
		case BITMAP:
			s9x->DrawStandard(Width, Height);
			break;
	}
	if(settings.ui.s9x_snooze) sleep_until();
	asm("emms");
	return (true);
}

uint32 S9xReadJoypad (int which1)
{
	return (which1 < 5) ? (0x80000000 | joypads [which1]) : 0;
}

void S9xProcessEvents (bool8 block)
{
	s9x->ProcessEvents();
}

static void clear_keys(input_interface *ai)
{
	ai[S9X_SCRLOCK].pressed = 0;
	ai[S9X_PAUSE].pressed = 0;
	ai[S9X_NUMLOCK].pressed = 0;
	ai[S9X_CAPSLOCK].pressed = 0;
}

int32 s9x_main_window::key_thread(void *o)
{
	s9x_main_window *d = (s9x_main_window *)o;
	while(1)
	{
		key_info info;
		if(d->IsActive() && get_key_info(&info) == B_NO_ERROR)
		{
			for(int i = 0; i < 128; i++)
				d->key[i].pressed = info.key_states[i >> 3] & (1 << (7 - (i % 8)));
		}
		clear_keys(d->key);
		snooze(100000 / 30);
	}
	return 0;
}

void s9x_main_window::ProcessEvents()
{
	input_interface *ai = key;
	for(int i = 0; i < 5; i++)
		joypads[i] = 0;
	
	// Controller 1
	ai[settings.s9x_controller1.k_up].pressed		? joypads[0] |= SNES_UP_MASK : 0;
	ai[settings.s9x_controller1.k_down].pressed		? joypads[0] |= SNES_DOWN_MASK : 0;
	ai[settings.s9x_controller1.k_left].pressed		? joypads[0] |= SNES_LEFT_MASK : 0;
	ai[settings.s9x_controller1.k_right].pressed 	? joypads[0] |= SNES_RIGHT_MASK : 0;
	ai[settings.s9x_controller1.k_a].pressed		? joypads[0] |= SNES_A_MASK : 0;
	ai[settings.s9x_controller1.k_b].pressed 		? joypads[0] |= SNES_B_MASK : 0;
	ai[settings.s9x_controller1.k_x].pressed 		? joypads[0] |= SNES_X_MASK : 0;
	ai[settings.s9x_controller1.k_y].pressed 		? joypads[0] |= SNES_Y_MASK : 0;
	ai[settings.s9x_controller1.k_l].pressed 		? joypads[0] |= SNES_TL_MASK : 0;
	ai[settings.s9x_controller1.k_r].pressed 		? joypads[0] |= SNES_TR_MASK : 0;
	ai[settings.s9x_controller1.k_start].pressed 	? joypads[0] |= SNES_START_MASK : 0;
	ai[settings.s9x_controller1.k_select].pressed 	? joypads[0] |= SNES_SELECT_MASK : 0;
	
	// Controller 2
	ai[settings.s9x_controller2.k_up].pressed 		? joypads[1] |= SNES_UP_MASK : 0;
	ai[settings.s9x_controller2.k_down].pressed 	? joypads[1] |= SNES_DOWN_MASK : 0;
	ai[settings.s9x_controller2.k_left].pressed 	? joypads[1] |= SNES_LEFT_MASK : 0;
	ai[settings.s9x_controller2.k_right].pressed 	? joypads[1] |= SNES_RIGHT_MASK : 0;
	ai[settings.s9x_controller2.k_a].pressed 		? joypads[1] |= SNES_A_MASK : 0;
	ai[settings.s9x_controller2.k_b].pressed 		? joypads[1] |= SNES_B_MASK : 0;
	ai[settings.s9x_controller2.k_x].pressed 		? joypads[1] |= SNES_X_MASK : 0;
	ai[settings.s9x_controller2.k_y].pressed 		? joypads[1] |= SNES_Y_MASK : 0;
	ai[settings.s9x_controller2.k_l].pressed 		? joypads[1] |= SNES_TL_MASK : 0;
	ai[settings.s9x_controller2.k_r].pressed 		? joypads[1] |= SNES_TR_MASK : 0;
	ai[settings.s9x_controller2.k_start].pressed 	? joypads[1] |= SNES_START_MASK : 0;
	ai[settings.s9x_controller2.k_select].pressed 	? joypads[1] |= SNES_SELECT_MASK : 0;
	
	// Controller 3
	ai[settings.s9x_controller3.k_up].pressed 		? joypads[2] |= SNES_UP_MASK : 0;
	ai[settings.s9x_controller3.k_down].pressed 	? joypads[2] |= SNES_DOWN_MASK : 0;
	ai[settings.s9x_controller3.k_left].pressed 	? joypads[2] |= SNES_LEFT_MASK : 0;
	ai[settings.s9x_controller3.k_right].pressed 	? joypads[2] |= SNES_RIGHT_MASK : 0;
	ai[settings.s9x_controller3.k_a].pressed 		? joypads[2] |= SNES_A_MASK : 0;
	ai[settings.s9x_controller3.k_b].pressed 		? joypads[2] |= SNES_B_MASK : 0;
	ai[settings.s9x_controller3.k_x].pressed 		? joypads[2] |= SNES_X_MASK : 0;
	ai[settings.s9x_controller3.k_y].pressed 		? joypads[2] |= SNES_Y_MASK : 0;
	ai[settings.s9x_controller3.k_l].pressed 		? joypads[2] |= SNES_TL_MASK : 0;
	ai[settings.s9x_controller3.k_r].pressed 		? joypads[2] |= SNES_TR_MASK : 0;
	ai[settings.s9x_controller3.k_start].pressed 	? joypads[2] |= SNES_START_MASK : 0;
	ai[settings.s9x_controller3.k_select].pressed 	? joypads[2] |= SNES_SELECT_MASK : 0;
	
	// Controller 4
	ai[settings.s9x_controller4.k_up].pressed 		? joypads[3] |= SNES_UP_MASK : 0;
	ai[settings.s9x_controller4.k_down].pressed 	? joypads[3] |= SNES_DOWN_MASK : 0;
	ai[settings.s9x_controller4.k_left].pressed 	? joypads[3] |= SNES_LEFT_MASK : 0;
	ai[settings.s9x_controller4.k_right].pressed 	? joypads[3] |= SNES_RIGHT_MASK : 0;
	ai[settings.s9x_controller4.k_a].pressed 		? joypads[3] |= SNES_A_MASK : 0;
	ai[settings.s9x_controller4.k_b].pressed 		? joypads[3] |= SNES_B_MASK : 0;
	ai[settings.s9x_controller4.k_x].pressed 		? joypads[3] |= SNES_X_MASK : 0;
	ai[settings.s9x_controller4.k_y].pressed 		? joypads[3] |= SNES_Y_MASK : 0;
	ai[settings.s9x_controller4.k_l].pressed 		? joypads[3] |= SNES_TL_MASK : 0;
	ai[settings.s9x_controller4.k_r].pressed 		? joypads[3] |= SNES_TR_MASK : 0;
	ai[settings.s9x_controller4.k_start].pressed 	? joypads[3] |= SNES_START_MASK : 0;
	ai[settings.s9x_controller4.k_select].pressed 	? joypads[3] |= SNES_SELECT_MASK : 0;
	
	// Controller 5
	ai[settings.s9x_controller5.k_up].pressed 		? joypads[4] |= SNES_UP_MASK : 0;
	ai[settings.s9x_controller5.k_down].pressed 	? joypads[4] |= SNES_DOWN_MASK : 0;
	ai[settings.s9x_controller5.k_left].pressed 	? joypads[4] |= SNES_LEFT_MASK : 0;
	ai[settings.s9x_controller5.k_right].pressed 	? joypads[4] |= SNES_RIGHT_MASK : 0;
	ai[settings.s9x_controller5.k_a].pressed 		? joypads[4] |= SNES_A_MASK : 0;
	ai[settings.s9x_controller5.k_b].pressed 		? joypads[4] |= SNES_B_MASK : 0;
	ai[settings.s9x_controller5.k_x].pressed 		? joypads[4] |= SNES_X_MASK : 0;
	ai[settings.s9x_controller5.k_y].pressed 		? joypads[4] |= SNES_Y_MASK : 0;
	ai[settings.s9x_controller5.k_l].pressed 		? joypads[4] |= SNES_TL_MASK : 0;
	ai[settings.s9x_controller5.k_r].pressed 		? joypads[4] |= SNES_TR_MASK : 0;
	ai[settings.s9x_controller5.k_start].pressed 	? joypads[4] |= SNES_START_MASK : 0;
	ai[settings.s9x_controller5.k_select].pressed 	? joypads[4] |= SNES_SELECT_MASK : 0;
	
	Settings.TurboMode = ai[S9X_TAB].pressed;
	if(check_key(S9X_F1)) s9x_states(0);
	else if(check_key(S9X_F2)) s9x_states(1);
	else if(check_key(S9X_F3)) s9x_states(2);
	else if(check_key(S9X_F4)) s9x_states(3);
	else if(check_key(S9X_F5)) s9x_states(4);
	
	if(check_key(S9X_F6)) s9x_states(0, false);
	else if(check_key(S9X_F7)) s9x_states(1, false);
	else if(check_key(S9X_F8)) s9x_states(2, false);
	else if(check_key(S9X_F9)) s9x_states(3, false);
	else if(check_key(S9X_F10)) s9x_states(4, false);
}

/* Debounce keyboard buttons */
int s9x_main_window::check_key(int code) // handy function from smsplus
{
    static int lastpressed = 0;

    if((!key[code].pressed) && (lastpressed == code))
        lastpressed = 0;

    if((key[code].pressed) && (lastpressed != code))
    {
        lastpressed = code;
        return (1);
    }
    return (0);
}

void S9xParseArg (char **argv, int &i, int argc)
{
}

void S9xSyncSpeed ()
{
    if (!Settings.TurboMode && Settings.SkipFrames == AUTO_FRAMERATE)
    {
	while (FrameTimer == 0)
	{
	    S9xProcessEvents (FALSE);
	}

	if (FrameTimer > 10)
	    FrameTimer = 10;
	if (FrameTimer > 1 && IPPU.SkippedFrames < 10)
	{
	    IPPU.SkippedFrames++;
	    IPPU.RenderThisFrame = FALSE;
	}
	else
	{
	    IPPU.RenderThisFrame = TRUE;
	    IPPU.SkippedFrames = 0;
	}
	FrameTimer--;
    }
    else
    {
	if (++IPPU.FrameSkip >= Settings.SkipFrames)
	{
	    IPPU.FrameSkip = 0;
	    IPPU.RenderThisFrame = TRUE;
	    IPPU.SkippedFrames = 0;
	}
	else
	{
	    IPPU.SkippedFrames++;
	    IPPU.RenderThisFrame = FALSE;
	}
    }
}

void S9xMessage(int type, int message_no, const char *str)
{
	S9xSetInfoString (str);
}

void S9xSetPalette()
{
}

void _makepath (char *path, const char *, const char *dir,
		const char *fname, const char *ext)
{
    if (dir && *dir)
    {
	strcpy (path, dir);
	strcat (path, "/");
    }
    else
	*path = 0;
    strcat (path, fname);
    if (ext && *ext)
    {
        strcat (path, ".");
        strcat (path, ext);
    }
}

void _splitpath (const char *path, char *drive, char *dir, char *fname,
		 char *ext)
{
    *drive = 0;

    char *slash = strrchr (path, '/');
    if (!slash)
	slash = strrchr (path, '\\');

    char *dot = strrchr (path, '.');

    if (dot && slash && dot < slash)
	dot = NULL;

    if (!slash)
    {
	strcpy (dir, "");
	strcpy (fname, path);
        if (dot)
        {
	    *(fname + (dot - path)) = 0;
	    strcpy (ext, dot + 1);
        }
	else
	    strcpy (ext, "");
    }
    else
    {
	strcpy (dir, path);
	*(dir + (slash - path)) = 0;
	strcpy (fname, slash + 1);
        if (dot)
	{
	    *(fname + (dot - slash) - 1) = 0;
    	    strcpy (ext, dot + 1);
	}
	else
	    strcpy (ext, "");
    }
}

const char *S9xBasename (const char *f)
{
    const char *p;
    if ((p = strrchr (f, '/')) != NULL || (p = strrchr (f, '\\')) != NULL)
	return (p + 1);

    if ((p = strrchr (f, SLASH_CHAR)))
	return (p + 1);

    return (f);
}

const char *S9xGetFilename (const char *ex)
{
    static char filename [PATH_MAX + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];

    _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
    strcpy (filename, S9xGetSnapshotDirectory ());
    strcat (filename, SLASH_STR);
    strcat (filename, fname);
    strcat (filename, ex);

    return (filename);
}

const char *GetHomeDirectory ()
{
    return (getenv ("HOME"));
}

const char *S9xGetSnapshotDirectory ()
{
	static char filename [PATH_MAX];
	BPath path;
	if(find_directory(B_USER_SETTINGS_DIRECTORY, &path, true) != B_OK)
	path.SetTo("/boot/home/config/settings");
	path.Append("snapshots");
	strcpy(filename, path.Path());
	mkdir (filename, 0777);
	return filename;
}

void S9xAutoSaveSRAM ()
{
    Memory.SaveSRAM (S9xGetFilename (".srm"));
}

void S9xGenerateSound ()
{
}

void S9xExtraUsage ()
{
}

int superscope_pause = 0;
int superscope_turbo = 0;

bool8 S9xReadSuperScopePosition (int &x, int &y, uint32 &buttons)
{
    x = xx;
    y = yy;
    buttons = button_d | (superscope_turbo << 2) | 
              (superscope_pause << 3);
    return (TRUE);
}

bool8 S9xReadMousePosition (int which1, int &x, int &y, uint32 &buttons)
{
	if (which1 == 0)
    {
      x = xx;
      y = yy;
      buttons = button_d;
      return (TRUE);
    }
    return (FALSE);
}

bool JustifierOffscreen()
{
	return (bool)((button_d&2)!=0);
}

void JustifierButtons(uint32& justifiers)
{
	if(IPPU.Controller==SNES_JUSTIFIER_2)
	{
		if((button_d&1)||(button_d&2))
		{
			justifiers|=0x00200;
		}
		if(button_d&4)
		{
			justifiers|=0x00800;
		}
	}
	else
	{
		if((button_d&1)||(button_d&2))
		{
			justifiers|=0x00100;
		}
		if(button_d&4)
		{
			justifiers|=0x00400;
		}
	}
}

extern "C" char* osd_GetPackDir()
{
  static char filename[_MAX_PATH];
  memset(filename, 0, _MAX_PATH);
  
  if(strlen(S9xGetSnapshotDirectory())!=0)
    strcpy (filename, S9xGetSnapshotDirectory());
  else
  {
    char dir [_MAX_DIR + 1];
    char drive [_MAX_DRIVE + 1];
    char name [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
    _splitpath(Memory.ROMFilename, drive, dir, name, ext);
    _makepath(filename, drive, dir, NULL, NULL);
  }
  
  if(!strncmp((char*)&Memory.ROM [0xffc0], "SUPER POWER LEAG 4   ", 21))
  {
    if (getenv("SPL4PACK"))
      return getenv("SPL4PACK");
    else 
      strcat(filename, "/SPL4-SP7");
  }
  else if(!strncmp((char*)&Memory.ROM [0xffc0], "MOMOTETSU HAPPY      ",21))
  {
    if (getenv("MDHPACK"))
      return getenv("MDHPACK");
    else 
      strcat(filename, "/SMHT-SP7");
  }
  else if(!strncmp((char*)&Memory.ROM [0xffc0], "HU TENGAI MAKYO ZERO ", 21))
  {
    if (getenv("FEOEZPACK"))
      return getenv("FEOEZPACK");
    else 
      strcat(filename, "/FEOEZSP7");
  }
  else if(!strncmp((char*)&Memory.ROM [0xffc0], "JUMP TENGAIMAKYO ZERO",21))
  {
    if (getenv("SJNSPACK"))
      return getenv("SJNSPACK");
    else 
      strcat(filename, "/SJUMPSP7");
  } else strcat(filename, "/MISC-SP7");
  return filename;
}

const char *S9xGetFilenameInc (const char *e)
{
    static char filename [_MAX_PATH + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
    char *ptr;
    struct stat buf;

    if (strlen (S9xGetSnapshotDirectory()))
    {
        _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
        strcpy (filename, S9xGetSnapshotDirectory());
        strcat (filename, "/");
        strcat (filename, fname);
        ptr = filename + strlen (filename);
        strcat (filename, "00/");
        strcat (filename, e);
    }
    else
    {
        _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
        strcat (fname, "00/");
        _makepath (filename, drive, dir, fname, e);
        ptr = strstr (filename, "00/");
    }

    do
    {
        if (++*(ptr + 2) > '9')
        {
            *(ptr + 2) = '0';
            if (++*(ptr + 1) > '9')
            {
                *(ptr + 1) = '0';
                if (++*ptr > '9')
                    break;
            }
        }
    } while( stat(filename, &buf) == 0 );

    return (filename);
}

void S9xSetTitle (const char *string)
{
	s9x->SetTitle(string);
}

bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file)
{
    char filename [PATH_MAX + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char ext [_MAX_EXT + 1];

    _splitpath (fname, drive, dir, filename, ext);

    if (*drive || *dir == '/' ||
	(*dir == '.' && (*(dir + 1) == '/'
        )))
    {
	strcpy (filename, fname);
	if (!*ext)
	    strcat (filename, ".s96");
    }
    else
    {
	strcpy (filename, S9xGetSnapshotDirectory ());
	strcat (filename, SLASH_STR);
	strcat (filename, fname);
	if (!*ext)
	    strcat (filename, ".s96");
    }
    
#ifdef ZLIB
    if (read_only)
    {
	if ((*file = OPEN_STREAM (filename, "rb")))
	    return (TRUE);
    }
    else
    {
	if ((*file = OPEN_STREAM (filename, "wb")))
	{
	    chown (filename, getuid (), getgid ());
	    return (TRUE);
	}
    }
#else
    char command [PATH_MAX];
    
    if (read_only)
    {
	sprintf (command, "gzip -d <\"%s\"", filename);
	if (*file = popen (command, "r"))
	    return (TRUE);
    }
    else
    {
	sprintf (command, "gzip --best >\"%s\"", filename);
	if (*file = popen (command, "wb"))
	    return (TRUE);
    }
#endif
    return (FALSE);
}

void S9xCloseSnapshotFile (STREAM file)
{
#ifdef ZLIB
    CLOSE_STREAM (file);
#else
    pclose (file);
#endif
}

void S9xExit()
{
	if(GFX.ZBuffer) { delete [] GFX.ZBuffer; GFX.ZBuffer = 0; }
	if(GFX.SubZBuffer) { delete [] GFX.SubZBuffer; GFX.SubZBuffer = 0; }
	Memory.SaveSRAM (S9xGetFilename (".srm"));
    Memory.Deinit ();
    S9xDeinitAPU ();
}

static int S9xCompareSDD1IndexEntries (const void *p1, const void *p2)
{
    return (*(uint32 *) p1 - *(uint32 *) p2);
}

void BufferProc(void *cookie, void *buff, size_t len, const media_raw_audio_format &format)
{
	if(settings.ui.s9x_mute) { memset(buff, 0, len); return; }
	sound_locker.Lock();
	S9xMixSamples((uint8 *)buff,(so.sixteen_bit?so.buffer_size>>1:so.buffer_size));
	sound_locker.Unlock();
}

static int Rates[8] =
{
    0, 8192, 11025, 16500, 22050, 29300, 36600, 44100
};

bool8 S9xOpenSoundDevice (int mode, bool8 stereo, int buffer_size)
{
	media_raw_audio_format format;
    int playback_rate = Rates [mode & 7];
    int sixteen_bit = TRUE;
    int in_stereo = stereo;

    so.mute_sound = TRUE;
    
    format.frame_rate = playback_rate;
    format.channel_count = (in_stereo?2:1);
    format.format = media_raw_audio_format::B_AUDIO_SHORT;
    format.buffer_size = buffer_size;
    format.byte_order = B_MEDIA_LITTLE_ENDIAN;
    
 	s9x_player = new BSoundPlayer(&format, "Snes9x", BufferProc);
 	
 	if(s9x_player->InitCheck() != B_OK) {
 		s9x_player->Stop();
 		delete s9x_player;
 		s9x_player = 0;
		return (FALSE);
    }
#if DEBUG
    printf("buffer_size: %ld\n", format.buffer_size);
    printf("Initialized Audio: %f hz %s\n", format.frame_rate, (format.channel_count == 1 ? "Mono" : "Stereo"));
#endif
	s9x_player->Start();
	so.stereo = in_stereo;
	so.sixteen_bit = sixteen_bit;
	so.playback_rate = playback_rate;
	so.buffer_size = buffer_size;
	so.encoded = FALSE;
	s9x_player->SetHasData(true);
	
#if DEBUG
    printf ("Rate: %d, Buffer size: %d, 16-bit: %s, Stereo: %s, Encoded: %s\n",
	    so.playback_rate, so.buffer_size, so.sixteen_bit ? "yes" : "no",
	    so.stereo ? "yes" : "no", so.encoded ? "yes" : "no");
#endif
    so.mute_sound = FALSE;

    return (true);
}

void S9xLoadSDD1Data ()
{
    char filename [_MAX_PATH + 1];
    char index [_MAX_PATH + 1];
    char data [_MAX_PATH + 1];
    char patch [_MAX_PATH + 1];

    Memory.FreeSDD1Data ();

    strcpy (filename, S9xGetSnapshotDirectory ());

    if (strncmp (Memory.ROMName, "Star Ocean", 10) == 0)
	strcat (filename, "/socnsdd1");
    else
	strcat (filename, "/sfa2sdd1");

    DIR *dir = opendir (filename);

    index [0] = 0;
    data [0] = 0;
    patch [0] = 0;

    if (dir)
    {
	struct dirent *d;
	
	while ((d = readdir (dir)))
	{
	    if (strcasecmp (d->d_name, "SDD1GFX.IDX") == 0)
	    {
		strcpy (index, filename);
		strcat (index, "/");
		strcat (index, d->d_name);
	    }
	    else
	    if (strcasecmp (d->d_name, "SDD1GFX.DAT") == 0)
	    {
		strcpy (data, filename);
		strcat (data, "/");
		strcat (data, d->d_name);
	    }
	    if (strcasecmp (d->d_name, "SDD1GFX.PAT") == 0)
	    {
		strcpy (patch, filename);
		strcat (patch, "/");
		strcat (patch, d->d_name);
	    }
	}
	closedir (dir);

	if (strlen (index) && strlen (data))
	{
	    FILE *fs = fopen (index, "rb");
	    int len = 0;

	    if (fs)
	    {
		// Index is stored as a sequence of entries, each entry being
		// 12 bytes consisting of:
		// 4 byte key: (24bit address & 0xfffff * 16) | translated block
		// 4 byte ROM offset
		// 4 byte length
		fseek (fs, 0, SEEK_END);
		len = ftell (fs);
		rewind (fs);
		Memory.SDD1Index = (uint8 *) malloc (len);
		fread (Memory.SDD1Index, 1, len, fs);
		fclose (fs);
		Memory.SDD1Entries = len / 12;

		if (!(fs = fopen (data, "rb")))
		{
		    free ((char *) Memory.SDD1Index);
		    Memory.SDD1Index = NULL;
		    Memory.SDD1Entries = 0;
		}
		else
		{
		    fseek (fs, 0, SEEK_END);
		    len = ftell (fs);
		    rewind (fs);
		    Memory.SDD1Data = (uint8 *) malloc (len);
		    fread (Memory.SDD1Data, 1, len, fs);
		    fclose (fs);

		    if (strlen (patch) > 0 &&
			(fs = fopen (patch, "rb")))
		    {
			fclose (fs);
		    }
#ifdef MSB_FIRST
		    // Swap the byte order of the 32-bit value triplets on
		    // MSBFirst machines.
		    uint8 *ptr = Memory.SDD1Index;
		    for (int i = 0; i < Memory.SDD1Entries; i++, ptr += 12)
		    {
			SWAP_DWORD ((*(uint32 *) (ptr + 0)));
			SWAP_DWORD ((*(uint32 *) (ptr + 4)));
			SWAP_DWORD ((*(uint32 *) (ptr + 8)));
		    }
#endif
		    qsort (Memory.SDD1Index, Memory.SDD1Entries, 12,
			   S9xCompareSDD1IndexEntries);
		}
	    }
	}
	else
	{
	    printf ("Decompressed data pack not found in '%s'.\n", filename);
	}
    }
}

int32 s9x_timer(void *o) // from Be's Millipede game(thanks Be)
{
	bigtime_t nextTime, snoozeTime, baseTime;
	baseTime = system_time();
	if((Settings.FrameTime) == 0) Settings.FrameTime = 16667;
	while(1) {
		nextTime = baseTime + Settings.FrameTime;
		snoozeTime = (nextTime - system_time());
		
		while (snoozeTime >= 1.0) {
			snooze(snoozeTime);
			snoozeTime = nextTime - system_time();
		}
		FrameTimer++;
		baseTime = nextTime;
	}
	return(0);
}