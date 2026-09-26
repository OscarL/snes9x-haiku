#include "s9x_key_names.h"
#include "input_interface.h"
#include "Gamestick.h"

GameStick::GameStick(input_interface *buffer) : jbuffer(buffer)
{
	for(int i = 0; i < MAX_JOYSTICKS; i++)
		sticks[i] = 0;
	
	BJoystick stick;
	char devName[B_OS_NAME_LENGTH];
	for(int i = 0; i < stick.CountDevices() && i < MAX_JOYSTICKS; i++)
	{
		sticks[i] = new Joystick(i, buffer);
		if((sticks[i]->GetDeviceName(i, devName) != B_OK) || (sticks[i]->Open(devName, true) < B_OK))
		{
			delete sticks[i];
			sticks[i] = 0;
		} else
			sticks[i]->InitNames();
	}
	InitKeyNames();
}

void GameStick::InitKeyNames()
{
	for(int i = 0; i < 128; i++)
		jbuffer[i].name = key_name(i);
}

void GameStick::Poll()
{
	for(int i = 0; i < MAX_JOYSTICKS; i++)
		if(sticks[i]) sticks[i]->Update();
}

GameStick::~GameStick()
{
	for(int i = 0; i < MAX_JOYSTICKS; i++)
		if(sticks[i]) { sticks[i]->Close(); delete sticks[i]; }
}

GameStickThread::GameStickThread()
	: ATicker(25)
{
}

void GameStickThread::Tick(const double tickCount)
{
	this->Poll();
}
