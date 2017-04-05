/*
 * Task.c
 *
 *  Created on: 05.03.2015
 *      Author: Richard
 */

#include "Task.h"

#include <HardwareAdaptionLayer.h>

//#include <msp430.h>

#define MAX_NR_OF_FOLLOWUP_TASKS 7

static int8_t task_schedulerEnabled = 0;
static int8_t task_savedTaskNumber = -1;

Task* addTask(unsigned char priority, TaskFunction* taskfunction)
{
	task_mem[tasks_size].status = (priority & priorityMask);
	task_mem[tasks_size].task = taskfunction;
	task_mem[tasks_size].currentCycle = 0;
	task_mem[tasks_size].currentDelay = 0;

	tasks_size += 1;
	return &task_mem[tasks_size-1];
}

void addFollowUpTask(Task* task, Task* *followUpArray, Task* followUpTask)
{
	unsigned char numberOfFollowUps = (task->status & followUpNumberMask) >> 12;
	if (numberOfFollowUps < MAX_NR_OF_FOLLOWUP_TASKS)
	{
	    if (numberOfFollowUps == 0)
	    {
	        if (followUpArray == 0) {
	            return; //error
	        }
	        else {
	            task->followUpTask = followUpArray;
	        }
	    }
		task->followUpTask[numberOfFollowUps] = followUpTask;

		numberOfFollowUps += 1;

		task->status &= ~followUpNumberMask;
		task->status |= (numberOfFollowUps << 12) & followUpNumberMask;
	}
}

void setTaskCyclic(Task* task, char cycles)
{
	task->status |= isCycleTask;
	task->status |= ((cycles-2) << 8) & cycleNumberMask;
	task->currentCycle = cycles - 1;
}

void setTaskDelay(Task* task, char delay)
{
	task->status |= hasWaitTime;
	task->status |= ((delay-1) << 4) & waitTimeMask;
	task->currentDelay = delay;
}

void enableScheduler()
{
	task_schedulerEnabled = 1;
}

void disableScheduler()
{
	task_schedulerEnabled = 0;
}

inline void resetDelay(Task* task);
void resetDelay(Task* task)
{
	task->currentDelay = ((task->status & waitTimeMask) >> 4) + 1;
}

static inline void resetCycles(Task* task) __attribute__((always_inline));
static inline void resetCycles(Task* task)
{
	task->currentCycle = ((task->status & cycleNumberMask) >> 8) + 1;
}

static inline void unscheduleTask(Task* task) __attribute__((always_inline));
static inline void unscheduleTask(Task* task)
{
    if (task->status & Task_isActive) {
        task->status &= ~Task_isActive;
        numberOfRunningTasks -= 1;
        currentPriority = 0;
    }

    if (task->followUpTask != 0)
    {
        signed char i = ((task->status & followUpNumberMask) >> 12) - 1;
        for (; i>0; i--)
        {
            scheduleTask(task->followUpTask[i-1]);
        }
    }

#ifndef NEWSCHEDULER
    if (task->status & hasWaitTime)
    {
        resetDelay(task);
    }
#endif /* NEWSCHEDULER */
}

void saveCurrentContext()
{
	task_savedTaskNumber = currentRunningTask;
	unscheduleTask(&task_mem[currentRunningTask]);		//this is bad, schedules all following tasks
	//todo: another unschedule function
}

void restoreCurrentContext()
{
	currentRunningTask = task_savedTaskNumber;
	scheduleTask(&task_mem[currentRunningTask]);
}

/**
 * returns the task that is active and has the highest priority
 * @return a number in the task_mem array or -1 if no task is active
 */
static inline int8_t getNextTaskNumber() __attribute__((always_inline));
static inline int8_t getNextTaskNumber()
{
	int8_t i;
	int8_t maxPrioTask = -1;
	numberOfRunningTasks = 0;
	for (i=tasks_size; i>0; i-=1)
	{
		if (task_mem[i-1].status & Task_isActive)
		{
		    numberOfRunningTasks += 1;
			int8_t prio = task_mem[i-1].status & priorityMask;
			if ( (prio) >= currentPriority)
			{
				maxPrioTask = i-1;
				currentPriority = prio;
			}
		}
	}
	return maxPrioTask;
}

#ifdef NEWSCHEDULER
void scheduler()
{
	while (numberOfRunningTasks || task_schedulerEnabled)
	{
		schedulerEntered();
#ifdef STRADEGY_NOWAIT
		for (currentRunningTask = getNextTaskNumber();
			 currentRunningTask != -1;
			 currentRunningTask = getNextTaskNumber())
#endif // STRADEGY_NOWAIT //
		{
#ifndef STRADEGY_NOWAIT
			currentRunningTask = getNextTaskNumber();
#endif // STRADEGY_NOWAIT //
			if (currentRunningTask != -1)
			{
				Task* task = &task_mem[currentRunningTask];
				if (task->currentDelay == 0x00)					//if the delay is zero
				{
					task->task();

					if (task->status & isCycleTask)
					{
						if (task->currentCycle == 0x00)
						{
							resetCycles(task);
							unscheduleTask(task);
						}
						else
						{
							task->currentCycle -= 1;
						}
					}
					else
					{
						unscheduleTask(task);
					}
				}
				if (task->status & hasWaitTime)
				{
					if (task->currentDelay != 0x00)
					{
						task->currentDelay -= 1;
						break;		//exit scheduler, todo: synchronize timer interrupt waking scheduler
					}
					else
					{
						resetDelay(task);
					}
				}
			}
		}

		schedulerExited();
		schedulerWait();
	}
}
// old scheduler function
#else
void scheduler()
{
	while (numberOfRunningTasks || task_schedulerEnabled)
	{
		signed char i;
		Task* task;
		schedulerEntered();
		for (i=tasks_size-1; i>=0; i--)
		{
			task = &task_mem[i];
			currentRunningTask = i;

			if (task->status & Task_isActive)					//Only active tasks go further
			{
				if (task->currentDelay == 0x00)					//if the delay is zero
				{
					if ( (task->status & priorityMask) >= currentPriority)//if the currentPriority is even / higher than
					{
						task->task();

						if (task->status & isCycleTask)
						{
							if (task->currentCycle == 0x00)
							{
								resetCycles(task);
								unscheduleTask(task);
							}
							else
							{
								task->currentCycle -= 1;
							}
						}
						else
						{
							unscheduleTask(task);
						}
					}
				}
				if (task->status & hasWaitTime)
				{
					if (task->currentDelay != 0x00)
					{
						task->currentDelay -= 1;
					}
					else
					{
//						resetDelay(task); // New: Delay is not reset during run (when Task is cyclic) but in function unscheduleTask()
					}
				}
			}
		}
		schedulerExited();
		schedulerWait();
//		__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
//		__bis_SR_register(LPM3_bits + GIE);       // Enter LPM3 w/ interrupt
	}
}
#endif /* NEWSCHEDULER */

