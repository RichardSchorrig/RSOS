/*
 * Stepper_ShiftRegister.c
 *
 *  Created on: 18.04.2017
 *      Author: Richard
 */

#include "Stepper_ShiftRegister.h"

#ifdef STEPPER_SHIFTREGISTER

static SPIOperation* stepperShiftRegister = 0;
static uint8_t totalNumberOfBytes = 0;
Task* g_task_stepperScheduler = 0;

static void stepTask();

void initStepperOperation(SPIOperation* operation, uint8_t numOfBytes)
{
    stepperShiftRegister = operation;
    totalNumberOfBytes = numOfBytes;
    g_task_stepperScheduler = addTask(STEPPER_SCHEDULERPRIORITY, stepTask);
    setTaskCyclic(g_task_stepperScheduler, 2);
}

Stepper* initStepper(unsigned char shiftregisterPosition)
{
    if (stepper_size < MAXSTEPPER)
    {
        stepper_mem[stepper_size].registerPosition.position = shiftregisterPosition;
        stepper_mem[stepper_size].position_motor = 0;
        stepper_mem[stepper_size].position_needle = 0;
        stepper_mem[stepper_size].position_needle_current = 0;
        stepper_size += 1;
    }

    return &stepper_mem[stepper_size - 1];
}

static inline void toggleShiftRegister(Stepper* stepper) __attribute__((always_inline));
static inline void toggleShiftRegister(Stepper* stepper)
{
    unsigned char move = 0;
    unsigned char shiftregNr = (stepper->registerPosition.position - 1) >> 1;
    if ((stepper->registerPosition.position) & 0x01)
    {
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] &= ~(STEPPER_Bit_Mask << 4);
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] |= (stepper->position_motor & STEPPER_Bit_Mask) << 4;
    }
    else
    {
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] &= ~(STEPPER_Bit_Mask << 0);
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] |= (stepper->position_motor & STEPPER_Bit_Mask) << 0;
    }
}

void rotate(Stepper* stepper)
{
    unsigned char pos = stepper->position_motor & STEPPER_Bit_Mask;

#ifdef STEPPER_SINGLEPHASE
    if (stepper->position_motor & STEPPER_Counterclockwise)
    {
        switch (pos) {
        case 1: pos = 8; stepper->position_needle_current -= 1; break;
        case 7: pos = 1; break;
        case 8: pos = 14; break;
        case 14: pos = 7; break;
        default: pos = 8; break;
        }
    }
    else
    {
        switch (pos) {
        case 1: pos = 7; break;
        case 7: pos = 14; break;
        case 8: pos = 1; stepper->position_needle_current += 1; break;
        case 14: pos = 8; break;
        default: pos = 1; break;
        }
    }
#else
    //new dual phase drive
    if (stepper->position_motor & STEPPER_Counterclockwise)
    {
        switch (pos) {
        case 5: pos = 9; stepper->position_needle_current -= 1; break;
        case 6: pos = 5; break;
        case 9: pos = 10; break;
        case 10: pos = 6; break;
        default: pos = 9; break;
        }
        /**
         * 0101 -> 1001 -> 1010 -> 0110 -> 0101
         */
    }
    else
    {
        switch (pos) {
        case 5: pos = 6; break;
        case 6: pos = 10; break;
        case 9: pos = 5; stepper->position_needle_current += 1; break;
        case 10: pos = 9; break;
        default: pos = 5; break;
        }
    }
    stepper->position_motor &= ~STEPPER_Bit_Mask;
    stepper->position_motor |= pos & STEPPER_Bit_Mask;

#endif /* STEPPER_SINGLEPHASE */
}

static void stepTask()
{
    static int8_t noSROperation = 0;
    signed char i = stepper_size - 1;
    unsigned char no_motors = 0;
    if (noSROperation != -1)
    {
        for (; i>=0; i-=1)
        {
            if (stepper_mem[i].position_motor & STEPPER_isActive)
            {
                if (stepper_mem[i].position_needle != stepper_mem[i].position_needle_current)
                {
                    rotate(&stepper_mem[i]);
                }
                else
                {
                    stepper_mem[i].position_motor &= ~STEPPER_Bit_Mask;
                    stepper_mem[i].position_motor &= ~STEPPER_isActive;
                }
                toggleShiftRegister(&stepper_mem[i]);
            }
            else
            {
                no_motors += 1;
            }
        }
    }

    if (no_motors == stepper_size)
    {
        // signal scheduler to stop running this task, all motors are at the desired position
        task_mem[currentRunningTask].currentCycle = 0;
    }
    else
    {
        // add some cycle (it doesn't matter how many cycles since it is repeated and checked each time
        task_mem[currentRunningTask].currentCycle = 2;
        noSROperation = SPI_activateSPIOperation(stepperShiftRegister, 1);
    }
}

#endif /* STEPPER_SHIFTREGISTER */
