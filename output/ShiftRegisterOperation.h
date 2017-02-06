/*
 * ShiftRegisterOperation.h
 *
 *  Created on: 20.09.2016
 *      Author: Richard
 *
 *  Changelog:
 *  2017.020.05: changed SR_activateShiftRegister, added support for different Transfer and Receive Buffer Registers (to be tested)
 */

#ifndef SHIFTREGISTEROPERATION_H_
#define SHIFTREGISTEROPERATION_H_

#include "../Path.h"
#include <msp430.h>
#include <stdint.h>
#include "../Task.h"

#include PATH_RSOSDEFINES_H

typedef struct ShiftRegisterStrobe_t {
    uint8_t pin;
    volatile uint8_t * port;
} ShiftRegisterStrobe;

/**
 * Shift Register Operation structure
 *  Fields:
 *      buffer: a pointer to a buffer array
 *      bufferLength: the length of the buffer
 *      bytesReceived: the number of bytes received while this SR is active, is reset to 0 when activated
 *      bytesToProcess: the number of bytes left to be written, is set when activated
 *      strobePin: todo!
 *
 *  MEMORY:
 *      this structure takes up 8 bytes
 */
typedef struct ShiftRegisterOperation_t {
	uint8_t * buffer;
	uint8_t bufferLength;
	uint8_t bytesReceived;
	uint8_t bytesToProcess;
//	uint8_t strobePin;	// - todo: rename status; bit field with information of strobe pin and sequence of strobe pin (select pin or strobe on end of transfer)
	ShiftRegisterStrobe strobePin;
} ShiftRegisterOperation;

extern ShiftRegisterOperation shiftRegisterOperation_mem[MAXSHIFTREGISTER];
extern int8_t shiftRegisterOperation_size;

/**
 * set the address to write to, i.e. the interface transfer buffer register the shift register is connected to.
 * MSP Devices: something like UCA0TXBUF
 */
void SR_initWriteAddress(volatile unsigned char * address);

/**
 * set the address to read from, i.e. the interface receive buffer register the shift register is connected to.
 * MSP Devices: something like UCA0RXBUF
 */
void SR_initReadAddress(volatile unsigned char * address);

/**
 * initialize the Shift Register Operation with the read and write address
 * of the SPI interface
 * Needs a total of 2 Tasks to operate, consider 2 extra task in RSOSDefines.h
 * @param writeAddress: the address bytes are written into the interface
 * @param readAddress: the address to read from the interface
 */
void SR_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress);

/**
 * creates a new shift register operation structure
 * @param strobePin: a number indicating the pin for the strobe input
 * of the shift register
 * @param buffer: the buffer memory pointer to be used
 * @param bufferLength: the maximum length of the buffer
 * @return: the initialized structure pointer
 */
ShiftRegisterOperation* SR_initShiftRegister(uint8_t strobePin, uint8_t * buffer, uint8_t bufferLength);

/**
 * interrupt service routine call
 * writes the next byte of the active shift register into the address specified
 * by the init function @see SR_initWriteReadAddress
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
