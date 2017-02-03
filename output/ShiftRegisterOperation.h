/*
 * ShiftRegisterOperation.h
 *
 *  Created on: 20.09.2016
 *      Author: Richard
 */

#ifndef SHIFTREGISTEROPERATION_H_
#define SHIFTREGISTEROPERATION_H_

#include <msp430.h>
#include <stdint.h>
#include "defines.h"
#include "Task.h"

typedef struct ShiftRegisterOperation_t {
	uint8_t * buffer;
	uint8_t bufferLength;
	uint8_t bytesReceived;
	uint8_t bytesToProcess;
	uint8_t strobePin;	// - todo: rename status; bit field with information of strobe pin and sequence of strobe pin (select pin or strobe on end of transfer)
} ShiftRegisterOperation;

extern ShiftRegisterOperation shiftRegisterOperation_mem[MAXSHIFTREGISTER];
extern int8_t shiftRegisterOperation_size;

void SR_initWriteAddress(volatile unsigned char * address);
void SR_initReadAddress(volatile unsigned char * address);

/**
 * initialize the Shift Register Operation with the read and write address
 * of the SPI interface
 * also uses a task with prio 2
 * @param writeAddress: the address bytes are written into the interface
 * @param readAddress: the address to read from the interface
 */
void SR_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress);

/**
 * creates a new shift register operation structure
 * @param strobePin: a number indicating the pin for the strobe input
 * of the shift register
 * @param buffer: the buffer memory pointer to be used
 * @return: the initialized structure pointer
 */
ShiftRegisterOperation* SR_initShiftRegister(uint8_t strobePin, uint8_t * buffer, uint8_t bufferLength);

/**
 * interrupt service routine call
 * writes the next byte of the active shift register into the address specified
 * by the init function @see SR_initWriteReadAddress
 * @param strobePin: todo
 * @param buffer: the buffer to which data is written / from where data is read to the interface
 * @param bufferLength: the maximum length of the buffer
 * @return -1 in case no byte is written, 1 in case byte was available
 */
int8_t SR_nextByte_ActiveShiftRegister();

/**
 * activates a specific Shift Register
 * @param sr: the shift register to be activated, in other words which bytes
 * are written to the SPI interface
 * @param bytesToProcess: the number of bytes to be written/read. Ensure the buffer is long enough!
 * @return: -1 if another SR is activated and being processed or a positive number (including zero) which
 * identifies the activated SR.
 */
int8_t SR_activateShiftRegister(ShiftRegisterOperation* sr, uint8_t bytesToProcess);

/**
 * checks for SRs which are ready for communication, activates the first found and transmits
 * the first byte
 * @return: -1 if no SR is currently ready, else the number of the activated SR (0..127)
 */
int8_t SR_checkForActiveSROps();




#endif /* SHIFTREGISTEROPERATION_H_ */
