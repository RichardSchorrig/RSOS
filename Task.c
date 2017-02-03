/*
 * Task.c
 *
 *  Created on: 05.03.2015
 *      Author: Richard
 */

#include "Task.h"
#include <msp430.h>

#define MAX_NR_OF_FOLLOWUP_TASKS 7

static const unsigned int activeBit = 0x8000;

//static const unsigned int hasFollowUpTask = 0x4000;	//deprecated, check
static const unsigned int followUpNumberMask = 0x7000;	//new, check

static const unsigned int isCycleTask = 0x0800;
static const unsigned int cycleNumberMask = 0x0700;

static const unsigned int hasWaitTime = 0x0080;
static const unsigned int waitTimeMask = 0x0070;

static const unsigned int priorityMask = 0x000F;

unsigned char savedTaskNumber;

Task* addTask(unsigned char priority, TaskFunction* taskfunction)
{
//	Task task;
	task_mem[tasks_size].status = (priority & priorityMask);
	task_mem[tasks_size].task = taskfunction;
	task_mem[tasks_size].currentCycle = 0;
	task_mem[tasks_size].currentDelay = 0;

//	tasks[tasks_size] = &task_mem[tasks_size];
//	task_mem[tasks_size] = task;
	tasks_size += 1;
	return &task_mem[tasks_size-1];
}


void scheduleTask(Task* task)
{
	if (~task->status & activeBit)
	{
		task->status |= activeBit;
		numberOfRunningTasks += 1;

		if ((task->status & priorityMask) > currentPriority)		//check the currentPriority
		{
			currentPriority = (task->status & priorityMask);		//maximum priority of running task
		}
	}
}

inline void resetDelay(Task* task)
{
	task->currentDelay = ((task->status & waitTimeMask) >> 4) + 1;
}

inline void resetCycles(Task* task)
{
	task->currentCycle = ((task->status & cycleNumberMask) >> 8) + 1;
}

void unscheduleTask(Task* task) //todo: check if active
{
    if (task->status & activeBit) {
        numberOfRunningTasks -= 1;
        task->status &= ~activeBit;
        currentPriority = 0;
    }

	signed char i = ((task->status & followUpNumberMask) >> 12) - 1;
	for (; i>=0; i--)
	{
		scheduleTask(task->followUpTask[i]);
	}
	
	if (task->status & hasWaitTime)
	{
		resetDelay(task);
	}
}

void addFollowUpTask(Task* task, Task* *followUpArray, Task* followUpTask)
{

	//task_mem[taskNumber].status |= hasFollowUpTask;

	unsigned char numberOfFollowUps = (task->status & followUpNumberMask) >> 12;
	if (numberOfFollowUps < MAX_NR_OF_FOLLOWUP_TASKS)
	{
	    if (numberOfFollowUps == 0)
	    {
	        if (task->followUpTask == 0) {
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
	schedulerEnabled = 1;
}

void disableScheduler()
{
	schedulerEnabled = 0;
}

void saveCurrentContext()
{
	savedTaskNumber = currentRunningTask;
	unscheduleTask(&task_mem[currentRunningTask]);		//this is bad, schedules all following tasks
	//todo: another unschedule function
}

void restoreCurrentContext()
{
	currentRunningTask = savedTaskNumber;
	scheduleTask(&task_mem[currentRunningTask]);
}

void scheduler()
{
	while (numberOfRunningTasks || schedulerEnabled)
	{
		signed char i;
		Task* task;

		for (i=tasks_size-1; i>=0; i--)
		{
			task = &task_mem[i];
			currentRunningTask = i;

			if (task->status & activeBit)						//Only active tasks go further
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
//						resetDelay(task); /* New: Delay is not resetted during run (when Task is cyclic) but in function unscheduleTask() */
					}
				}
			}
		}
		__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
//		__bis_SR_register(LPM3_bits + GIE);       // Enter LPM0 w/ interrupt
	}
}


