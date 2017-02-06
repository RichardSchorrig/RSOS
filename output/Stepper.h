/*
 * Stepper.h
 *
 *  Created on: 07.05.2016
 *      Author: Richard
 *
 * This header file is to support a bipolar stepper motor with 4 inputs.
 * The stepper motor can be connected to the controller directly, or with the help of a shift register.
 * It is not supported to connect motors both ways at the same time.
 *
 * The shift register is connected via SPI to the controller
 */

#ifndef STEPPER_H_
#define STEPPER_H_

/**
 * defines that all steppers are connected via a shift register instead of a direct connection
 */
#define STEPPER_SHIFTREGISTER

/**
 * maximum steps of a motor
 */
#define STEPPER_MAXSTEPS 160

#include "../Path.h"
#include <msp430.h>


#include PATH_RSOSDEFINES_H
#include "../Task.h"
#include "ShiftRegisterOperation.h"

typedef struct stepper_pins_t{
	unsigned char port;
	unsigned char pin0;
	unsigned char pin1;
	unsigned char pin2;
	unsigned char pin3;
} stepperPins;

typedef struct stepper_shiftregister_t{
	unsigned char position;
} stepperShiftregister;

typedef struct stepper_t{
#ifdef STEPPER_SHIFTREGISTER
	stepperShiftregister registerPosition;
#else
	stepperPins pins;
#endif /* STEPPER_SHIFTREGISTER */
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

#define STEPPER_PIN0 BIT0
#define STEPPER_PIN1 BIT1
#define STEPPER_PIN2 BIT2
#define STEPPER_PIN3 BIT3
#define STEPPER_PINS_MASK 0x0F
#define STEPPER_COUNTERCLOCKWISE BIT6
#define STEPPER_SECTOR_HIGH BIT5
#define STEPPER_ACTIVE BIT7

extern Stepper stepper_mem[MAXSTEPPER];
extern char stepper_size;

#ifdef STEPPER_SHIFTREGISTER
extern unsigned char stepper_buffer[STEPPER_BUFFERSIZE];
extern unsigned char stepper_bufferPosition;
ShiftRegisterOperation* stepperShiftRegister;
#endif /* STEPPER_SHIFTREGISTER */

/**
 * task needed to step the motor(s)
 * set cyclic!
 */
extern Task* stepTask;

extern Task* task_strobe;

/**
 * inits the stepper operation.
 * Must be called in order to step anything
 *
 * initializes a Shift Register Operation structure
 */
#ifdef STEPPER_SHIFTREGISTER
void initStepperOperation();
#endif /* STEPPER_SHIFTREGISTER */

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
#ifdef STEPPER_SHIFTREGISTER
	Stepper* initStepper(unsigned char shiftregisterPosition);
#else
	Stepper* initStepper(unsigned char port, unsigned char pin0, unsigned char pin1, unsigned char pin2, unsigned char pin3);
#endif /* STEPPER_SHIFTREGISTER */

/**
 * turns the motor's needle some steps in a direction
 * @param stepper: the stepper motor to control
 * @param direction: the direction to turn: -1 to turn counterclockwise, +1 to turn clockwise
 * @param steps: the number of steps to turn
 */
void step_relative(Stepper* stepper, signed char direction, unsigned char steps);

/**
 * turn the motor's needle to a given position
 * @param stepper: the stepper motor to control
 * @param steps: the absolute position to turn the needle to
 */
void step_absolute(Stepper* stepper, unsigned char steps);

/**
 * the task to be called when a motor should turn;
 * checks the actual and designated position of each stepper
 * stays active until all stepper match the desired position
 *
 * task is scheduled by function step_relative() or step_absolute()
 */
void task_stepTask();

#ifdef STEPPER_SHIFTREGISTER
inline void toggleShiftRegister(Stepper* stepper);
inline void nextByte_ShiftRegister();
#else
inline void togglePins(Stepper* stepper);
#endif /* STEPPER_SHIFTREGISTER */

inline void rotate(Stepper* stepper);


#endif /* STEPPER_H_ */
