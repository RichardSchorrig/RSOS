/*
 * LongPressButton.c
 *
 *  Created on: 10.03.2017
 *      Author: Richard
 */

#include "LongPressButton.h"

#ifdef MAXLONGPRESSBUTTONS

static Task* task_enableLPB = 0;

void initLongPressButtonOperation(uint16_t clockMultiply) {
    task_enableLPB = addTask(0, longPressButton_Enable);

    Task* task_scheduler = addTask(0, longPressButtonWaitScheduler);
    WaitTimer* timer_scheduler = initWaitTimer(clockMultiply);
    setTaskOnStop(timer_scheduler, task_scheduler);
    setTimerCyclic(timer_scheduler);
    setTimer(timer_scheduler);
}

LongPressButton* initLPButton(uint8_t bit, volatile uint8_t * portRegister, uint8_t waitTime) {
    longPressButton_mem[longPressButton_size].button = initButton(bit, portRegister, waitTime);
    addTaskOnPressToButton(longPressButton_mem[longPressButton_size].button, task_enableLPB);
    longPressButton_mem[longPressButton_size].cycle = 0;
    longPressButton_mem[longPressButton_size].longPressTask = -1;
    longPressButton_mem[longPressButton_size].shortPressTask = -1;
    longPressButton_mem[longPressButton_size].status = 0;

    longPressButton_size += 1;
    return &longPressButton_mem[longPressButton_size-1];
}

void addShortPressTask_toLPButton(LongPressButton* lpbutton, Task* task) {
    int8_t taskNr = getTaskNumber(task);
    if (taskNr >= 0) {
        lpbutton->shortPressTask = taskNr;
    }
}

void addLongPressTask_toLPButton(LongPressButton* lpbutton, Task* task, int8_t isRepetitive) {
    int8_t taskNr = getTaskNumber(task);
    if (taskNr >= 0) {
        lpbutton->longPressTask = taskNr;

        if (isRepetitive == 1) {
            lpbutton->status |= LongPressButton_isRepetitive;
        }
    }
}

void setLongPressButton_decrementWaitTime(LongPressButton* lpbutton, uint8_t cycles) {
    lpbutton->status |= LongPressButton_isDecremental;
    lpbutton->status |= (cycles & LongPressButton_CycleMask);
}

static inline void longPressButton_setWaitTime(LongPressButton* btn);
static void longPressButton_setWaitTime(LongPressButton* btn) {
    Button_setWaitTime(btn->button);
    if (btn->status & LongPressButton_isDecremental)
    {
        if ( btn->cycle > (btn->status & LongPressButton_CycleMask) )
        {
            btn->button->currentWaitTime >>= 2;
        }
    }
}

void longPressButton_Enable() {
    int8_t i = longPressButton_size;
    for (; i>0; i-= 1) {
        LongPressButton* btn = &longPressButton_mem[i-1];
        if (btn->button->status & Button_isActive)
        {
            if (~btn->status & LongPressButton_isReleased)
            {
                btn->status |= LongPressButton_isActive;
                btn->button->status &= ~Button_isActive;
                btn->cycle = 0;
                longPressButton_setWaitTime(btn);
            }
        }
    }
}

static inline void longPressButton_Disable(LongPressButton* btn);
static void longPressButton_Disable(LongPressButton* btn) {
    btn->button->status |= Button_isActive;
    btn->status &= ~LongPressButton_isActive;
    btn->status |= LongPressButton_isReleased;
}

void longPressButtonWaitScheduler() {
    int8_t i = longPressButton_size;
    for (; i>0; i-= 1) {
        LongPressButton* btn = &longPressButton_mem[i-1];

        if (btn->status & LongPressButton_isActive)
        {

            if (*(btn->button->port) & btn->button->bit)    //button released
            {
                if (btn->cycle == 0)                        //pressed for short time
                {
                    if (btn->shortPressTask != -1)
                    {
                        scheduleTask(&task_mem[btn->shortPressTask]);
                    }
                }
                longPressButton_Disable(btn);
            }
            else                                            //button still pressed
            {
                if (btn->button->currentWaitTime == 0)
                {
                    if (~btn->cycle & 0x10)                 //cycle smaller than 0x10: 16
                    {
                        btn->cycle += 1;
                    }
                    if (btn->longPressTask != -1)           //schedule longPressTask
                    {
                        scheduleTask(&task_mem[btn->longPressTask]);
                    }
                    if (btn->status & LongPressButton_isRepetitive)
                    {
                        longPressButton_setWaitTime(btn);
                    }
                    else
                    {
                        longPressButton_Disable(btn);
                    }
                }
                else
                {
                    btn->button->currentWaitTime -= 1;
                }
            }

        }
        else
        {
            if (~btn->button->status & Button_isActive)
            {
                btn->status &= ~LongPressButton_isReleased;
            }
        }
    }

}

#endif /* MAXLONGPRESSBUTTONS */


