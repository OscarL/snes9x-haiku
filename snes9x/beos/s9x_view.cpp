#include "s9x_main_window.h"
#include "s9x_view.h"
#include "Cursors.h"

s9x_view::s9x_view(BRect rect, s9x_main_window *window)
	: BView(rect, "", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	handle(window),
	c(new BCursor(c_crosshairs_cursor)),
	lastindex(0),
	needs_mouse(false) {}

void s9x_view::SetNeedsMouse(bool mouse)
{
	needs_mouse = mouse;
}

void s9x_view::ChangeCursor(int index)
{
	if(index == lastindex) return;
	index == 0 ? SetViewCursor(B_CURSOR_SYSTEM_DEFAULT) : SetViewCursor(c);
	lastindex = index;
}

void s9x_view::MouseDown(BPoint point)
{
	uint32		buttons;
	if (Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons) == B_NO_ERROR) {
		if(needs_mouse) handle->UpdateMouse(buttons);
	}
	BView::MouseDown(point);
}

void s9x_view::MouseUp(BPoint point)
{
	uint32		buttons;
	if (Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons) == B_NO_ERROR) {
		if(needs_mouse) handle->UpdateMouse(buttons);
	}
	BView::MouseUp(point);
}

void s9x_view::MouseMoved(BPoint point, uint32 transit, const BMessage *m)
{
	if(needs_mouse) handle->UpdateMousePosition((int)point.x, (int)point.y);
	BView::MouseMoved(point, transit, m);
}