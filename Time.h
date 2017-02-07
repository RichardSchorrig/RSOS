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
 *      refactoring: splitting WaitTimer and Button, changed task reference to number (pending)
 */

#ifndef TIME_H_
#define TIME_H_

#include "Task.h"
#include "Path.h"
#include PATH_RSOSDEFINES_H

#include <stdint.h>

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
 * todo: extra field button, extent wait time to 11 Bits (0..2040)
 */
typedef struct WaitTimer_t{
	volatile uint16_t status;
	volatile uint16_t currentWaitTime;
	int8_t taskOnStart;
	int8_t taskOnStop;
//	Button* connectedButton;
} WaitTimer;

extern char timers_size;
//extern WaitTimer* waitTimers_mem;
extern WaitTimer waitTimers_mem[MAXTIMERS];

/**
 * initializes a wait timer. Timer can schedule a Task on starting and on stop
 * @param waitTime: the time to wait; 0..250 Ticks (250 Ticks = 5 sec)
 * @return a reference to the new WaitTimer
 */
WaitTimer* initWaitTimer(unsigned char waitTime);

/**
 * add a task that should be scheduled if the waitTimer is started
 * if the timer is cyclic, the task is also scheduled every time the timer reaches zero
 * @param waitTimer: the WaitTimer to add the task to
 * @param task: the task to add
 */
void setTaskOnStart(WaitTimer* waitTimer, Task* task);

/**
 * add a task that should be scheduled if the WaitTimer is stopped
 * if the timer is cyclic, the task is scheduled every time the timer reaches zero
 * @param waitTimer: the WaitTimer to add the task to
 * @param task: the task to add
 */
void setTaskOnStop(WaitTimer* waitTimer, Task* task);

/**
 * sets a new wait time to the timer. this only affects the current wait time, after the timer is stopped, the original time is restored
 * can be used in combination with haltTimer and continueTimer
 * @param waitTimer: the wait timer to set
 * @param waitTime: the new wait time
 */
void setNewWaitTime(uint16_t waitTime, WaitTimer* waitTimer);

/**
 * sets the specified timer to be started again after it reaches zero
 * @param waitTimer: the timer to set cyclic
 */
void setTimerCyclic(WaitTimer* waitTimer);

/**
 * sets the specified timer active to count
 * @param waitTimer: the timer to set active
 */
void setTimer(WaitTimer* waitTimer);

/**
 * stops the specified timer. the end task is not scheduled.
 * @param waitTimer: the timer to stop
 */
void haltTimer(WaitTimer* waitTimer);

/**
 * continues the specified timer. the task on start is not scheduled,
 * and the timer is not reset. It continues to run where it was halted.
 * @param waitTimer: the timer to continue
 */
void continueTimer(WaitTimer* waitTimer);

inline void waitScheduler();

#endif /* TIME_H_ */
