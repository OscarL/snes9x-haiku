#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <Joystick.h>

class input_interface;
class Joystick : public BJoystick
{
public:
	Joystick(int index, input_interface *buffer);
	enum { UP, DOWN, LEFT, RIGHT, BUTTONS };
	virtual void UpdateAxes();
	virtual void UpdateButtons();
	virtual status_t Update();
	virtual void InitNames();
private:
	input_interface *mBuf;
	int mIndex;
	static const int MAX_BUTTONS = 32;
	static const int OFFSET = 36;
	static const int KEY_END = 128;
};

#endif
