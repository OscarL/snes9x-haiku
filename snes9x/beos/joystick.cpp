#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input_interface.h"
#include "joystick.h"

Joystick::Joystick(int index, input_interface *buffer)
	: BJoystick()
{
	this->mBuf = &buffer[KEY_END + OFFSET * index];
	this->mIndex = index;
}

void Joystick::InitNames()
{
	mBuf[UP].name = "";
	mBuf[UP].name << "J(" << mIndex << ") A(Up)";
	mBuf[DOWN].name = "";
	mBuf[DOWN].name << "J(" << mIndex << ") A(Down)";
	mBuf[LEFT].name = "";
	mBuf[LEFT].name << "J(" << mIndex << ") A(Left)";
	mBuf[RIGHT].name = "";
	mBuf[RIGHT].name << "J(" << mIndex << ") A(Right)";
	
	int32 numButtons = this->CountButtons();
	for(int i = 0; i < numButtons; i++) {
		mBuf[BUTTONS+i].name = "";
		mBuf[BUTTONS+i].name << "J(" << mIndex << ") " << "B(" << i << ")";
	}
}

status_t Joystick::Update()
{
	status_t status = BJoystick::Update();
	if(status == B_NO_ERROR) {
		UpdateAxes();
		UpdateButtons();
	}
	return(status);
}

void Joystick::UpdateAxes()
{
	int16 axis_count = this->CountAxes();
	int16 *axes = (int16 *) malloc(sizeof(int16) * axis_count);
	this->GetAxisValues(axes);
	mBuf[UP].pressed = axes[1] < -16384;
	mBuf[DOWN].pressed = axes[1] > 16384;
	mBuf[LEFT].pressed = axes[0] < -16384;
	mBuf[RIGHT].pressed = axes[0] > 16384;
	free(axes);
}

void Joystick::UpdateButtons()
{
	uint32 buttons = ButtonValues();
	for(int i = 0; i < MAX_BUTTONS; i++)
		mBuf[BUTTONS+i].pressed = buttons & (1 << i) ? 1 : 0;
}
