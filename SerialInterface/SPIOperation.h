/*
 * ShiftRegisterOperation.h
 *
 *  Created on: 20.09.2016
 *      Author: Richard
 *
 *  Changelog:
 *  2017.020.05: changed SR_activateShiftRegister, added support for different Transfer and Receive Buffer Registers
 *
 *  2017 04 02
 *      renamed files, renamed structures, functions etc to SPIOperation
 *
 *  2017 04 04
 *      changed the strobe operation to be dynamic for each SPIOperation, added more strobe types
 *
 *  2017 04 05
 *      todo: add priority to SPIOperations, dynamic spi scheduler that checks for active spiops by itself
 */

#ifndef SHIFTREGISTEROPERATION_H_
#define SHIFTREGISTEROPERATION_H_

#include <RSOSDefines.h>

/* exclude everything if not used */
#ifdef MAXSHIFTREGISTER

#include <msp430.h>
#include <stdint.h>
#include "../Task.h"
#include "../buffer/BasicBuffer.h"
#include "../buffer/Buffer_int8.h"
#include "../buffer/BufferBuffer_int8.h"

extern volatile unsigned char * SR_SPIinterface_readAddress;
extern volatile unsigned char * SR_SPIinterface_writeAddress;

extern int8_t g_SPI_activeTransmission;
extern int8_t g_SPI_lastActiveTransmission;

extern Task* g_SPI_task_activateShiftRegister;
extern Task* g_SPI_task_strobeSet;
extern Task* g_SPI_task_strobeReset;

typedef struct SPIStrobe_t {
    uint8_t pin;
    volatile uint8_t * port;
} SPIStrobe;

/**
 * Shift Register Operation structure
 *  Fields:
 *      buffer: a pointer to a buffer array
 *      strobeOperation: indicates the type of strobe signal:
 *              - short strobe on start
 *              - short strobe on end
 *              - strobe while transmission is active
 *              - no strobe signal
 *          the strobe polarity can be selected: active high or active low
 *      bytesReceived: the number of bytes received while this SR is active, is reset to 0 when activated
 *      bytesToProcess: the number of bytes left to be written, is set when activated
 *      strobePin: structure with the pin and port to set
 *
 *  MEMORY:
 *      this structure takes up 8 bytes
 */
typedef struct SPIOperation_t {
    BufferBuffer_uint8* bufferbuffer;
	uint8_t strobeOperation;
	uint8_t bytesReceived;
	uint8_t bytesToProcess;
	SPIStrobe strobePin;
} SPIOperation;

extern SPIOperation spiOperation_mem[MAXSHIFTREGISTER];
extern int8_t spiOperation_size;
/*
extern int8_t activeShiftRegister;
extern int8_t lastActiveShiftRegister;
extern volatile unsigned char * SR_SPIinterface_readAddress;
extern volatile unsigned char * SR_SPIinterface_writeAddress;
extern Task* task_strobe;
*/

/**
 * strobe is activated on end of transfer
 * a short square signal is initiated
 */
#define STROBE_ON_TRANSFER_END 0x10

/**
 * strobe is activated on start of transfer
 * a short square signal is initiated that will be deactivated
 * when transmission is active
 */
#define STROBE_ON_TRANSFER_START 0x20

/**
 * strobe is activated on start of transfer and held till end
 * a long square signal is generated during the whole transmission
 */
#define STROBE_ON_TRANSFER 0x40

/**
 * no strobe signal is put out
 */
#define STROBE_NO_STROBE 0x00

/**
 * strobe polarity active high
 * strobe active is logic high, strobe inactive is pulled low
 */
#define STROBE_POLARITY_HIGH 0x01

/**
 * strobe polarity active low
 * strobe active is logic low, strobe inactive is pulled high
 */
#define STROBE_POLARITY_LOW 0x00

/**
 * set the address to write to, i.e. the interface transfer buffer register the shift register is connected to.
 * MSP Devices: something like UCA0TXBUF
 */
static inline void SPI_initWriteAddress(volatile unsigned char * address);
static inline void SPI_initWriteAddress(volatile unsigned char * address)
{
    SR_SPIinterface_writeAddress = address;
}

/**
 * set the address to read from, i.e. the interface receive buffer register the shift register is connected to.
 * MSP Devices: something like UCA0RXBUF
 */
static inline void SPI_initReadAddress(volatile unsigned char * address);
static inline void SPI_initReadAddress(volatile unsigned char * address)
{
    SR_SPIinterface_readAddress = address;
}

/**
 * initialize the SPI Operation with the read and write address
 * of the SPI interface
 * Needs a total of 3 Tasks to operate, consider 3 extra task in RSOSDefines.h
 * @param writeAddress: the address bytes are written into the interface
 * @param readAddress: the address to read from the interface
 */
void SPI_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress);

/**
 * creates a new SPI operation structure
 * @param strobePin: a number indicating the pin for the strobe input
 * of the shift register
 * @param bufferbuffer: the buffer memory pointer to be used
 * @param bufferLength: the maximum length of the buffer
 * @param strobeOperation: the operation mode for the strobe pin.
 * The byte contains the mode and polarity of the strobe signal:
 *  - STROBE_ON_TRANSFER
 *  - STROBE_ON_TRANSFER_START
 *  - STROBE_ON_TRANSFER_END
 *  - STROBE_NO_STROBE
 *  logically or'd with:
 *  - STROBE_POLARITY_HIGH
 *  - STROBE_POLARITY_LOW
 * @return: the initialized structure pointer
 */
SPIOperation* SPI_initSPIOperation(uint8_t strobePin, volatile uint8_t * strobePort, BufferBuffer_uint8* bufferbuffer, uint8_t strobeOperation);

/**
 * changes the SPI operation strobe mode
 * @param spiop: the spi operation structure to change
 * @param strobeOperation: the operation mode for the strobe pin
 * @return -1 if the structure is being processed, 0 on success
 */
static inline int8_t SPI_changeStrobeOperaton(SPIOperation* spiop, uint8_t strobeOperation) __attribute__((always_inline));
static inline int8_t SPI_changeStrobeOperaton(SPIOperation* spiop, uint8_t strobeOperation)
{
    if (0 == spiop->bytesToProcess)
    {
        spiop->strobeOperation = strobeOperation;
        return 0;
    }
    return -1;
}

/**
 * interrupt service routine call
 * writes the next byte of the active shift register into the address specified
 * by the init function @see SR_initWriteReadAddress
 * @return -1 in case no byte is written, 1 in case byte was available
 */
static inline int8_t SPI_nextByte_ActiveShiftRegister();
static inline int8_t SPI_nextByte_ActiveShiftRegister()
{
    int8_t retVal = -1;
    if (g_SPI_activeTransmission != -1)
    {
        if (spiOperation_mem[g_SPI_activeTransmission].bytesToProcess > 0)
        {
            spiOperation_mem[g_SPI_activeTransmission].bytesToProcess -= 1;
            retVal = 1;
            set_getNext_bufferbuffer_uint8(spiOperation_mem[g_SPI_activeTransmission].bufferbuffer,
                                           SR_SPIinterface_readAddress,
                                           SR_SPIinterface_writeAddress);
        }
        else
        {
            if (spiOperation_mem[g_SPI_activeTransmission].strobeOperation & STROBE_ON_TRANSFER_END)
            {
                scheduleTask(g_SPI_task_strobeSet);
            }
            else if (spiOperation_mem[g_SPI_activeTransmission].strobeOperation & STROBE_ON_TRANSFER)
            {
                scheduleTask(g_SPI_task_strobeReset);
            }
            else
            {
                g_SPI_activeTransmission = -1;
            }
        }
    }
    return retVal;
}

/**
 * activates a specific Shift Register
 * @param sr: the shift register to be activated, in other words which bytes
 * are written to the SPI interface
 * @param bytesToProcess: the number of bytes to be written/read. Ensure the buffer is long enough!
 * @return: -1 if another SR is activated and being processed or a positive number (including zero) which
 * identifies the activated SR.
 * you want to check the return value. if -1 is returned, try again in the next cycle
 */
static inline int8_t SPI_activateSPIOperation(SPIOperation* sr, uint8_t bytesToProcess);
static inline int8_t SPI_activateSPIOperation(SPIOperation* sr, uint8_t bytesToProcess)
{
    int8_t retVal = -1;
    if (g_SPI_activeTransmission == -1)
    {
        g_SPI_activeTransmission = sr - spiOperation_mem;
        sr->bytesToProcess = bytesToProcess;
        sr->bytesReceived = 0;
        if (sr->strobeOperation & STROBE_ON_TRANSFER_START || sr->strobeOperation & STROBE_ON_TRANSFER)
        {
            scheduleTask(g_SPI_task_strobeSet);
        }
        else
        {
            scheduleTask(g_SPI_task_activateShiftRegister);
        }
        retVal = g_SPI_activeTransmission;
    }
    return retVal;
}

#endif /* MAXSHIFTREGISTER */
#endif /* SHIFTREGISTEROPERATION_H_ */
