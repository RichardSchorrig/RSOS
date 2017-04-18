/*
 * Stepper_LV8549.c
 *
 *  Created on: 17.04.2017
 *      Author: Richard
 */

#include "Stepper_LV8549.h"

#ifdef STEPPER_LV8549

static SPIOperation* stepperShiftRegister = 0;
static uint8_t totalNumberOfBytes = 0;
Task* g_task_stepperScheduler = 0;

static void stepTask();

void initStepperOperation(SPIOperation* operation)
{
    stepperShiftRegister = operation;
    totalNumberOfBytes = operation->bufferbuffer->buffer[0]->size.size;
    g_task_stepperScheduler = addTask(STEPPER_SCHEDULERPRIORITY, stepTask);
    setTaskCyclic(g_task_stepperScheduler, 2);
}

Stepper* initStepper(uint8_t shiftregisterPosition)
{
    if (stepper_size < MAXSTEPPER)
    {
        stepper_mem[stepper_size].shiftRegisterPosition = shiftregisterPosition;
        stepper_mem[stepper_size].position_motor = 0;
        stepper_mem[stepper_size].position_needle = 0;
        stepper_mem[stepper_size].position_needle_current = 0;
        stepper_size += 1;
    }

    return &stepper_mem[stepper_size - 1];
}

static inline void Stepper_rotate(Stepper* stepper) __attribute__((always_inline));
static inline void Stepper_rotate(Stepper* stepper)
{
    uint8_t pos = stepper->position_motor & STEPPER_Bit_Mask;
    if (stepper->position_motor & STEPPER_Counterclockwise)
    {
        switch (pos) {
        case 0: pos = 2; break;
        case 1: pos = 0; break;
        case 2: pos = 3; stepper->position_needle_current -= 1; break;
        case 3: pos = 1; break;
        default: pos = 3; break;
        }
    }
    else
    {
        switch (pos) {
        case 0: pos = 1; break;
        case 1: pos = 3; stepper->position_needle_current += 1; break;
        case 2: pos = 0; break;
        case 3: pos = 2; break;
        default: pos = 3; break;
        }
    }
    stepper->position_motor &= ~STEPPER_Bit_Mask;
    stepper->position_motor |= pos & STEPPER_Bit_Mask;

    unsigned char shiftregNr = (stepper->shiftRegisterPosition) >> 1;
    if ((stepper->shiftRegisterPosition) & 0x01)
    {
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] &= ~(STEPPER_SRBit_Mask << 4);
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] |= (stepper->position_motor & STEPPER_SRBit_Mask) << 4;
    }
    else
    {
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] &= ~(STEPPER_SRBit_Mask << 0);
        stepperShiftRegister->bufferbuffer->buffer[0]->buffer[shiftregNr] |= (stepper->position_motor & STEPPER_SRBit_Mask) << 0;
    }
}

static void stepTask()
{
    static int8_t noSROperation = 0;
    int8_t i = stepper_size;
    uint8_t no_motors = 0;

    if (noSROperation != -1)
    {
        for (; i>0; i-=1)
        {
            if (stepper_mem[i-1].position_motor & STEPPER_isActive)
            {
                if (stepper_mem[i-1].position_needle != stepper_mem[i-1].position_needle_current)
                {
                    Stepper_rotate(&stepper_mem[i-1]);
                }
                else
                {
                    stepper_mem[i-1].position_motor &= ~STEPPER_ENA;
                    stepper_mem[i-1].position_motor &= ~STEPPER_isActive;
                }

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
        noSROperation = SPI_activateSPIOperation(stepperShiftRegister, totalNumberOfBytes);
    }
}

#endif /* STEPPER_LV8549 */