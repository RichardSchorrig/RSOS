/*
 * LongPressButton.h
 *
 *  Created on: 10.03.2017
 *      Author: Richard
 */

#ifndef INPUT_LONGPRESSBUTTON_H_
#define INPUT_LONGPRESSBUTTON_H_

#include <RSOSDefines.h>
#ifdef MAXLONGPRESSBUTTONS

#include "Buttons.h"
#include "../Task.h"
#include "../WaitTimer.h"
#include <stdint.h>

/**
 * Long Press Button Structure
 * Fields:
 *  button: a reference to the underlying button, todo: change to number to save memory
 *  status: bit field containing:
 *      AERF CCCC
 *      A: is active
 *      E: (end) button is released, don't activate
 *      R: the long press task is repetitive, i.e. it is scheduled repeatedly instead of only once
 *      F: reduce the wait timer as cycles increase (1: reduce wait time after x cycles, 0: wait time remains)
 *      C: the number of cycles to pass before the wait time is reduced
 *  cycle: the current cycle (the time the button is pressed, or how many times the repetitive task has been scheduled
 *         also used for the reduced wait time
 *  shortPressTask: the task scheduled on short press (Button released when cycle is still 0)
 *  longPressTask: the task scheduled repeatedly or once after long press
 *
 *  MEMORY
 *      this structure takes up 6 Bytes
 */
typedef struct LongPressButton_t {
    Button* button;
    uint8_t status;
    uint8_t cycle;
    int8_t shortPressTask;
    int8_t longPressTask;

} LongPressButton;

/**
 * bit identifier: is Active
 */
#define LongPressButton_isActive 0x80

/**
 * bit identifier: is released
 */
#define LongPressButton_isReleased 0x40

/**
 * bit identifier: is repetitive
 */
#define LongPressButton_isRepetitive 0x20

/**
 * bit identifier: is decremental
 * @see setLongPressButton_decrementWaitTime() for explanation
 */
#define LongPressButton_isDecremental 0x10

/**
 * mask for the decremented wait time cycle
 * @see setLongPressButton_decrementWaitTime() for explanation
 */
#define LongPressButton_CycleMask 0x0F

extern LongPressButton longPressButton_mem[MAXLONGPRESSBUTTONS];
extern int8_t longPressButton_size;

/**
 * enables long press button operation
 * this function inits the task "task_LPbuttonWaitScheduler"
 * and the connected WaitTimer "timer_LPbuttonWaitScheduler"
 *
 * to operate, the following structures must be available:
 *      2x Task
 *      1x WaitTimer
 *
 * it is possible to add a multiplier to all button debounce times
 * this is done by the wait timer which controls the button wait scheduler
 *
 * the functionality of the clockMultiply value is similar to the one in
 * initButtonOperation(), but a higher multiplier is recommended
 */
__EXTERN_C
void initLongPressButtonOperation(uint16_t clockMultiply);

/**
 * inits a long press button
 *
 * this function does the same as initButton() in Button.h,
 * (setting up the port as input, interrupt edge select high to low etc.)
 * additionally a new LongPressButton structure is initialized and returned.
 *
 * Do not attempt to modify the field button!
 *
 * call initLongPressButtonOperation() first!
 *
 * make sure that enough Buttom Memory is available! Every LongPressButton also needs a Button!
 *
 * @param bit: the bit the pin is at
 * @param port: the port the pin is at
 * @param waitTime: the time that passes until the interrupt is enabled after interrupt occurs
 * @return: a reference to the new button
 */
__EXTERN_C
LongPressButton* initLPButton(uint8_t bit, volatile uint8_t * portRegister, uint8_t waitTime);

/**
 * adds a task to be scheduled when button is short pressed (released before cycle count is 1)
 * @param lpbutton the button to add the task to
 * @param task the task to add
 */
__EXTERN_C
void addShortPressTask_toLPButton(LongPressButton* lpbutton, Task* task);

/**
 * adds a task to be scheduled when button is pressed for long time (cycle count 1 and more)
 * the task can be scheduled repeatedly (as long as the button is held down, the task is scheduled)
 * or only once (after the long press time, the task is scheduled and the long press button becomes inactive)
 * @param lpbutton the button to add the task to
 * @param task the task to add
 * @param isRepetitive: whether or not the task is repeatedly scheduled (1: repetitive, 0: once)
 */
__EXTERN_C
void addLongPressTask_toLPButton(LongPressButton* lpbutton, Task* task, int8_t isRepetitive);

/**
 * decreases the long press button's wait time on press after xx cycles (only for repetitive task,
 * for one time task, value is ignored)
 * for example, to set the minutes on a clock, pressing and holding down the button will
 * change minutes slowly, but soon after the minutes will change faster
 * @param lpbutton the button to reduce wait time
 * @param cycles the number of cycles to pass before the wait time is decreased.
 *        each time the wait time becomes zero, the cycle value is counted up, and the repetitive task
 *        is scheduled. When the number of cycles match the set cycles, the wait time is reduced (divided by 2)
 *        allowed values are: 0..15
 */
__EXTERN_C
void setLongPressButton_decrementWaitTime(LongPressButton* lpbutton, uint8_t cycles);

/**
 * the task function called on press for all Buttons that are LongPressButtons
 * disables the active bit in Button, enables the active bit in LongPressButton
 */
__EXTERN_C
void longPressButton_Enable();

/**
 * the long press button wait scheduler
 * takes care of cycles, task etc connected to the long press buttons
 * checks if button is still pressed, on release, reactivates button to be taken care of by the buttonScheduler
 * (which enables the button interrupt again when released)
 */
__EXTERN_C
void longPressButtonWaitScheduler();

#endif /* MAXLONGPRESSBUTTONS */

#endif /* INPUT_LONGPRESSBUTTON_H_ */
