/*
 * Time.c
 *
 *  Created on: 09.03.2015
 *      Author: Richard
 */

#include "Time.h"

#ifdef MAXTIMERS
#include "input/Buttons.h"
#include <msp430.h>

/**
 * status bit field:
 * ACSEWWWWWWWWBxxx
 * A: is active
 * C: is cyclic
 * S: Task on start
 * E: Task on end
 * WWWWWWWW: wait time (0..254)
 * B: button connected
 * xxx: number of the connected button (0..7)
 */
//static const unsigned int connectedButtonMask = 0x0007;
//static const unsigned int buttonConnected = 0x0008;
//static const unsigned int taskIsOnStart = 0x2000;
//static const unsigned int taskIsOnEnd = 0x1000;

inline uint16_t Timer_getExponentAndTime(uint16_t time);
uint16_t Timer_getExponentAndTime(uint16_t time) {
    uint16_t timeCpy = time;
    uint16_t exponent = 0;
    while (timeCpy & ~timer_waitTimeMask) {
        timeCpy >>= 2;
        exponent += 2;
    }
    switch (exponent) {
    case 0: break;
    case 2: exponent = WaitTimer_exponent_2; break;
    case 4: exponent = WaitTimer_exponent_4; break;
    default: exponent = 0; break;
    }
    return (exponent | timeCpy);
}

WaitTimer* initWaitTimer(uint16_t waitTime)
{
	waitTimers_mem[timers_size].status = 0;
	waitTimers_mem[timers_size].status |= Timer_getExponentAndTime(waitTime);
	waitTimers_mem[timers_size].currentWaitTime = 0;
	waitTimers_mem[timers_size].taskOnStart = -1;
	waitTimers_mem[timers_size].taskOnStop = -1;
	timers_size += 1;
	return &waitTimers_mem[timers_size-1];
}

inline void stopTimer(WaitTimer* waitTimer);
void stopTimer(WaitTimer* waitTimer) {
    if (waitTimer->taskOnStop != -1) {
        scheduleTask(&task_mem[waitTimer->taskOnStop]);
    }
    if (waitTimer->status & WaitTimer_isCyclicTimer) {
        setTimer(waitTimer);
    }
    else {
        waitTimer->status &= ~WaitTimer_isActive;
    }
}

void setTaskOnStart(WaitTimer* waitTimer, Task* task)
{
	waitTimer->taskOnStart = getTaskNumber(task);
}
void setTaskOnStop(WaitTimer* waitTimer, Task* task)
{
	waitTimer->taskOnStop = getTaskNumber(task);
}

void setNewWaitTime(uint16_t waitTime, WaitTimer* waitTimer)
{
	waitTimer->currentWaitTime = waitTime;
}

void setTimerCyclic(WaitTimer* waitTimer)
{
	waitTimer->status |= WaitTimer_isCyclicTimer;
}

void waitScheduler()
{
	signed char i;
	WaitTimer* wT;
	for (i=timers_size; i>0; i--)
	{
		wT = &waitTimers_mem[i-1];
		if (wT->status & WaitTimer_isActive)
		{
			if (wT->currentWaitTime != 0)
			{
				{
					wT->currentWaitTime -= 1;
				}
			}
			else
			{
			    stopTimer(wT);
			}
		}
	}
}

#endif /* MAXTIMERS */
