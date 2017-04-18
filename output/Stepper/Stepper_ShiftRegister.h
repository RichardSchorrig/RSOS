/*
 * Stepper_ShiftRegister.h
 *
 *  Created on: 18.04.2017
 *      Author: Richard
 */

#ifndef OUTPUT_STEPPER_STEPPER_SHIFTREGISTER_H_
#define OUTPUT_STEPPER_STEPPER_SHIFTREGISTER_H_

#include "../Stepper.h"

#ifdef STEPPER_SHIFTREGISTER

#include "../../SerialInterface/SPIOperation.h"

typedef struct stepper_shiftregister_t{
    unsigned char position;
} stepperShiftregister;

typedef struct stepper_t{
    stepperShiftregister registerPosition;
    unsigned char position_motor; /** the position of the motor (down, left, up, right), additionally saves a direction indicator */
    unsigned char position_needle; /** the position of the needle to be */
    unsigned char position_needle_current; /** the actual position of the needle */
} Stepper;

/*
 * position_motor explained:
 * the first four bits indicate the pin voltage (0: 0V, 1:VCC) of the motor,
 * it either turns (voltage over one coil) or does not (both coil potentials are identical)
 *
 * 5th bit indicates the motor state to compute the next value of the pins state
 * 0000 -> 0001 -> 0011 -> 0111 -> 1111 -> 1110 -> 1100 -> 1000 -> 0000
 * (0: 0000-1111: xn = 2*x + 1; 1: 1111-0000: xn = 2*x - 16;)
 * when the value 1111 or 0000 are reached, the bit is toggled, the direction is maintained
 *
 * 6th bit determines the rotation direction: 0: clockwise, 1: counterclockwise. with bit 5 the pin states are calculated
 * 0: rules above apply
 * 1: 0000 <- 0001 <- 0011 <- 0111 <- 1111 <- 1110 <- 1100 <- 1000 <- 0000
 * bit 5 is 0 (0000-1111): xn = x > 1; 1 (1111-0000): 0.5*x + 8, xn =  x>1 + 8;
 * to change the turning direction, simply toggle bit 6
 *
 * 7th bit indicates the state: 0: inactive (no operation to be done), 1: active (look into position_needle and position_needle_current)
 */

#define STEPPER_isActive (0x01 << 7)
#define STEPPER_Counterclockwise (0x01 << 6)

#define STEPPER_PIN0 (0x01 << 0)
#define STEPPER_PIN1 (0x01 << 1)
#define STEPPER_PIN2 (0x01 << 2)
#define STEPPER_PIN3 (0x01 << 3)
#define STEPPER_Bit_Mask 0x0F
#define STEPPER_SECTOR_HIGH (0x01 << 5)

extern Stepper stepper_mem[MAXSTEPPER];
extern int8_t stepper_size;

/**
 * Task for the stepper scheduler, must be set cyclic
 * defined in Stepper_ShiftRegister.c
 */
extern Task* g_task_stepperScheduler;

/**
 * inits the stepper operation.
 * Must be called in order to step anything
 * Ensure to call SR_initOperation() first.
 * also make sure to have one task and one shift register operation structure available
 *
 * @param port: the port the strobe pin is at
 * @param pin: the bit for the strobe pin
 */
void initStepperOperation(SPIOperation* operation);

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
Stepper* initStepper(unsigned char shiftregisterPosition);


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

#endif /* STEPPER_SHIFTREGISTER */

#endif /* OUTPUT_STEPPER_STEPPER_SHIFTREGISTER_H_ */
