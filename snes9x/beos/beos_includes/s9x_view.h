#ifndef __SNES9X_VIEW_H__
#define __SNES9X_VIEW_H__

#include <View.h>
#include <Cursor.h>

class s9x_main_window;

class s9x_view : public BView
{
public:
	s9x_view(BRect frame, s9x_main_window *window);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *m);
	virtual void ChangeCursor(int index);
	virtual void SetNeedsMouse(bool needs);
private:
	s9x_main_window *handle;
	BCursor *c;
	int lastindex;
	bool needs_mouse;
};

#endif
