/*
 * Buttons.h
 *
 *  Created on: 02.03.2015
 *      Author: Richard
 *
 * Changelog
 * 2017 02 05
 *      changed button structure: now contains port as pointer to actual port, further changes pending
 * 2017 02 06
 *      refactoring: splitting button and waitTimer, changed task reference to number
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "../Task.h"
#include "../Time.h"
#include "../Path.h"
#include PATH_RSOSDEFINES_H

#include <stdint.h>

struct Button_t;

/**
 * Button structure
 * Fields:
 *  status: bit field which holds:
 *      ATEE WWWW
 *      A: isActive (currently pressed)
 *      T: Task bit: 0: Task on press (start), 1: Task on debounce (wait time is zero)
 *      EE: exponent to wait time (0..4) wait time value is shifted by exponent
 *          possible combinations:
 *              00: shift by 0
 *              01: shift by 2
 *              10: shift by 4
 *              11: reserved
 *      W...: wait time (0..15)
 *  bit: the pin number
 *  port: the port the pin belongs to
 *  task: the number of the task, -1 if no task is available
 *
 *  MEMORY
 *      this structure takes up 6 Bytes
 */
typedef struct Button_t{
    uint8_t status;
    volatile uint8_t currentWaitTime;
	uint8_t bit;
	volatile unsigned char * port;
	int8_t task;
} Button;

extern char buttons_size;
//extern Button* buttons_mem;
extern Button buttons_mem[MAXBUTTONS];

/**
 * enables button operation
 * this function inits the task "task_buttonWaitScheduler"
 * and the connected WaitTimer "timer_buttonWaitScheduler"
 *
 * it is possible to add a multiplier to all button debounce times
 * this is done by the
 */
void initButtonOperation(uint16_t clockMultiply);

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
//Button* initButton(unsigned char bit, unsigned char port, unsigned char waitTime);
Button* initButton(unsigned char bit, volatile unsigned char * port, uint8_t waitTime);

/**
 * adds a task that should be scheduled if the pin interrupt is received
 *
 * only one task is supported. If a task was already added (either by addTaskOnPress... or addTaskOnRelease...,
 * the old task pointer is overwritten by the new one.
 *
 * @param button: the button to set the task to
 * @param task: the task to schedule
 */
void addTaskOnPressToButton(Button* button, Task* task);

/**
 * adds a task that should be scheduled after the wait time has passed
 *
 * only one task is supported. If a task was already added (either by addTaskOnPress... or addTaskOnRelease...,
 * the old task pointer is overwritten by the new one.
 *
 * @param button: the button to set the task to
 * @param task: the task to schedule
 */
void addTaskOnReleaseToButton(Button* button, Task* task);

/**
 * function to call in ISR, when button is pressed.
 * This function disables the interrupt and starts the wait timer for the button.
 * if a task is connected to the button press, it is scheduled.
 * else if a task is connected to the button release, it will be scheduled on release.
 */
void buttonPressed(Button* button);

/**
 * the button wait scheduler
 * checks all buttons and takes care of the interupt enable registers
 */
void buttonWaitScheduler();

void setWaitTime(struct Button_t* btn);
inline void enableBtnInterrupt(Button* btn);
inline void disableBtnInterrupt(Button* btn);

#endif /* BUTTONS_H_ */
