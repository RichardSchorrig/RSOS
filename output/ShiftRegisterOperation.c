/*
 * ShiftRegisterOperation.c
 *
 *  Created on: 20.09.2016
 *      Author: Richard
 *
 *  Changelog:
 *  2017.02.03: in function SR_initOperation: added value to match function addFollowUpTask
 */

#include "ShiftRegisterOperation.h"

/* exclude everything if not used */
#ifdef MAXSHIFTREGISTER

volatile unsigned char * SR_SPIinterface_readAddress = 0;
volatile unsigned char * SR_SPIinterface_writeAddress = 0;

static Task* task_strobe = 0;

static int8_t activeShiftRegister = -1;
static int8_t lastActiveShiftRegister = -1;

static uint8_t strobeMode = 0;

Task* task_activateShiftRegister = 0;

extern void enableUSCI_Interrupt();

/** task functions! */
void SR_task_activateShiftRegister()
{
    enableUSCI_Interrupt();
//    UCA0IFG |= UCTXIFG;

//    USCIA0TXRX_ISR();

//  shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess -= 1;
//    getNext_bufferbuffer_uint8(shiftRegisterOperation_mem[activeShiftRegister].bufferbuffer, SR_SPIinterface_writeAddress);

    //*SR_SPIinterface_writeAddress = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];

}

inline void strobePinPullHigh(uint8_t index);
void strobePinPullHigh(uint8_t index) {
    *shiftRegisterOperation_mem[index].strobePin.port |= shiftRegisterOperation_mem[index].strobePin.pin;
}

inline void strobePinPullLow(uint8_t index);
void strobePinPullLow(uint8_t index) {
    *shiftRegisterOperation_mem[index].strobePin.port &= ~(shiftRegisterOperation_mem[index].strobePin.pin);
}

void task_strobe_set()
{
    /*
     * todo smth like this:
     * typedef struct SR_StrobePins_t {
     *  volatile unsigned char * port;
     *  unsigned char pin;
     * }
     * sr_strobe_pins[ActiveSR_strobePin].port |= sr_strobe_pins[ActiveSR_strobePin].pin;

	switch (shiftRegisterOperation_mem[lastActiveShiftRegister].strobePin) {
	case 0: P6OUT |= STROBE_STEPPER; break;
	case 1: P6OUT |= STROBE_DISPLAY; break;
	case 2: break;
	default: break;
	}
	*/
    if (strobeMode & STROBE_ON_TRANSFER_END) {
        if (lastActiveShiftRegister != -1) {
            if (strobeMode & STROBE_POLARITY_HIGH) {
                strobePinPullHigh(lastActiveShiftRegister);
            }
            else {
                strobePinPullLow(lastActiveShiftRegister);
            }
        }
    }
    else if (strobeMode & STROBE_ON_TRANSFER_START) {
        if (activeShiftRegister != -1) {
            if (strobeMode & STROBE_POLARITY_HIGH) {
                strobePinPullHigh(activeShiftRegister);
            }
            else {
                strobePinPullLow(activeShiftRegister);
            }
        }
    }
}

void task_strobe_reset()
{
    /*
	switch (shiftRegisterOperation_mem[lastActiveShiftRegister].strobePin) {
	case 0: P6OUT &= ~STROBE_STEPPER; break;
	case 1: P6OUT &= ~STROBE_DISPLAY; break;
	case 2: break;
	default: break;
	}
	*/
    if (lastActiveShiftRegister != -1) {
        if (strobeMode & STROBE_POLARITY_HIGH) {
            strobePinPullLow(lastActiveShiftRegister);
        }
        else {
            strobePinPullHigh(lastActiveShiftRegister);
        }
        lastActiveShiftRegister = -1;
    }
}
/** end of task functions */

inline void SR_initWriteAddress(volatile unsigned char * address)
{
	SR_SPIinterface_writeAddress = address;
}

inline void SR_initReadAddress(volatile unsigned char * address)
{
	SR_SPIinterface_readAddress = address;
}

static Task* followUpArray[1] = {0};

void SR_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress, uint8_t strobeOperation)
{
	SR_initWriteAddress(writeAddress);
	SR_initReadAddress(readAddress);

	if (strobeOperation & STROBE_ON_TRANSFER_END) {
        task_strobe = addTask(SHIFTREGISTER_STROBESET_PRIORITY, task_strobe_set);
        Task* task_reset = addTask(SHIFTREGISTER_STROBERESET_PRIORITY, task_strobe_reset);
        addFollowUpTask(task_strobe, followUpArray, task_reset);
        task_activateShiftRegister = addTask(SHIFTREGISTER_ACTIVATE_PRIORITY, SR_task_activateShiftRegister);
	}
	else if (strobeOperation & STROBE_ON_TRANSFER_START) {
	    Task* task_transmit = addTask(SHIFTREGISTER_ACTIVATE_PRIORITY, SR_task_activateShiftRegister);
	    task_activateShiftRegister = addTask(SHIFTREGISTER_STROBESET_PRIORITY, task_strobe_set);
        task_strobe = addTask(SHIFTREGISTER_STROBERESET_PRIORITY, task_strobe_reset);

        addFollowUpTask(task_activateShiftRegister, followUpArray, task_transmit);
	}

	strobeMode = strobeOperation;
}

ShiftRegisterOperation* SR_initShiftRegister(uint8_t strobePin, volatile uint8_t * strobePort, BufferBuffer_uint8* bufferbuffer, uint8_t bufferLength)
{
	shiftRegisterOperation_mem[shiftRegisterOperation_size].bufferbuffer = bufferbuffer;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].bufferLength = bufferLength;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].bytesReceived = 0;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].bytesToProcess = 0;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].strobePin.pin = strobePin;
	shiftRegisterOperation_mem[shiftRegisterOperation_size].strobePin.port = strobePort;

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
			getNext_bufferbuffer_uint8(shiftRegisterOperation_mem[activeShiftRegister].bufferbuffer, SR_SPIinterface_writeAddress);
			//*SR_SPIinterface_writeAddress = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
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
/*
		if (shiftRegisterOperation_mem[activeShiftRegister].bytesReceived < shiftRegisterOperation_mem[activeShiftRegister].bufferLength)
		{
//			shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived] = UCA0RXBUF;
		    set_bufferbuffer_uint8(shiftRegisterOperation_mem[activeShiftRegister].bufferbuffer, SR_SPIinterface_readAddress);
		    //shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived] = *SR_SPIinterface_readAddress;
			shiftRegisterOperation_mem[activeShiftRegister].bytesReceived += 1;
		}
*/
		if (shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess > 0)
		{
			//*SR_SPIinterface_writeAddress = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
//			UCA0TXBUF = shiftRegisterOperation_mem[activeShiftRegister].buffer[shiftRegisterOperation_mem[activeShiftRegister].bytesReceived];
			shiftRegisterOperation_mem[activeShiftRegister].bytesToProcess -= 1;
			retVal = 1;
			set_getNext_bufferbuffer_uint8(shiftRegisterOperation_mem[activeShiftRegister].bufferbuffer, 0, SR_SPIinterface_writeAddress);
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
	int8_t retVal = -1;
	if (activeShiftRegister == -1)
	{
	    activeShiftRegister = sr - shiftRegisterOperation_mem;
	    sr->bytesToProcess = bytesToProcess;
	    sr->bytesReceived = 0;
	    /*
	    activeShiftRegister = 0;
		ShiftRegisterOperation* p = shiftRegisterOperation_mem;
		while (p != sr) {
		    activeShiftRegister += 1;
		    p += 1;
		    if (activeShiftRegister >= MAXSHIFTREGISTER) {
		        activeShiftRegister = -1;
		        break;
		    }
		}
		*/
		scheduleTask(task_activateShiftRegister);
		retVal = activeShiftRegister;
	}
	return retVal;
}
/*
void SR_changeBuffer(ShiftRegisterOperation* sr, uint8_t* buffer, uint8_t bufferLength) {
    sr->buffer = buffer;
    sr->bufferLength = bufferLength;
}
*/

#endif /* MAXSHIFTREGISTER */
