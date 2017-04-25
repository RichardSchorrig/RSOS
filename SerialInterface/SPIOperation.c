/*
 * ShiftRegisterOperation.c
 *
 *  Created on: 20.09.2016
 *      Author: Richard
 *
 *  Changelog:
 *  2017.02.03: in function SR_initOperation: added value to match function addFollowUpTask
 */

#include "SPIOperation.h"

/* exclude everything if not used */
#ifdef MAXSHIFTREGISTER

volatile unsigned char * SR_SPIinterface_readAddress = 0;
volatile unsigned char * SR_SPIinterface_writeAddress = 0;

int8_t g_SPI_activeTransmission = -1;

volatile uint8_t dummyReadByte = 0;

Task* g_SPI_task_strobeSet = 0;
Task* g_SPI_task_strobeReset = 0;
Task* g_SPI_task_activateShiftRegister = 0;

/** task functions! */
void SR_enableTransmission()
{
    USCI_enable_TXIFG(1);
    USCI_enable_RXIFG(0);
    enableUSCI_Interrupt();
}

static inline void strobePinPullHigh();
static void strobePinPullHigh() {
    *spiOperation_mem[g_SPI_activeTransmission].strobePin.port |= spiOperation_mem[g_SPI_activeTransmission].strobePin.pin;
}

static inline void strobePinPullLow();
static void strobePinPullLow() {
    *spiOperation_mem[g_SPI_activeTransmission].strobePin.port &= ~(spiOperation_mem[g_SPI_activeTransmission].strobePin.pin);
}

void task_strobe_set()
{
    if (g_SPI_activeTransmission != -1)
    {
        if (spiOperation_mem[g_SPI_activeTransmission].operationMode & STROBE_POLARITY_HIGH) {
            strobePinPullHigh();
        }
        else {
            strobePinPullLow();
        }

        if (spiOperation_mem[g_SPI_activeTransmission].operationMode & STROBE_ON_TRANSFER_END
            || spiOperation_mem[g_SPI_activeTransmission].operationMode & STROBE_ON_TRANSFER_START)
        {
            scheduleTask(g_SPI_task_strobeReset);
        }
        else
        {
            scheduleTask(g_SPI_task_activateShiftRegister);
        }
    }
}

void task_strobe_reset()
{
    if (g_SPI_activeTransmission != -1)
    {
        if (spiOperation_mem[g_SPI_activeTransmission].operationMode & STROBE_POLARITY_HIGH) {
            strobePinPullLow();
        }
        else {
            strobePinPullHigh();
        }

        if (spiOperation_mem[g_SPI_activeTransmission].operationMode & STROBE_ON_TRANSFER_START)
        {
            scheduleTask(g_SPI_task_activateShiftRegister);
        }
        else
        {
            g_SPI_activeTransmission = -1;
        }
    }
}
/** end of task functions */

void SPI_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress)
{
	SPI_initWriteAddress(writeAddress);
	SPI_initReadAddress(readAddress);

	g_SPI_task_activateShiftRegister = addTask(SHIFTREGISTER_ACTIVATE_PRIORITY, SR_enableTransmission);
	g_SPI_task_strobeSet = addTask(SHIFTREGISTER_STROBESET_PRIORITY, task_strobe_set);
	g_SPI_task_strobeReset = addTask(SHIFTREGISTER_STROBERESET_PRIORITY, task_strobe_reset);
}

SPIOperation* SPI_initSPIOperation(uint8_t strobePin, volatile uint8_t * strobePort, BufferBuffer_uint8* bufferbuffer, uint8_t strobeOperation)
{
	spiOperation_mem[spiOperation_size].bufferbuffer = bufferbuffer;
	spiOperation_mem[spiOperation_size].operationMode = strobeOperation;
	spiOperation_mem[spiOperation_size].bytesReceived = 0;
	spiOperation_mem[spiOperation_size].bytesToProcess = 0;
	spiOperation_mem[spiOperation_size].strobePin.pin = strobePin;
	spiOperation_mem[spiOperation_size].strobePin.port = strobePort;

	spiOperation_size += 1;
	return &spiOperation_mem[spiOperation_size - 1];
}

#endif /* MAXSHIFTREGISTER */
