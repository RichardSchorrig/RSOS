/*
 * Time.h
 *
 *  Created on: 09.03.2015
 *      Author: Richard
 *
 * Changelog:
 * 2017 02 05
 *      added function continueTimer(WaitTimer*), not tested
 * 2017 02 06
 *      refactoring: splitting WaitTimer and Button, changed task reference to number
 * 2017 03 05
 *      inlining functions setTimer(), haltTimer(), continueTimer()
 * 2017 03 31
 * 		changed function setTimer(): now checks if active, activates task on start only
 * 		if status is inactive
 * 		changed function stopTimer(): deactivates timer before activating again to comply
 * 		with changes to setTimer()
 * 2017 05 04
 *      added flag WAITTIMER_TASK to identify that the waitScheduler is called from the task
 *      scheduler. if WAITTIMER_TASK is not defined, the waitScheduler is run directly in
 *      the timer ISR
 */

#ifndef WAITTIMER_H_
#define WAITTIMER_H_

#include "Task.h"
#include <RSOSDefines.h>

#include <stdint.h>

#include "RSOS_BasicInclude.h"

#ifdef MAXTIMERS

/**
 * mask for the actual wait time
 */
#define timer_waitTimeMask 0x0FFF

/**
 * mask for the exponent (0, 2, 4)
 */
#define exponentMask 0x3000

/**
 * exponent 1
 */
#define WaitTimer_exponent_0 0x0000

/**
 * exponent 2
 */
#define WaitTimer_exponent_2 0x1000

/**
 * exponent 4
 */
#define WaitTimer_exponent_4 0x2000

/**
 * bit identifier: cyclic
 */
#define WaitTimer_isCyclicTimer 0x4000

/**
 * bit identifier: active
 */
#define WaitTimer_isActive 0x8000

struct WaitTimer_t;

/**
 * WaitTimer structure
 * Fields:
 *  currentWaitTime: the wait time the timer is currently in, decrements per cycle
 *  status: bit field which holds:
 *      ACEE WWWW WWWW WWWW
 *      A: is active
 *      C: is cyclic
 *      EE: exponent to wait time (0..4) wait time value is shifted by exponent
 *          possible combinations:
 *              00: shift by 0
 *              01: shift by 2
 *              10: shift by 4
 *              11: reserved
 *      W...: wait time (0..4095)
 *  taskOnStart: a task that is scheduled when its WaitTimer is set active
 *  taskOnStop: a task that is scheduled when its WaitTimer is stopped
 *  connectedButton: a button connected to this timer, to debounce button
 *
 * MEMORY:
 *  this structure takes up 6 Bytes
 *
 */
typedef struct WaitTimer_t{
	volatile uint16_t status;
	volatile uint16_t currentWaitTime;
	int8_t taskOnStart;
	int8_t taskOnStop;
} WaitTimer;

extern int8_t timers_size;
extern WaitTimer waitTimers_mem[MAXTIMERS];

#ifdef WAITTIMER_TASK
extern Task* task_waitScheduler;
#endif /* WAITTIMER_TASK */

/**
 * initialize the wait timer operation.
 * Inits a task with priority 0 that schedules the
 * waitScheduler. The waitScheduler is responsible for
 * counting down the wait time and scheduling the tasks
 *
 * if WAITTIMER_TASK is defined,
 * to operate, a free task structure is needed
 * 1x Task
 * consider the structures in your RSOSDefines
 * the task is scheduled on every timer ISR and run when possible
 *
 * if WAITTIMER_TASK is not defined,
 * the waitScheduler is run directly in the timer ISR.
 */
__EXTERN_C
void Timer_initOperation();

/**
 * the waitTimer waitScheduler.
 * checks all active WaitTimer, decrements their waitTime, runs the tasks if they stop
 */
__EXTERN_C
void waitScheduler();

/**
 * to be called in a timer interrupt routine.
 * The task task_waitScheduler is scheduled.
 */
static inline void Timer_ISR() __attribute__((always_inline));
static inline void Timer_ISR()
{
#ifdef WAITTIMER_TASK
    scheduleTask(task_waitScheduler);
#else
    waitScheduler();
#endif /* WAITTIMER_TASK */
}

/**
 * initializes a wait timer. Timer can schedule a Task on starting and on stop
 * @param waitTime: the time to wait; 0..250 Ticks (250 Ticks = 5 sec)
 * @return a reference to the new WaitTimer
 */
__EXTERN_C
WaitTimer* initWaitTimer(uint16_t waitTime);

/**
 * add a task that should be scheduled if the waitTimer is started
 * if the timer is cyclic, the task is also scheduled every time the timer reaches zero
 * @param waitTimer: the WaitTimer to add the task to
 * @param task: the task to add
 */
__EXTERN_C
void setTaskOnStart(WaitTimer* waitTimer, Task* task);

/**
 * add a task that should be scheduled if the WaitTimer is stopped
 * if the timer is cyclic, the task is scheduled every time the timer reaches zero
 * @param waitTimer: the WaitTimer to add the task to
 * @param task: the task to add
 */
__EXTERN_C
void setTaskOnStop(WaitTimer* waitTimer, Task* task);

/**
 * sets a new wait time to the timer. this only affects the current wait time, after the timer is stopped, the original time is restored
 * can be used in combination with haltTimer and continueTimer
 * @param waitTimer: the wait timer to set
 * @param waitTime: the new wait time
 */
__EXTERN_C
void setNewWaitTime(uint16_t waitTime, WaitTimer* waitTimer);

/**
 * sets the specified timer to be started again after it reaches zero
 * @param waitTimer: the timer to set cyclic
 */
__EXTERN_C
void setTimerCyclic(WaitTimer* waitTimer);

/**
 * sets the specified timer active to count
 * will activate the task on start if the timer is not running
 * @param waitTimer: the timer to set active
 */
static inline void setTimer(WaitTimer* waitTimer) __attribute__((always_inline));
static inline void setTimer(WaitTimer* waitTimer)
{
	if (!(waitTimer->status & WaitTimer_isActive))
	{
		if (waitTimer->taskOnStart != -1)
		{
			scheduleTask(&task_mem[waitTimer->taskOnStart]);
		}
		switch (waitTimer->status & exponentMask) {
		case WaitTimer_exponent_0: waitTimer->currentWaitTime = waitTimer->status & timer_waitTimeMask; break;
		case WaitTimer_exponent_2: waitTimer->currentWaitTime = (waitTimer->status & timer_waitTimeMask) << 2; break;
		case WaitTimer_exponent_4: waitTimer->currentWaitTime = (waitTimer->status & timer_waitTimeMask) << 4; break;
		}
		waitTimer->status |= WaitTimer_isActive;
	}
}

/**
 * stops the specified timer. the end task is not scheduled.
 * @param waitTimer: the timer to stop
 */
static inline void haltTimer(WaitTimer* waitTimer) __attribute__((always_inline));
static inline void haltTimer(WaitTimer* waitTimer)
{
    waitTimer->status &= ~WaitTimer_isActive;
}

/**
 * continues the specified timer. the task on start is not scheduled,
 * and the timer is not reset. It continues to run where it was halted.
 * @param waitTimer: the timer to continue
 */
static inline void continueTimer(WaitTimer* waitTimer) __attribute__((always_inline));
static inline void continueTimer(WaitTimer* waitTimer)
{
    waitTimer->status |= WaitTimer_isActive;
}

#endif /* MAXTIMERS */
#endif /* WAITTIMER_H_ */
