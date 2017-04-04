/*
 * ShiftRegisterOperation.c
 *
 *  Created on: 20.09.2016
 *      Author: Richard
 *
 *  Changelog:
 *  2017.02.03: in function SR_initOperation: added value to match function addFollowUpTask
 */

#include <HardwareAdaptionLayer.h>
#include "SPIOperation.h"

/* exclude everything if not used */
#ifdef MAXSHIFTREGISTER

volatile unsigned char * SR_SPIinterface_readAddress = 0;
volatile unsigned char * SR_SPIinterface_writeAddress = 0;

int8_t g_SPI_activeTransmission = -1;
int8_t g_SPI_lastActiveTransmission = -1;

static uint8_t g_SPI_strobeMode = 0;

Task* g_SPI_task_strobe = 0;
Task* g_SPI_task_activateShiftRegister = 0;

/** task functions! */
void SR_enableTransmission()
{
    enableUSCI_Interrupt();
}

static inline void strobePinPullHigh(uint8_t index);
static void strobePinPullHigh(uint8_t index) {
    *spiOperation_mem[index].strobePin.port |= spiOperation_mem[index].strobePin.pin;
}

static inline void strobePinPullLow(uint8_t index);
static void strobePinPullLow(uint8_t index) {
    *spiOperation_mem[index].strobePin.port &= ~(spiOperation_mem[index].strobePin.pin);
}

void task_strobe_set()
{
    if (g_SPI_strobeMode & STROBE_ON_TRANSFER_END) {
        if (g_SPI_lastActiveTransmission != -1) {
            if (g_SPI_strobeMode & STROBE_POLARITY_HIGH) {
                strobePinPullHigh(g_SPI_lastActiveTransmission);
            }
            else {
                strobePinPullLow(g_SPI_lastActiveTransmission);
            }
        }
    }
    else if (g_SPI_strobeMode & STROBE_ON_TRANSFER_START) {
        if (g_SPI_activeTransmission != -1) {
            if (g_SPI_strobeMode & STROBE_POLARITY_HIGH) {
                strobePinPullHigh(g_SPI_activeTransmission);
            }
            else {
                strobePinPullLow(g_SPI_activeTransmission);
            }
        }
    }
}

void task_strobe_reset()
{
    if (g_SPI_lastActiveTransmission != -1) {
        if (g_SPI_strobeMode & STROBE_POLARITY_HIGH) {
            strobePinPullLow(g_SPI_lastActiveTransmission);
        }
        else {
            strobePinPullHigh(g_SPI_lastActiveTransmission);
        }
        g_SPI_lastActiveTransmission = -1;
    }
}
/** end of task functions */

static Task* followUpArray[1] = {0};

void SPI_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress, uint8_t strobeOperation)
{
	SPI_initWriteAddress(writeAddress);
	SPI_initReadAddress(readAddress);

	if (strobeOperation & STROBE_ON_TRANSFER_END) {
        g_SPI_task_strobe = addTask(SHIFTREGISTER_STROBESET_PRIORITY, task_strobe_set);
        Task* task_reset = addTask(SHIFTREGISTER_STROBERESET_PRIORITY, task_strobe_reset);
        addFollowUpTask(g_SPI_task_strobe, followUpArray, task_reset);
        g_SPI_task_activateShiftRegister = addTask(SHIFTREGISTER_ACTIVATE_PRIORITY, SR_enableTransmission);
	}
	else if (strobeOperation & STROBE_ON_TRANSFER_START) {
	    Task* task_transmit = addTask(SHIFTREGISTER_ACTIVATE_PRIORITY, SR_enableTransmission);
	    g_SPI_task_activateShiftRegister = addTask(SHIFTREGISTER_STROBESET_PRIORITY, task_strobe_set);
        g_SPI_task_strobe = addTask(SHIFTREGISTER_STROBERESET_PRIORITY, task_strobe_reset);

        addFollowUpTask(g_SPI_task_activateShiftRegister, followUpArray, task_transmit);
	}

	g_SPI_strobeMode = strobeOperation;
}

SPIOperation* SPI_initSPIOperation(uint8_t strobePin, volatile uint8_t * strobePort, BufferBuffer_uint8* bufferbuffer, uint8_t bufferLength)
{
	spiOperation_mem[spiOperation_size].bufferbuffer = bufferbuffer;
	spiOperation_mem[spiOperation_size].bufferLength = bufferLength;
	spiOperation_mem[spiOperation_size].bytesReceived = 0;
	spiOperation_mem[spiOperation_size].bytesToProcess = 0;
	spiOperation_mem[spiOperation_size].strobePin.pin = strobePin;
	spiOperation_mem[spiOperation_size].strobePin.port = strobePort;

	spiOperation_size += 1;
	return &spiOperation_mem[spiOperation_size - 1];
}

#endif /* MAXSHIFTREGISTER */
