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
 * 2017 03 08
 *      moved inline functions to header file
 * 2017 03 31
 * 		changed function buttonWaitScheduler(): checks if any button is active, if not, the timer
 * 		calling the buttonWaitScheduler is halted
 * 		changed function buttonPressed(): calls setTimer() to activate the waitTimer calling the button
 * 		wait scheduler
 * 		changed function initButton(): no changes to the port is done (past: port pin was put to input direction,
 * 		interrupt enabled etc)
 * 		changed function disableBtnInterrupt() and enableBtnInterrupt(): now call setPortInterrupt() in the
 * 		hardware adaption layer
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_


#include <RSOSDefines.h>

#include <stdint.h>

/* exclude everything if not used */
#ifdef MAXBUTTONS

#include "../Task.h"
#include "../WaitTimer.h"
#include <HardwareAdaptionLayer.h>

extern WaitTimer* timer_buttonWaitScheduler;

//#include <msp430.h>

struct Button_t;

/**
 * Button structure
 * Fields:
 *  status: bit field which holds:
 *      ATEE WWWW
 *      A: isActive (currently pressed)
 *      T: Task bit: 0: Task is scheduled on press (start), 1: Task is scheduled on debounce (when wait time is zero)
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
    volatile unsigned char * port;
	uint8_t bit;
	int8_t task;
} Button;

extern int8_t buttons_size;
extern Button buttons_mem[MAXBUTTONS];

/**
 * bit identifier: is active
 */
#define Button_isActive 0x80

/**
 * bit identifier: task is on press (immediately scheduled when button is pressed)
 * if not set, the task is scheduled when debounce time is counted to zero
 */
#define Button_taskOnPress 0x40

/**
 * mask for the wait time's exponent
 */
#define Button_exponentMask 0x30

/**
 * exponent 1
 */
#define Button_exponent_0 0x00

/**
 * exponent 2
 */
#define Button_exponent_2 0x10

/**
 * exponent 4
 */
#define Button_exponent_4 0x20

/**
 * mask for the actual wait time (debounce time)
 */
#define button_waitTimeMask 0x0F

/**
 * enables button operation
 * this function inits the task "task_buttonWaitScheduler"
 * and the connected WaitTimer "timer_buttonWaitScheduler"
 *
 * to operate, the following structures must be available:
 *      1x Task
 *      1x WaitTimer
 *
 * it is possible to add a multiplier to all button debounce times
 * this is done by the wait timer which controls the button wait scheduler
 */
__EXTERN_C
void initButtonOperation(uint16_t clockMultiply);

/**
 * inits a button to handle the port interrupt when pressed.
 * Does not initialize the port to be input direction etc.
 *
 * when buttonPressed() is called, a scheduler is activated which
 * checks if the button is held down and controls the debouncing of
 * the input pin
 *
 * Call initButtonOperation() first!
 *
 * @param bit: the bit the pin is at
 * @param port: the port the pin is at
 * @param waitTime: the time that passes until the interrupt is enabled after interrupt occurs
 * @return: a reference to the new button
 */
//Button* initButton(unsigned char bit, unsigned char port, unsigned char waitTime);
__EXTERN_C
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
__EXTERN_C
void addTaskOnPressToButton(Button* button, Task* task);

/**
 * adds a task that should be scheduled after the wait time has passed
 *
 * only one task is supported. If a task was already added (either by addTaskOnPress... or addTaskOnRelease...)
 * the old task pointer is overwritten by the new one.
 *
 * @param button: the button to set the task to
 * @param task: the task to schedule
 */
__EXTERN_C
void addTaskOnReleaseToButton(Button* button, Task* task);

/**
 * disables the interrupt for the pin the button is connected to.
 * @param btn the button which interrupt should be disabled.
 */
static inline void disableBtnInterrupt(Button* btn) __attribute__((always_inline));;
static inline void disableBtnInterrupt(Button* btn)
{
	setPortInterrupt(btn->port, btn->bit, 0);
	/*
    if (btn->port == &P1IN)
    {P1IE &= ~btn->bit;}
    else if (btn->port == &P2IN)
    {P2IE &= ~btn->bit;}
    */
}

/**
 * sets the button's wait time for debouncing
 * @param btn the button which wait time is set
 */
static inline void Button_setWaitTime(Button* btn) __attribute__((always_inline));;
static inline void Button_setWaitTime(Button* btn) {
    uint8_t exponent;
    switch (btn->status & Button_exponentMask) {
    case Button_exponent_0: exponent = 0; break;
    case Button_exponent_2: exponent = 2; break;
    case Button_exponent_4: exponent = 4; break;
    default: exponent = 0; break;
    }
    btn->currentWaitTime = (btn->status & button_waitTimeMask) << (exponent);
}

/**
 * function to call in ISR, when button is pressed.
 * This function disables the interrupt and starts the wait timer for the button.
 * if a task is connected to the button press, it is scheduled.
 * else if a task is connected to the button release, it will be scheduled on release.
 * @param button the button being pressed
 */
static inline void buttonPressed(Button* button) __attribute__((always_inline));;
static inline void buttonPressed(Button* button) {
    if (!(button->status & Button_isActive)) {
        disableBtnInterrupt(button);
        Button_setWaitTime(button);
        if ((button->status & Button_taskOnPress) && (button->task != -1)) {
            scheduleTask(&task_mem[button->task]);
        }
        button->status |= Button_isActive;
    }
    setTimer(timer_buttonWaitScheduler);
}

/**
 * the button wait scheduler
 * checks all buttons and takes care of the interrupt enable registers
 */
__EXTERN_C
void buttonWaitScheduler();

//inline void enableBtnInterrupt(Button* btn);
//inline void disableBtnInterrupt(Button* btn);

#endif /* MAXBUTTONS */
#endif /* BUTTONS_H_ */
