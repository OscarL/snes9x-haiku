#ifndef __GAMESTICK_H__
#define __GAMESTICK_H__

#include "atime.h"
#include "joystick.h"

class GameStickThread : public ATicker
{
public:
			GameStickThread();
			
	virtual	void Tick(const double tickCount);
	virtual void Poll() = 0;
	
protected:
private:

};

class GameStick : public GameStickThread
{
public:
	GameStick(input_interface *buffer);
	virtual ~GameStick();
	virtual void Poll();
	virtual void InitKeyNames();
private:
	static const int MAX_JOYSTICKS = 5;
	Joystick *sticks[MAX_JOYSTICKS];
	input_interface *jbuffer;
};

#endif
