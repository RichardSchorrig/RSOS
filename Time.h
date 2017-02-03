/*
 * Time.h
 *
 *  Created on: 09.03.2015
 *      Author: Richard
 */

#ifndef TIME_H_
#define TIME_H_

#include "Task.h"
#include "RSOSDefines.h"

struct WaitTimer_t;

/**
 * WaitTimer structure
 * Fields:
 *  currentWaitTime: the wait time the timer is currently in, decrements per cycle
 *  status: bit field which holds:
 *      ACSEWWWWWWWWBxxx
 *      A: is active
 *      C: is cyclic
 *      S: Task on start
 *      E: Task on stop
 *      WWWWWWWW: wait time (0..254)
 *      B: button connected
 *      xxx: number of the connected button (0..7)  //todo: 0000 no button 0001: button 0, 1111 button 14
 *  taskOnStart: a task that is scheduled when its WaitTimer is set active
 *  taskOnStop: a task that is scheduled when its WaitTimer is stopped
 *
 * MEMORY:
 *  this structure takes up 7 Bytes
 *
 * todo: extra field button, extent wait time to 11 Bits (0..2040)
 */
typedef struct WaitTimer_t{
	volatile unsigned char currentWaitTime;
	volatile unsigned int status;
	Task* taskOnStart;
	Task* taskOnStop;
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
 * @param waitTimer: the wait timer to set
 * @param waitTime: the new wait time
 */
void setNewWaitTime(unsigned char waitTime, WaitTimer* waitTimer);

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
 * adds a button to the specified timer. this is called automatically if a Button is initialized.
 * DO NOT CALL!
 * @param waitTimer: the timer to set the button to
 * @param button the button to set
 */
void addButtonToTimer(WaitTimer* waitTimer, unsigned char buttonNr);

inline void waitScheduler();

#endif /* TIME_H_ */
