#ifndef __SNES9X_REFRESH_WINDOW_H__
#define __SNES9X_REFRESH_WINDOW_H__

class s9x_refresh_window : public BWindow
{
public:
	s9x_refresh_window(BRect rect, BWindow *parent, uint32 closemsg);
	virtual void Quit();
private:
	BWindow *handle;
	uint32 s9x_msg;
};

#endif
