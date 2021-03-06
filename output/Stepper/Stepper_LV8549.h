/*
 * Stepper_LV8549.h
 *
 *  Created on: 17.04.2017
 *      Author: Richard
 *
 * This file drives the LV8549MC Stepper Motor Driver IC
 * The driver ic is connected via a shift register:
 * MCU=|    |=Shift Register|
 * SIMO|--->|SIMO           |    |=LV8549MC=|
 *  CLK|--->|CLK          Q1|--->|IN2   OUT1|---> Coil1
 *          |             Q2|--->|IN1   OUT2|---> Coil1
 *          |             Q3|--->|ENA   OUT3|---> Coil2
 *          |            ...|    |      OUT4|---> Coil2
 *          |===============|    |==========|
 */

#ifndef OUTPUT_STEPPER_STEPPER_LV8549_H_
#define OUTPUT_STEPPER_STEPPER_LV8549_H_

#include "../Stepper.h"

#ifdef STEPPER_LV8549

#include "../../SerialInterface/SPIOperation.h"

#define STEPPER_IN2 (0x01 << 0)
#define STEPPER_IN1 (0x01 << 1)
#define STEPPER_ENA (0x01 << 2)
#define STEPPER_SRBit_Mask (0x07)

#define STEPPER_isActive (0x01 << 7)
#define STEPPER_Counterclockwise (0x01 << 6)

#define STEPPER_Bit_Mask (0x03)
#define STEPPER_Bit0 (0x01 << 0)
#define STEPPER_Bit1 (0x01 << 1)

typedef struct stepper_t{
    /** the position in the shift register (chain) */
    uint8_t shiftRegisterPosition;

    /**
     * position_motor explained:
     * bit field containing the following information
     *  AxxD xE21
     *      A: is active
     *      D: direction (0 clockwise, 1: counterclockwise)
     *      E: enable
     *      21: Bits to step the motor
     *          11 -> 10 -> 00 -> 01 -> 11
     *           3     2     0     1     3
     */
    uint8_t position_motor;

    /** the position of the needle to be */
    uint8_t position_needle;

    /** the actual position of the needle */
    uint8_t position_needle_current;
} Stepper;

extern Stepper stepper_mem[MAXSTEPPER];
extern int8_t stepper_size;

/**
 * Task for the stepper scheduler, must be set cyclic
 * defined in Stepper_LV8549.c
 */
extern Task* g_task_stepperScheduler;

/**
 * inits the stepper operation.
 * Must be called in order to step anything
 * Ensure to call SR_initOperation() first.
 *
 * to operate, 1x Task is needed
 *
 * @param operation: an initialized SPIOperation pointer, which is used to activate the stepper
 * @param dataBuffer: the buffer connected to the SPIOperation's buffer, the buffer is modified
 *                    during the stepper operation, but can be shared and modified if other periphery
 *                    is connected to the shift register
 * @param numberOfBytes: the length of dataBuffer
 * @param delayTicks: the stepper task's delay. The task will be delayed for the number of ticks.
 */
void initStepperOperation(SPIOperation* operation, uint8_t* dataBuffer, uint8_t numberOfBytes, uint8_t delayTicks);

/**
 * inits a step motor
 *
 * @param shiftregisterPosition: the position of the motor in the shift register chain
 * @return: a reference to the new stepper
 */
Stepper* initStepper(uint8_t shiftregisterPosition);

/**
 * turn the motor's needle to a given position
 * @param stepper: the stepper motor to control
 * @param steps: the absolute position to turn the needle to
 */
static inline void step_absolute(Stepper* stepper, uint8_t steps) __attribute__((always_inline));
static inline void step_absolute(Stepper* stepper, uint8_t steps)
{
    if (steps > STEPPER_MAXSTEPS)
    {
        steps = STEPPER_MAXSTEPS;
    }
    stepper->position_needle = steps;

    if (stepper->position_needle > stepper->position_needle_current)
    {
        if (stepper->position_motor & STEPPER_Counterclockwise)
        {
            if (stepper->position_motor & STEPPER_Bit_Mask)
            {
                stepper->position_needle_current -= 1;
            }
        }
        stepper->position_motor &= ~STEPPER_Counterclockwise;
    }
    else
    {
        if (!(stepper->position_motor & STEPPER_Counterclockwise))
        {
            if (stepper->position_motor & STEPPER_Bit_Mask)
            {
                stepper->position_needle_current += 1;
            }
        }
        stepper->position_motor |= STEPPER_Counterclockwise;
    }

    stepper->position_motor |= STEPPER_isActive | STEPPER_ENA;
    scheduleTask(g_task_stepperScheduler);
}

/**
 * turns the motor's needle some steps in a direction
 * @param stepper: the stepper motor to control
 * @param direction: the direction to turn: -1 to turn counterclockwise, +1 to turn clockwise
 * @param steps: the number of steps to turn
 */
static inline void step_relative(Stepper* stepper, int8_t direction, uint8_t steps) __attribute__((always_inline));
static inline void step_relative(Stepper* stepper, int8_t direction, uint8_t steps)
{
    if (direction == -1)
    {
        if ((stepper->position_needle <= steps))
            step_absolute(stepper, 0);
        else
            step_absolute(stepper, stepper->position_needle - steps);
    }
    else if (direction == 1)
    {
        if ((stepper->position_needle + steps) > STEPPER_MAXSTEPS)
            step_absolute(stepper, STEPPER_MAXSTEPS);
        else
            step_absolute(stepper, stepper->position_needle + steps);
    }
}

static inline void force_stepperOutput(Stepper* stepper) __attribute__((always_inline));
static inline void force_stepperOutput(Stepper* stepper)
{
    if (!(stepper->position_motor & STEPPER_isActive))
    {
        stepper->position_motor |= STEPPER_isActive;
        scheduleTask(g_task_stepperScheduler);
    }
}

#endif /* STEPPER_LV8549 */

#endif /* OUTPUT_STEPPER_STEPPER_LV8549_H_ */
