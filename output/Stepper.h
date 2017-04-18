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
 *
 * changelog:
 *  2017 04 17
 *      rewriting of all functions, splitting up into 3 different files
 *      (by connection of the stepper to the mcu)
 *
 *  2017 04 18
 *      done rewriting, check if functions step_relative and step_absolute can be defined here
 */

#ifndef STEPPER_H_
#define STEPPER_H_

/**
 * defines that all steppers are connected via a shift register instead of a direct connection
 */


/**
 * maximum steps of a motor
 */
#define STEPPER_MAXSTEPS 160

#include <RSOSDefines.h>

/* exclude everything if not used */
#ifdef MAXSTEPPER
#include <stdint.h>

#include "../Task.h"

#ifdef STEPPER_DIRECT
#include "Stepper/Stepper_Direct.h"
#elif defined STEPPER_SHIFTREGISTER
#include "Stepper/Stepper_ShiftRegister.h"
#elif defined STEPPERLV8549
#include "Stepper/Stepper_LV8549.h"
#endif /* STEPPER */

#endif /* MAXSTEPPER */
#endif /* STEPPER_H_ */
