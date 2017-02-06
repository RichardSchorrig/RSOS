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
 */

#ifndef TASK_H_
#define TASK_H_

#include "Path.h"
#include PATH_RSOSDEFINES_H

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
 * 	  							0001: 2
 * 	  							0010: 3
 * 	  							...
 * 	  							1110: 15
 * 	  							1111: infinite  //todo!
 * 	  W: number of delay cycles:
 * 	  							0001: 1
 * 	  							0010: 2
 * 	  							...
 * 	  							1110: 14
 * 	  							1111: 15    //todo!
 * 	  PPPP: Priority from 0 to 15
 *
 * 	currentDelay: delay counter when executed
 * 	currentCycle: cycle counter when executed
 * 	followUpTask: more task structures
 *
 *
 * MEMORY:
 *  This structure takes up 8 Bytes
 */
typedef struct Task_t {
	TaskFunction* task;
	volatile unsigned int status;
	volatile unsigned char currentDelay;
	volatile unsigned char currentCycle;
	struct Task_t* *followUpTask;
} Task;

extern signed char tasks_size;  //todo: remove extern
extern Task task_mem[MAXTASKS];
//extern Task* task_mem;

extern unsigned char numberOfRunningTasks;  //todo: remove extern, add encapsulation
extern unsigned char schedulerEnabled;      //todo: remove extern, add encapsulation

extern unsigned char currentRunningTask;    //todo: remove extern, add encapsulation

extern unsigned char currentPriority;       //todo: remove extern, add encapsulation

/**
 * adds a task to the task array
 * @param unsigned int priority: defines the priority of this task. if the task becomes active and has higher
 * 									 priority than other active tasks, those other tasks are not executed
 * @param TaskFunction* taskfunction: a pointer to a function, which should be executed when task is active
 * @return: a reference to the added task
 */
Task* addTask(unsigned char priority, TaskFunction* taskfunction);

/**
 * adds a task that is executed when the parents task is completed
 * it is possible to add up to three following tasks to one task
 * @param task: the position of the parent task in the tasks-array
 * @param followUpArray: a pointer to an initialized Array where the followup tasks are stored.
 *        might be null, but only if a valid array was added prior
 * @param followUpTask: the position of the following task in the tasks-array
 */
void addFollowUpTask(Task* task, Task* *followUpArray, Task* followUpTask);

/**
 * sets a task cyclic, so it is executed more than once if it becomes active
 * @param task: the position of the task in the tasks-array
 * @param cycles: the number of cycles to be executed (2 to 9 cycles)
 */
void setTaskCyclic(Task* task, char cycles);

/**
 * sets a delay to a task, after a task becomes active, the scheduler waits the given number of cycles before the task is executed
 * @param task: the position of the task in the tasks-array
 * @param delay: the number of cycles the scheduler waits (1 to 8 delay cycles)
 */
void setTaskDelay(Task* task, char delay);

/**
 * saves the current running task to make a context change
 */
void saveCurrentContext();

/**
 * restores the previous saved context
 */
void restoreCurrentContext();

/**
 * sets a task active, it is executed when the scheduler is working
 * @param task: pointer to the task that should be scheduled
 */
void scheduleTask(struct Task_t* task);

/**
 * sets the scheduler enabled, the scheduler is running continously
 */
void enableScheduler();

/**
 * stops the scheduler, the scheduler will finish the remaining active tasks
 */
void disableScheduler();

/**
 * the scheduler handles the tasks. they are currently executed in reverse order they where added.
 * the scheduler is called only once when in enabled mode. If it is not enabled, it returns after all tasks are completed.
 * After the tasks-array is completely went through, the scheduler goes to sleep in LPM0 with interrupts.
 * it is woken by the timerA0-interrupt, which must be enabled prior call!
 */
void scheduler();


#endif /* TASK_H_ */
