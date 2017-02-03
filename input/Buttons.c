/*
 * Buttons.c
 *
 *  Created on: 02.03.2015
 *      Author: Richard
 */

#include "Buttons.h"
#include <msp430.h>

Button* initButton(unsigned char bit, unsigned char port, unsigned char waitTime)
{

	buttons_mem[buttons_size].bit = bit;
	buttons_mem[buttons_size].port = port;


	WaitTimer* timer = initWaitTimer(waitTime);
	buttons_mem[buttons_size].timer = timer;
	addButtonToTimer(timer, buttons_size);

	buttons_size += 1;

	if (port == 1)
	{
		P1DIR &= ~bit;
		P1IE |= bit;
		P1IES |= bit;
	}
	else if (port == 2)
	{
		P2DIR &= ~bit;
		P2IE |= bit;
		P2IES |= bit;
	}

	return &buttons_mem[buttons_size-1];
}

void addTaskOnPressToButton(Button* button, Task* task)
{
	setTaskOnStart(button->timer, task);
}

void addTaskOnReleaseToButton(Button* button, Task* task)
{
	setTaskOnStop(button->timer, task);
}

inline void setWaitTime(Button* btn)
{
	setTimer(btn->timer);
	//btn->timer->currentWaitTime = btn->timer->waitTime;
}

inline void enableBtnInterrupt(Button* btn)
{
	if (btn->port == 1)
	{P1IE |= btn->bit;}
	else if (btn->port == 2)
	{P2IE |= btn->bit;}
}

inline void disableBtnInterrupt(Button* btn)
{
	if (btn->port == 1)
	{P1IE &= ~btn->bit;}
	else if (btn->port == 2)
	{P2IE &= ~btn->bit;}
}

