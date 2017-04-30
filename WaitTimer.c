/*
 * Time.c
 *
 *  Created on: 09.03.2015
 *      Author: Richard
 */

#include "WaitTimer.h"
#include <HardwareAdaptionLayer.h>

#ifdef MAXTIMERS

Task* task_waitScheduler = 0;

void waitScheduler();

void Timer_initOperation()
{
    task_waitScheduler = addTask(0, waitScheduler);
}

static inline uint16_t Timer_getExponentAndTime(uint16_t time) __attribute__((always_inline));;
static inline uint16_t Timer_getExponentAndTime(uint16_t time) {
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

static inline void stopTimer(WaitTimer* waitTimer) __attribute__((always_inline));;
static inline void stopTimer(WaitTimer* waitTimer) {
    if (waitTimer->taskOnStop != -1) {
        scheduleTask(&task_mem[waitTimer->taskOnStop]);
    }

    waitTimer->status &= ~WaitTimer_isActive;

    if (waitTimer->status & WaitTimer_isCyclicTimer) {
        setTimer(waitTimer);
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
	waitSchedulerEntered();

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

	waitSchedulerExited();
}

#endif /* MAXTIMERS */
