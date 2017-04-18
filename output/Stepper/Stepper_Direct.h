/*
 * Stepper_Direct.h
 *
 *  Created on: 17.04.2017
 *      Author: Richard
 */

#ifndef OUTPUT_STEPPER_STEPPER_DIRECT_H_
#define OUTPUT_STEPPER_STEPPER_DIRECT_H_

#include "../Stepper.h"

#ifdef STEPPER_DIRECT

/**
 * Task for the stepper scheduler, must be set cyclic
 * defined in Stepper_Direct.c
 */
extern Task* g_task_stepperScheduler;

typedef struct stepper_pins_t{
    volatile uint8_t* port;
    uint8_t pin0;
    uint8_t pin1;
    uint8_t pin2;
    uint8_t pin3;
} stepperPins;

typedef struct stepper_t{
    stepperPins pins;
    uint8_t position_motor; /** the position of the motor (down, left, up, right), additionally saves a direction indicator */
    uint8_t position_needle; /** the destination of the needle to be */
    uint8_t position_needle_current; /** the actual position of the needle */
} Stepper;

extern Stepper stepper_mem[MAXSTEPPER];
extern int8_t stepper_size;

/**
 * inits a step motor
 *
 * the motor must be connected to 4 pins of the controller
 * the first two control coil 1 of the motor
 * the second two control coil 2 of the motor
 * the pins must belong to one port
 *
 * there are two versions of this function, depending on the definition of STEPPER_SHIFTREGISTER
 *
 * if defined STEPPER_SHIFTREGISTER
 * @param shiftregisterPosition: the position the motor is connected to the shift register.
 * A shift register usually has 8 pins, giving 2 possible positions for one motor. When shift registers are chained,
 * the position can be more than "2".
 *
 * else
 * @param port: the port of the pins the motor is connected to, initializes the given pins as outputs
 * @param pinX: all the pin numbers
 * endif
 *
 * @return: a reference to the new stepper
 */
Stepper* initStepper(volatile unsigned char* port, unsigned char pin0, unsigned char pin1, unsigned char pin2, unsigned char pin3);

/**
 * turn the motor's needle to a given position
 * @param stepper: the stepper motor to control
 * @param steps: the absolute position to turn the needle to
 */
static inline void step_absolute(Stepper* stepper, uint8_t steps) __attribute((always_inline));
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

    stepper->position_motor |= STEPPER_isActive;
    scheduleTask(g_task_stepperScheduler);
}

/**
 * turns the motor's needle some steps in a direction
 * @param stepper: the stepper motor to control
 * @param direction: the direction to turn: -1 to turn counterclockwise, +1 to turn clockwise
 * @param steps: the number of steps to turn
 */
static inline void step_relative(Stepper* stepper, int8_t direction, uint8_t steps) __attribute((always_inline));
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


#endif /* STEPPER_DIRECT */

#endif /* OUTPUT_STEPPER_STEPPER_DIRECT_H_ */
