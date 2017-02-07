/*
 * Time.c
 *
 *  Created on: 09.03.2015
 *      Author: Richard
 */

#include "Time.h"
#include "input/Buttons.h"
#include <msp430.h>

//static Task dummyTask;


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
static const uint16_t waitTimeMask = 0x0FFF;

static const uint16_t exponentMask = 0x3000;
#define exponent_0 0x0000
#define exponent_2 0x1000
#define exponent_4 0x2000

static const unsigned int isCyclicTimer = 0x4000;
static const unsigned int isActive = 0x8000;

WaitTimer* initWaitTimer(unsigned char waitTime)
{
	waitTimers_mem[timers_size].status = 0;
	waitTimers_mem[timers_size].status |= (waitTime << 4) & waitTimeMask;
	waitTimers_mem[timers_size].currentWaitTime = 0;
	waitTimers_mem[timers_size].taskOnStart = -1;
	waitTimers_mem[timers_size].taskOnStop = -1;
	timers_size += 1;
	return &waitTimers_mem[timers_size-1];
}

void setTimer(WaitTimer* waitTimer)
{
//	if (waitTimer->status & taskIsOnStart)
    if (waitTimer->taskOnStart != -1)
	{
		scheduleTask(&task_mem[waitTimer->taskOnStart]);
	}
    switch (waitTimer->status & exponentMask) {
    case exponent_0: waitTimer->currentWaitTime = waitTimer->status & waitTimeMask; break;
    case exponent_2: waitTimer->currentWaitTime = (waitTimer->status & waitTimeMask) << 2; break;
    case exponent_4: waitTimer->currentWaitTime = (waitTimer->status & waitTimeMask) << 4; break;
    }
	waitTimer->status |= isActive;
}

inline void stopTimer(WaitTimer* waitTimer) {
    if (waitTimer->taskOnStop != -1) {
        scheduleTask(&task_mem[waitTimer->taskOnStop]);
    }
    if (waitTimer->status & isCyclicTimer) {
        setTimer(waitTimer);
    }
    else {
        waitTimer->status &= ~isActive;
    }
}

void haltTimer(WaitTimer* waitTimer)
{
	waitTimer->status &= ~isActive;
}

void continueTimer(WaitTimer* waitTimer)
{
    waitTimer->status |= isActive;
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
	waitTimer->status |= isCyclicTimer;
}

inline void waitScheduler()
{
	signed char i;
	WaitTimer* wT;
	for (i=timers_size; i>0; i--)
	{
		wT = &waitTimers_mem[i-1];
		if (wT->status & isActive)
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

