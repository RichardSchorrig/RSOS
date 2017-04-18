/*
 * Stepper_Direct.c
 *
 *  Created on: 17.04.2017
 *      Author: Richard
 */

#include "Stepper_Direct.h"

#ifdef STEPPER_DIRECT

Stepper* initStepper(unsigned char port, unsigned char pin0, unsigned char pin1, unsigned char pin2, unsigned char pin3)
{
    if (stepper_size < MAXSTEPPER)
    {
        if (port == 1)
        {
            P1DIR |= pin0 | pin1 | pin2 | pin3;
            P1OUT &= ~(pin0 | pin1 | pin2 | pin3);
        }
        else if (port == 2)
        {
            P2DIR |= pin0 | pin1 | pin2 | pin3;
            P2OUT &= ~(pin0 | pin1 | pin2 | pin3);
        }
        stepper_mem[stepper_size].pins.port = port;
        stepper_mem[stepper_size].pins.pin0 = pin0;
        stepper_mem[stepper_size].pins.pin1 = pin1;
        stepper_mem[stepper_size].pins.pin2 = pin2;
        stepper_mem[stepper_size].pins.pin3 = pin3;
        stepper_mem[stepper_size].position_motor = 0;
        stepper_mem[stepper_size].position_needle = 0;
        stepper_mem[stepper_size].position_needle_current = 0;

        stepper_size += 1;
    }

    return &stepper_mem[stepper_size - 1];
}

void rotate(Stepper* stepper)
{
    unsigned char pos = stepper->position_motor & STEPPER_PINS_MASK;

#ifdef STEPPER_SINGLEPHASE
    if (stepper->position_motor & STEPPER_COUNTERCLOCKWISE)
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
    if (stepper->position_motor & STEPPER_COUNTERCLOCKWISE)
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
#endif /* STEPPER_SINGLEPHASE */
    stepper->position_motor &= ~STEPPER_PINS_MASK;
    stepper->position_motor |= pos & STEPPER_PINS_MASK;
}

void togglePins(Stepper* stepper)
{
    if (stepper->pins.port == 1)
    {
        if (stepper->position_motor & STEPPER_PIN0)
            P1OUT |= stepper->pins.pin0;
        else
            P1OUT &= ~stepper->pins.pin0;

        if (stepper->position_motor & STEPPER_PIN1)
            P1OUT |= stepper->pins.pin1;
        else
            P1OUT &= ~stepper->pins.pin1;

        if (stepper->position_motor & STEPPER_PIN2)
            P1OUT |= stepper->pins.pin2;
        else
            P1OUT &= ~stepper->pins.pin2;

        if (stepper->position_motor & STEPPER_PIN3)
            P1OUT |= stepper->pins.pin3;
        else
            P1OUT &= ~stepper->pins.pin3;
    }
    else if (stepper->pins.port == 2)
    {
        if (stepper->position_motor & STEPPER_PIN0)
            P2OUT |= stepper->pins.pin0;
        else
            P2OUT &= ~stepper->pins.pin0;

        if (stepper->position_motor & STEPPER_PIN1)
            P2OUT |= stepper->pins.pin1;
        else
            P2OUT &= ~stepper->pins.pin1;

        if (stepper->position_motor & STEPPER_PIN2)
            P2OUT |= stepper->pins.pin2;
        else
            P2OUT &= ~stepper->pins.pin2;

        if (stepper->position_motor & STEPPER_PIN3)
            P2OUT |= stepper->pins.pin3;
        else
            P2OUT &= ~stepper->pins.pin3;
    }
}

void task_stepTask()
{
    signed char i = stepper_size - 1;
    unsigned char no_motors = 0;

    for (; i>=0; i-=1)
    {
        if (stepper_mem[i].position_motor & STEPPER_ACTIVE)
        {
            if (stepper_mem[i].position_needle != stepper_mem[i].position_needle_current)
            {
                rotate(&stepper_mem[i]);
            }
            else
            {
                stepper_mem[i].position_motor &= ~STEPPER_PINS_MASK;
                stepper_mem[i].position_motor &= ~STEPPER_ACTIVE;
            }
            togglePins(&stepper_mem[i]);
        }
        else
        {
            no_motors += 1;
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
    }
}

#endif /* STEPPER_DIRECT */
