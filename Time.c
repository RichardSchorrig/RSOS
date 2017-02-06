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
static const unsigned int connectedButtonMask = 0x0007;
static const unsigned int buttonConnected = 0x0008;
static const unsigned int taskIsOnStart = 0x2000;
static const unsigned int taskIsOnEnd = 0x1000;
static const unsigned int waitTimeMask = 0x0FF0;
static const unsigned int isCyclicTimer = 0x4000;
static const unsigned int isActive = 0x8000;

WaitTimer* initWaitTimer(unsigned char waitTime)
{
	waitTimers_mem[timers_size].status = 0;
	waitTimers_mem[timers_size].status |= (waitTime << 4) & waitTimeMask;
	waitTimers_mem[timers_size].currentWaitTime = 0;
	timers_size += 1;
	return &waitTimers_mem[timers_size-1];
}

void addButtonToTimer(WaitTimer* waitTimer, unsigned char buttonNr)
{
	waitTimer->status |= buttonConnected | (buttonNr & connectedButtonMask);
}

void setTimer(WaitTimer* waitTimer)
{
//	if (waitTimer->status & taskIsOnStart)
    if (waitTimer->taskOnStart != 0)
	{
		scheduleTask(waitTimer->taskOnStart);
	}
	waitTimer->currentWaitTime = (waitTimer->status & waitTimeMask) >> 4;
	waitTimer->status |= isActive;
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
	waitTimer->taskOnStart = task;
	waitTimer->status |= taskIsOnStart;
}
void setTaskOnStop(WaitTimer* waitTimer, Task* task)
{
	waitTimer->taskOnStop = task;
	waitTimer->status |= taskIsOnEnd;
}

void setNewWaitTime(unsigned char waitTime, WaitTimer* waitTimer)
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
	for (i=timers_size-1; i>=0; i--)
	{
		wT = &waitTimers_mem[i];
		if (wT->status & isActive)
		{
			if (wT->currentWaitTime != 0 && wT->currentWaitTime != 255)
			{
				{
					wT->currentWaitTime -= 1;
				}
			}
			else
			{
//				if (wT->status & buttonConnected)
			    if (wT->connectedButton != 0)
				{
					signed char btn = (wT->status & connectedButtonMask);
					/*
					if (buttons_mem[btn].port == 1)				//checks if buttons still pressed & enables interrupt only on release
					{
						if (P1IN & buttons_mem[btn].bit)
						{
							if (wT->currentWaitTime == 255)
								setTimer(wT);
							else
							{
								enableBtnInterrupt(&buttons_mem[wT->status & connectedButtonMask]);
								scheduleTask(wT->taskOnStop);
								wT->status &= ~isActive;
							}
						}
						else
						{
							wT->currentWaitTime = 255;
							wT->status |= isActive;
						}
					}
					else if (buttons_mem[btn].port == 2)
					{
						if (P2IN & buttons_mem[btn].bit)
						{
							if (wT->currentWaitTime == 255)
								setTimer(wT);
							else
							{
								enableBtnInterrupt(&buttons_mem[wT->status & connectedButtonMask]);
								scheduleTask(wT->taskOnStop);
								wT->status &= ~isActive;
							}
						}
						else
						{
							wT->currentWaitTime = 255;
							wT->status |= isActive;
						}
					}
					*/
					if (*(wT->connectedButton->port) & wT->connectedButton->bit) {
//					if (*buttons_mem[btn].port & buttons_mem[btn].bit) {
					    if (wT->currentWaitTime == 255)
                            setTimer(wT);
                        else
                        {
                            enableBtnInterrupt(&buttons_mem[wT->status & connectedButtonMask]);
                            scheduleTask(wT->taskOnStop);
                            wT->status &= ~isActive;
                        }
					}
                    else
                    {
                        wT->currentWaitTime = 255;
                        wT->status |= isActive;
                    }
				}
				else
				{
//					if (wT->status & taskIsOnEnd && wT->currentWaitTime == 0)
				    if (wT->taskOnStop != 0)
					{
						scheduleTask(wT->taskOnStop);
					}
					if (wT->status & isCyclicTimer)
					{
						setTimer(wT);
					}
					else
					{
						wT->status &= ~isActive;
					}
				}

			}
		}
	}
}

