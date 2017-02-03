/*
 * Buttons.h
 *
 *  Created on: 02.03.2015
 *      Author: Richard
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "../Task.h"
#include "../Time.h"
#include "RSOSDefines.h"

struct Button_t;

/**
 * Button structure
 * Fields:
 *  bit: the pin number
 *  port: the port the pin belongs to
 *  timer: a reference to the wait timer to debounce the hardware
 */
typedef struct Button_t{
	unsigned char bit;
	unsigned char port;
	WaitTimer* timer;
} Button;

extern char buttons_size;
//extern Button* buttons_mem;
extern Button buttons_mem[MAXBUTTONS];

//void ButtonConstructor();

/**
 * Inits a Pin to be an input, with direction set to input,
 * interrupt enabled, and interrupt edge select to from high to low
 * If a pin interrupt is received, the interrupt is disabled for that pin.
 * after waitTime ticks it will be enabled again.
 *
 * @param bit: the bit the pin is at
 * @param port: the port the pin is at
 * @param waitTime: the time that passes until the interrupt is enabled after interrupt occurs
 * @return: a reference to the new button
 */
Button* initButton(unsigned char bit, unsigned char port, unsigned char waitTime);

/**
 * adds a task that should be scheduled if the pin interrupt is received
 * @param button: the button to set the task to
 * @param task: the task to schedule
 */
void addTaskOnPressToButton(Button* button, Task* task);

/**
 * adds a task that should be scheduled after the wait time has passed
 * @param button: the button to set the task to
 * @param task: the task to schedule
 */
void addTaskOnReleaseToButton(Button* button, Task* task);

void setWaitTime(struct Button_t* btn);
inline void enableBtnInterrupt(Button* btn);
inline void disableBtnInterrupt(Button* btn);

#endif /* BUTTONS_H_ */
