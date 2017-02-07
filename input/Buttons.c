/*
 * Buttons.c
 *
 *  Created on: 02.03.2015
 *      Author: Richard
 */

#include "Buttons.h"
#include <msp430.h>

static const uint8_t isActive = 0x80;
static const uint8_t taskOnPress = 0x40;

static const uint8_t exponentMask = 0x30;
#define exponent_0 0x00
#define exponent_2 0x01
#define exponent_4 0x10

static const uint8_t waitTimeMask = 0x0F;

static Task* task_buttonWaitScheduler;
static WaitTimer* timer_buttonWaitScheduler;

void initButtonOperation(uint16_t clockMultiply) {
    task_buttonWaitScheduler = addTask(0, buttonWaitScheduler);
    timer_buttonWaitScheduler = initWaitTimer(clockMultiply);

    setTimerCyclic(timer_buttonWaitScheduler);
    setTaskOnStop(timer_buttonWaitScheduler, task_buttonWaitScheduler);
    setTimer(timer_buttonWaitScheduler);
}

inline uint8_t getExponentAndTime(uint8_t time) {
    uint8_t timeCpy = time;
    uint8_t exponent = 0;
    while (timeCpy & ~waitTimeMask) {
        timeCpy >>= 2;
        exponent += 2;
    }
    switch (exponent) {
    case 0: break;
    case 2: exponent = exponent_2;
    case 4: exponent = exponent_4;
    }
    return (exponent | timeCpy);
}

Button* initButton(unsigned char bit, volatile unsigned char * port, uint8_t waitTime)
{
	buttons_mem[buttons_size].bit = bit;
	buttons_mem[buttons_size].port = port;
	buttons_mem[buttons_size].status = getExponentAndTime(waitTime);
	buttons_mem[buttons_size].currentWaitTime = 0;
	buttons_mem[buttons_size].task = -1;

	buttons_size += 1;

	if (port == &P1IN)
	{
		P1DIR &= ~bit;
		P1IE |= bit;
		P1IES |= bit;
	}
	else if (port == &P2IN)
	{
		P2DIR &= ~bit;
		P2IE |= bit;
		P2IES |= bit;
	}

	return &buttons_mem[buttons_size-1];
}

void addTaskOnPressToButton(Button* button, Task* task)
{
	button->task = getTaskNumber(task);
	button->status |= taskOnPress;
}

void addTaskOnReleaseToButton(Button* button, Task* task)
{
    button->task = getTaskNumber(task);
    button->status &= ~taskOnPress;
}

inline void enableBtnInterrupt(Button* btn)
{
	if (btn->port == &P1IN)
	{P1IE |= btn->bit;}
	else if (btn->port == &P2IN)
	{P2IE |= btn->bit;}
}

inline void disableBtnInterrupt(Button* btn)
{
	if (btn->port == &P1IN)
	{P1IE &= ~btn->bit;}
	else if (btn->port == &P2IN)
	{P2IE &= ~btn->bit;}
}

inline void Button_setWaitTime(Button* btn) {
    uint8_t exponent;
    switch (btn->status & exponentMask) {
    case exponent_0: exponent = 0; break;
    case exponent_2: exponent = 2; break;
    case exponent_4: exponent = 4; break;
    default: exponent = 0; break;
    }
    btn->currentWaitTime = (btn->status & waitTimeMask) << (exponent);
}

void buttonPressed(Button* button) {
    if ((~button->status) & isActive) {
        disableBtnInterrupt(button);
        Button_setWaitTime(button);
        if (button->status & taskOnPress && button->task != -1) {
            scheduleTask(&task_mem[button->task]);
        }
        button->status |= isActive;
    }
}

inline void buttonReleased(Button* button) {
    enableBtnInterrupt(button);
    if (!(button->status & taskOnPress) && button->task != -1) {
        scheduleTask(&task_mem[button->task]);
    }
    button->status &= ~isActive;
}

void buttonWaitScheduler() {
    uint8_t i;
    Button* btn;
    for (i=buttons_size; i>0; i-=1) {
        btn = &buttons_mem[i-1];
        if (btn->status & isActive) {

            if (btn->currentWaitTime == 0 || btn->currentWaitTime == 255) {    //wait time is over

                if (*(btn->port) & btn->bit) {  //button is not pressed
                    if (btn->currentWaitTime == 0xFF) { //button was pressed on debounce time
                        Button_setWaitTime(btn);               //debounce second time
                    }
                    else {
                        buttonReleased(btn);
                    }
                }
                else {                          //button is still pressed
                    btn->currentWaitTime = 255;
                }
            }
            else {
                btn->currentWaitTime -= 1;
            }
        }
    }
}

