/*
 * ShiftRegisterOperation.c
 *
 *  Created on: 20.09.2016
 *      Author: Richard
 */

#include "ShiftRegisterOperation.h"

volatile unsigned char * SR_SPIinterface_readAddress;
volatile unsigned char * SR_SPIinterface_writeAddress;

extern Task* task_strobe;

int8_t activeShiftRegister = -1;
int8_t lastActiveShiftRegister = -1;

Task* task_activateShiftRegister;

/** task functions! */
void SR_task_activateShiftRegister()
{
	UCA0TXBUF = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
	shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess -= 1;
}

void task_strobe_set()
{
	switch (shiftRegisterOperation_mem[lastActiveShiftRegister].strobePin) {
	case 0: P6OUT |= STROBE_STEPPER; break;
	case 1: P6OUT |= STROBE_DISPLAY; break;
	case 2: break;
	default: break;
	}
}

void task_strobe_reset()
{
	switch (shiftRegisterOperation_mem[lastActiveShiftRegister].strobePin) {
	case 0: P6OUT &= ~STROBE_STEPPER; break;
	case 1: P6OUT &= ~STROBE_DISPLAY; break;
	case 2: break;
	default: break;
	}
	lastActiveShiftRegister = -1;
}
/** end of task functions */

void SR_initWriteAddress(volatile unsigned char * address)
{
	SR_SPIinterface_writeAddress = address;
}

void SR_initReadAddress(volatile unsigned char * address)
{
	SR_SPIinterface_readAddress = address;
}

void SR_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress)
{
	SR_initWriteAddress(writeAddress);
	SR_initReadAddress(readAddress);
	task_strobe = addTask(12, task_strobe_set);
	Task* task_reset = addTask(13, task_strobe_reset);
	addFollowUpTask(task_strobe, task_reset);
//	addFollowUpTask(task_activateShiftRegister, task_strobe);

	task_activateShiftRegister = addTask(4, SR_task_activateShiftRegister);
}

ShiftRegisterOperation* SR_initShiftRegister(uint8_t strobePin, uint8_t * buffer, uint8_t bufferLength)
{
	shiftRegisterOperation_mem[shiftRegisterOperation_size].buffer = buffer;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].bufferLength = bufferLength;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].bytesReceived = 0;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].bytesToProcess = 0;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].strobePin = strobePin;

	shiftRegisterOperation_size += 1;
	return &shiftRegisterOperation_mem[shiftRegisterOperation_size - 1];
}

int8_t SR_checkForActiveSROps()
{
	activeShiftRegister = -1;
	int8_t i;
	for (i = shiftRegisterOperation_size-1; i>=0; i-=1)
	{
		if (shiftRegisterOperation_mem[i].bytesToProcess != 0)
		{
			activeShiftRegister = i;
			UCA0TXBUF = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
			shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess -= 1;
			break;
		}
	}
	return activeShiftRegister;
}

int8_t SR_nextByte_ActiveShiftRegister()
{
	int8_t retVal = -1;
	if (activeShiftRegister != -1)
	{
		//shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived] = *SR_SPIinterface_readAddress;
		if (shiftRegisterOperation_mem[activeShiftRegister].bytesReceived < shiftRegisterOperation_mem[activeShiftRegister].bufferLength)
		{
			shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived] = UCA0RXBUF;
			shiftRegisterOperation_mem[activeShiftRegister].bytesReceived += 1;
		}
		if (shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess != 0)
		{
			//*SR_SPIinterface_writeAddress = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
			UCA0TXBUF = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
			shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess -= 1;
			retVal = 1;
		}
		else
		{
			scheduleTask(task_strobe);
			lastActiveShiftRegister = activeShiftRegister;
			activeShiftRegister = -1;
//			retVal = SR_checkForActiveSROps();
		}
	}
	return retVal;
}

int8_t SR_activateShiftRegister(ShiftRegisterOperation* sr, uint8_t bytesToProcess)
{
	sr->bytesToProcess = bytesToProcess;
	sr->bytesReceived = 0;

	int8_t retVal = -1;
	if (activeShiftRegister == -1)
	{
		activeShiftRegister = sr - shiftRegisterOperation_mem;
//		UCA0TXBUF = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
//		shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess -= 1;
		scheduleTask(task_activateShiftRegister);
		retVal = activeShiftRegister;
	}
	return retVal;
}


