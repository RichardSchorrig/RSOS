/*
 * Stepper.c
 *
 *  Created on: 07.05.2016
 *      Author: Richard
 */

#include "Stepper.h"

/* exclude everything if not used */
#ifdef MAXSTEPPER

uint8_t stepper_buffer[STEPPER_BUFFERSIZE];
BufferBuffer_uint8* buffer;

#ifdef STEPPER_SHIFTREGISTER
void initStepperOperation(volatile uint8_t * port, uint8_t pin)
{
    Buffer_uint8* buf = (Buffer_uint8*) initBuffer((void*) stepper_buffer, STEPPER_BUFFERSIZE);
    buffer = (BufferBuffer_uint8*) initBuffer((void*) &buf, 1);
	stepperShiftRegister = SR_initShiftRegister(pin, port, buffer, STEPPER_BUFFERSIZE);
	stepTask = addTask(0, task_stepTask);
	setTaskCyclic(stepTask, 4);
}
#endif /* STEPPER_SHIFTREGISTER */

#ifdef STEPPER_SHIFTREGISTER
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
#else
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
#endif /* STEPPER_SHIFTREGISTER */

void step_relative(Stepper* stepper, signed char direction, unsigned char steps)
{
	if (direction == -1)
	{
		if ((stepper->position_needle - steps) < 0)
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

void step_absolute(Stepper* stepper, unsigned char steps)
{

	if (steps > STEPPER_MAXSTEPS)
		steps = STEPPER_MAXSTEPS;
	stepper->position_needle = steps;

	if (stepper->position_needle > stepper->position_needle_current)
	{
		if (stepper->position_motor & STEPPER_COUNTERCLOCKWISE)
		{
			if (stepper->position_motor & STEPPER_PINS_MASK)
			{
				stepper->position_needle_current -= 1;
			}
		}
		stepper->position_motor &= ~STEPPER_COUNTERCLOCKWISE;
	}
	else
	{
		if ( (stepper->position_motor & STEPPER_COUNTERCLOCKWISE) == 0)
		{
			if (stepper->position_motor & STEPPER_PINS_MASK)
			{
				stepper->position_needle_current += 1;
			}
		}
		stepper->position_motor |= STEPPER_COUNTERCLOCKWISE;
	}



	stepper->position_motor |= STEPPER_ACTIVE;
	scheduleTask(stepTask);
}

#ifdef STEPPER_SHIFTREGISTER
inline void toggleShiftRegister(Stepper* stepper);
void toggleShiftRegister(Stepper* stepper)
{
	unsigned char move = 0;
	unsigned char shiftregNr = (stepper->registerPosition.position - 1) >> 1;
	if ((stepper->registerPosition.position - 1) & 0x01)
	{
		move = 4;
		stepper_buffer[shiftregNr] &= 0x0F;
	}
	else
	{
		stepper_buffer[shiftregNr] &= 0xF0;
	}

	stepper_buffer[shiftregNr] |= (stepper->position_motor & STEPPER_PINS_MASK) << (move);
}
/*
void nextByte_ShiftRegister()
{
	stepper_bufferPosition += 1;
	if (stepper_bufferPosition >= STEPPER_BUFFERSIZE)
	{
		stepper_bufferPosition = 0;
	}
	else
	{
		UCA0TXBUF = stepper_buffer[stepper_bufferPosition];
	}
}
#else
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
*/
#endif /* STEPPER_SHIFTREGISTER */

void rotate(Stepper* stepper)
{
	unsigned char pos = stepper->position_motor & STEPPER_PINS_MASK;
/*
	if (stepper->position_motor & STEPPER_SECTOR_HIGH)
	{
		if (stepper->position_motor & STEPPER_COUNTERCLOCKWISE)
		{
			pos = (pos >> 1) + 8;
			if ((pos & STEPPER_PINS_MASK) == 0x0F)
			{
				stepper->position_motor &= ~STEPPER_SECTOR_HIGH;
			}
		}
		else
		{
			pos = (pos + pos) - 16;
			if ((pos & STEPPER_PINS_MASK) == 0x00)
			{
				stepper->position_motor &= ~STEPPER_SECTOR_HIGH;
				stepper->position_needle_current += 1;
			}
		}
	}
	else
	{
		if (stepper->position_motor & STEPPER_COUNTERCLOCKWISE)
		{
			pos = pos >> 1;
			if ((pos & STEPPER_PINS_MASK) == 0x00)
			{
				stepper->position_motor |= STEPPER_SECTOR_HIGH;
				stepper->position_needle_current -= 1;
			}
		}
		else
		{
			pos = pos + pos + 1;
			if ((pos & STEPPER_PINS_MASK) == 0x0F)
			{
				stepper->position_motor |= STEPPER_SECTOR_HIGH;
			}
		}
	}
*/
/* old single phase drive
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
*/
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
	stepper->position_motor &= ~STEPPER_PINS_MASK;
	stepper->position_motor |= pos & STEPPER_PINS_MASK;

#ifdef STEPPER_SHIFTREGISTER
//	toggleShiftRegister(stepper);
#else
//	togglePins(stepper);
#endif /* STEPPER_SHIFTREGISTER */
}

void task_stepTask()
{
	static int8_t noSROperation = 0;
	signed char i = stepper_size - 1;
	unsigned char no_motors = 0;
	if (noSROperation != -1)
	{
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
			#ifdef STEPPER_SHIFTREGISTER
				toggleShiftRegister(&stepper_mem[i]);
			#else
				togglePins(&stepper_mem[i]);
			#endif /* STEPPER_SHIFTREGISTER */
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
	#ifdef STEPPER_SHIFTREGISTER
		noSROperation = SR_activateShiftRegister(stepperShiftRegister, 1);
	#else
	#endif /* STEPPER_SHIFTREGISTER */
	}
}

#endif /* MAXSTEPPER */

