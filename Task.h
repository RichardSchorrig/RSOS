/*
 * Task.h
 *
 *  Created on: 05.03.2015
 *      Author: Richard
 *
 *  Changelog
 * 2017 02 03
 *      changed status bits definition
 *      changed followUpTask definition: structure contains pointer to an external defined array, saves some memory
 * 2017 02 05
 *      changed function addFollowUpTask, fixed error
 * 2017 02 06
 *      added function getTaskNumber
 * 2017 03 05
 *      moved function scheduleTask(), getTaskNumber(), to header for inlining
 */

#ifndef TASK_H_
#define TASK_H_

/**
 * compile flags
 */
#define STRADEGY_NOWAIT
#define NEWSCHEDULER
#define STRADEGY_NOBREAK_ONDELAY
#define TASK_WATCHDOG_ENABLE
#define TASK_WATCHDOG_COUNTER 15

#include <RSOSDefines.h>

#include <stdint.h>

#include "RSOS_BasicInclude.h"

/**
 * bit identifier: active
 */
#define Task_isActive 0x8000

/**
 * bit identifier: task has follow up tasks
 */
#define followUpNumberMask 0x7000

/**
 * bit identifier: is cyclic
 */
#define isCycleTask 0x0F00

/**
 * mask for the number of cycles
 */
#define cycleNumberMask 0x0F00

/**
 * bit identifier: has wait time
 */
#define hasWaitTime 0x00F0

/**
 * bit identifier: delayed state
 * in the field currentDelay of Task
 */
#define Task_isDelayed 0x80

/**
 * mask for the wait time
 */
#define waitTimeMask 0x00F0

/**
 * mask for the priority
 */
#define priorityMask 0x000F

/**
 * type definition of the function executed when task is scheduled
 * todo: void* (TaskFunction) (void*) to pass arguments and return values
 * specify those pointers in the linked task structure
 */
typedef void (TaskFunction) (void);

struct Task_t;

/**
 * Task structure
 * Fields:
 * 	task: pointer to a function which is executed
 * 	status: bit field which holds:
 * 	  AFFFCCCCWWWWPPPP
 * 	  A: is active
 * 	  F: number of followup tasks:
 * 	  							001: 1
 * 	  							010: 2
 * 	  							...
 * 	  							111: 7
 * 	  C: number of cycles:
 * 	                            0000: 0
 * 	  							0001: 2
 * 	  							0010: 3
 * 	  							...
 * 	  							1110: 15
 * 	  							1111: infinite  //todo!
 * 	  W: number of delay cycles:
 * 	                            0000: 0
 * 	  							0001: 1
 * 	  							0010: 2
 * 	  							...
 * 	  							1110: 14
 * 	  							1111: 15    //todo!
 * 	  PPPP: Priority from 0 to 15
 *
 * 	currentDelay: delay counter when executed,
 * 				  also contains bit identifier
 * 				  to determine the delay state.
 * 				  The delay state is set when the delay
 * 				  is decremented and reset on wake scheduler interrupt
 * 				  (the task can't be executed as long as this bit is set)
 * 	currentCycle: cycle counter when executed
 * 	followUpTask: more task structures
 *
 *
 * MEMORY:
 *  This structure takes up 8 Bytes
 */
typedef struct Task_t {
	TaskFunction* task;
	volatile uint16_t status;
	volatile uint8_t currentDelay;
	volatile uint8_t currentCycle;
	struct Task_t* *followUpTask;
} Task;

extern signed char tasks_size;
extern Task task_mem[MAXTASKS];
//extern Task* task_mem;

/**
 * shows the task number currently running
 */
extern int8_t currentRunningTask;

/**
 * shows the priority of the current running task
 */
extern uint8_t currentPriority;

/**
 * shows the number of currently active tasks
 */
extern uint8_t numberOfRunningTasks;

/**
 * Initialize the watchdog functionality.
 * This function must not be called if the watchdog function is not needed and the flag is not defined.
 * (Flag TASK_WATCHDOG_ENABLE)
 * Otherwise it must be called. One Task structure is needed, consider one extra in your RSOSDefines.
 *
 * !Important! This function must be the first to be called, before adding any other Tasks!
 */
__EXTERN_C
void Task_initOperation();

/**
 * adds a task to the task array
 * @param unsigned int priority: defines the priority of this task. if the task becomes active and has higher
 * 									 priority than other active tasks, those other tasks are not executed
 * @param TaskFunction* taskfunction: a pointer to a function, which should be executed when task is active
 * @return: a reference to the added task
 */
__EXTERN_C
Task* addTask(unsigned char priority, TaskFunction* taskfunction);

/**
 * adds a task that is executed when the parent task is completed
 * it is possible to add up to 7 following tasks to one task
 * @param task: the position of the parent task in the tasks-array
 * @param followUpArray: a pointer to an initialized Array where the follow up tasks are stored.
 *        might be null, but only if a valid array was added prior. The Array must contain enough space
 *        for the number of tasks to be added as follow up task. Calling the function with a different
 *        pointer than the prior pointer has no effect.
 * @param followUpTask: the position of the following task in the tasks-array
 */
__EXTERN_C
void addFollowUpTask(Task* task, Task* *followUpArray, Task* followUpTask);

/**
 * sets a task cyclic, so it is executed more than once if it becomes active
 * @param task: the position of the task in the tasks-array
 * @param cycles: the number of cycles to be executed (2 to 16 cycles)
 */
__EXTERN_C
void setTaskCyclic(Task* task, char cycles);

/**
 * sets a delay to a task, after a task becomes active, the scheduler waits the given number of cycles before the task is executed
 * @param task: the position of the task in the tasks-array
 * @param delay: the number of cycles the scheduler waits (1 to 15 delay cycles)
 */
__EXTERN_C
void setTaskDelay(Task* task, char delay);

/**
 * set a new delay to the current running task,
 * must only be called within the task function
 *
 * the delay is valid only once, after the delay has passed, the delay saved within the task is set again
 * @param delay the new delay to set
 */
static inline void setTaskDelay_Once(uint8_t delay) __attribute__((always_inline));
static inline void setTaskDelay_Once(uint8_t delay)
{
    task_mem[currentRunningTask].currentDelay = delay;
}

/**
 * set a new cycle number to the current running task,
 * must only be called within the task function
 *
 * the new number of cycles is valid until the task is unscheduled, after that the number of cycles is set to
 * the number within the task
 * @param cycle the new number of cycles
 */
static inline void setTaskCycle_Once(uint8_t cycle) __attribute__((always_inline));
static inline void setTaskCycle_Once(uint8_t cycle)
{
    task_mem[currentRunningTask].currentCycle = cycle;
}

/**
 * @return: the number of the task in the task_mem array, -1 on error
 */
static inline int8_t getTaskNumber(Task* task) __attribute__((always_inline));
static inline int8_t getTaskNumber(Task* task) {
    if (task-task_mem > tasks_size) {
        return -1;
    }
    return task - task_mem;
}

/**
 * resets all delay states for all tasks.
 * This function should only be called in the interrupt that waits the scheduler
 */
static inline void Task_resetDelayState() __attribute__((always_inline));
static inline void Task_resetDelayState()
{
	int8_t i;
	for (i=tasks_size; i>0; i-=1)
	{
		task_mem[i-1].currentDelay &= ~Task_isDelayed;
	}
}

/**
 * sets a task active, it is executed when the scheduler is working
 * This function disables and enables all interrupts within execution
 * @param task: pointer to the task that should be scheduled
 */
static inline void scheduleTask(Task* task) __attribute__((always_inline));
static inline void scheduleTask(Task* task)
{
#ifdef NEWSCHEDULER
    if (!(task->status & Task_isActive))
    {
        task->status |= Task_isActive;
    }
#else
    if (!(task->status & Task_isActive))
    {
        task->status |= Task_isActive;
        numberOfRunningTasks += 1;

        if ((task->status & priorityMask) > currentPriority)        //check the currentPriority
        {
            currentPriority = (task->status & priorityMask);        //maximum priority of running task
        }
    }
#endif /* NEWSCHEDULER */
}

/**
 * sets the scheduler enabled, the scheduler is running continuously
 */
__EXTERN_C
void enableScheduler();

/**
 * stops the scheduler, the scheduler will finish the remaining active tasks
 */
__EXTERN_C
void disableScheduler();

/**
 * the scheduler handles the tasks. they are currently executed in reverse order they where added.
 * the scheduler is called only once when in enabled mode. If it is not enabled, it returns after all tasks are completed.
 * After the tasks-array is completely went through, the scheduler goes to sleep in LPM0 with interrupts.
 * it is woken by the timerA0-interrupt, which must be enabled prior call!
 */
__EXTERN_C
void scheduler();


#endif /* TASK_H_ */
