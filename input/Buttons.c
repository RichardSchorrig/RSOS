/*
 * Buttons.c
 *
 *  Created on: 02.03.2015
 *      Author: Richard
 */

#include "Buttons.h"

/* exclude everything if not used */
#ifdef MAXBUTTONS

WaitTimer* timer_buttonWaitScheduler = 0;

void initButtonOperation(uint16_t clockMultiply) {
	Task* task_buttonWaitScheduler = addTask(0, buttonWaitScheduler);
    timer_buttonWaitScheduler = initWaitTimer(clockMultiply);

    setTimerCyclic(timer_buttonWaitScheduler);
    setTaskOnStop(timer_buttonWaitScheduler, task_buttonWaitScheduler);
//    setTimer(timer_buttonWaitScheduler);
}

inline uint8_t Button_getExponentAndTime(uint8_t time);
uint8_t Button_getExponentAndTime(uint8_t time) {
    uint8_t timeCpy = time;
    uint8_t exponent = 0;
    while (timeCpy & ~button_waitTimeMask) {
        timeCpy >>= 2;
        exponent += 2;
    }
    switch (exponent) {
    case 0: break;
    case 2: exponent = Button_exponent_2; break;
    case 4: exponent = Button_exponent_4; break;
    default: exponent = 0; break;
    }
    return (exponent | timeCpy);
}

Button* initButton(unsigned char bit, volatile unsigned char * port, uint8_t waitTime)
{
	buttons_mem[buttons_size].bit = bit;
	buttons_mem[buttons_size].port = port;
	buttons_mem[buttons_size].status = Button_getExponentAndTime(waitTime);
	buttons_mem[buttons_size].currentWaitTime = 0;
	buttons_mem[buttons_size].task = -1;

	buttons_size += 1;

	/*
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
	*/
	return &buttons_mem[buttons_size-1];
}

void addTaskOnPressToButton(Button* button, Task* task)
{
	button->task = getTaskNumber(task);
	button->status |= Button_taskOnPress;
}

void addTaskOnReleaseToButton(Button* button, Task* task)
{
    button->task = getTaskNumber(task);
    button->status &= ~Button_taskOnPress;
}

inline void enableBtnInterrupt(Button* btn);
void enableBtnInterrupt(Button* btn)
{
	setPortInterrupt(btn->port, btn->bit, 1);
	/*
	if (btn->port == &P1IN)
	{P1IE |= btn->bit;}
	else if (btn->port == &P2IN)
	{P2IE |= btn->bit;}
	*/
}

static inline void buttonReleased(Button* button);
static void buttonReleased(Button* button) {
    enableBtnInterrupt(button);
    if ((~button->status & Button_taskOnPress) && button->task != -1) {
        scheduleTask(&task_mem[button->task]);
    }
    button->status &= ~Button_isActive;
}

#include <msp430.h>
void buttonWaitScheduler() {
    buttonSchedulerEntered();

    int8_t i;
    uint8_t noButtons = 0;
    Button* btn;
    for (i=buttons_size; i>0; i-=1) {
        btn = &buttons_mem[i-1];
        if (buttons_mem[i-1].status & Button_isActive) {
        	btn = &buttons_mem[i-1];
            if (btn->currentWaitTime == 0 || btn->currentWaitTime == 255) {    //wait time is over

                if (*(btn->port) & btn->bit) {  //button is not pressed
                    if (btn->currentWaitTime == 0xFF) { //button was pressed on debounce time
                        Button_setWaitTime(btn);        //debounce second time
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
        else
        {
        	noButtons += 1;
        }
    }

    if (noButtons >= buttons_size)
    {
    	haltTimer(timer_buttonWaitScheduler);	// end operation
    }

    buttonSchedulerExited();
}

#endif /* MAXBUTTONS */
