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
 *  2017 04 18
 *      add noRead to strobeOperation, bytes in the buffer are not overwritten by received bytes
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
 *      bufferbuffer: a pointer to a buffer array
 *      operationMode: indicates the type of strobe signal and read/write operation:
 *          xTSE PRWx
 *              T: strobe while transmission is active (Chip Enable)
 *              S: short strobe on start
 *              E: short strobe on end
 *              P: strobe polarity (1: active high, 0: active low)
 *              - no strobe signal when TSE are low
 *              R: read from interface (1: read and write to buffer, 0: no read)
 *              W: write to interface (1: read from buffer and write to interface, 0: 0x00 is transferred for each byte to process)
 *
 *      bytesReceived: the number of bytes received while this SR is active, is reset to 0 when activated todo: not counted up
 *      bytesToProcess: the number of bytes left to be written, is set when activated
 *      strobePin: structure with the pin and port to set
 *
 *  MEMORY:
 *      this structure takes up 8 bytes
 */
typedef struct SPIOperation_t {
    BufferBuffer_uint8* bufferbuffer;
	uint8_t operationMode;
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
#define STROBE_POLARITY_HIGH 0x08

/**
 * strobe polarity active low
 * strobe active is logic low, strobe inactive is pulled high
 */
#define STROBE_POLARITY_LOW 0x00

/**
 * do Read operation: when being transceived, the bytes read by the
 * interface do overwrite the buffer
 */
#define SPI_DOREAD 0x04

/**
 * do write operation: when being transceived, the buffer content is written
 * to the interface
 */
#define SPI_DOWRITE 0x02

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
        spiop->operationMode = strobeOperation;
        return 0;
    }
    return -1;
}

/**
 * enables / disables the Read-Operation
 * @param spiop the spi operation structure to change
 * @param enable 0: the received bytes are not written to the buffer
 *               1: the received bytes are written to the buffer
 * @return -1 if the structure is being processed, 0 on success
 */
static inline int8_t SPI_changeReadOperation(SPIOperation* spiop, uint8_t enable) __attribute__((always_inline));
static inline int8_t SPI_changeReadOperation(SPIOperation* spiop, uint8_t enable)
{
    if (enable)
    {
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode | SPI_DOREAD);
    }
    else
    {
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode & ~SPI_DOREAD);
    }
}

/**
 * enables / disables the Write-Operation
 * @param spiop the spi operation structure to change
 * @param enable 0: for each byte 0x00 is transferred
 *               1: the content of the buffer is transferred
 * @return -1 if the structure is being processed, 0 on success
 */
static inline int8_t SPI_changeWriteOperation(SPIOperation* spiop, uint8_t enable) __attribute__((always_inline));
static inline int8_t SPI_changeWriteOperation(SPIOperation* spiop, uint8_t enable)
{
    if (enable)
    {
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode | SPI_DOWRITE);
    }
    else
    {
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode & ~SPI_DOWRITE);
    }
}

static inline void SPI_nextByte_Write() __attribute__((always_inline));
static inline void SPI_nextByte_Write()
{
    if (spiOperation_mem[g_SPI_activeTransmission].operationMode & SPI_DOWRITE)
    {
        BufferBuffer_uint8_get(spiOperation_mem[g_SPI_activeTransmission].bufferbuffer,
                               SR_SPIinterface_writeAddress);
    }
    else
    {
        *SR_SPIinterface_writeAddress = 0x00;
    }
}

static inline void SPI_nextByte_Read() __attribute__((always_inline));
static inline void SPI_nextByte_Read()
{
    if (spiOperation_mem[g_SPI_activeTransmission].operationMode & SPI_DOREAD)
    {
        BufferBuffer_uint8_set(spiOperation_mem[g_SPI_activeTransmission].bufferbuffer,
                               SR_SPIinterface_readAddress);
    }
}

/**
 * interrupt service routine call
 * writes the next byte of the active shift register into the address specified
 * by the init function @see SR_initWriteReadAddress
 * @return -1 in case no byte is written, 1 in case byte was available
 */
static inline int8_t SPI_nextByte_ActiveShiftRegister() __attribute__((always_inline));
static inline int8_t SPI_nextByte_ActiveShiftRegister()
{
    int8_t retVal = -1;
    if (g_SPI_activeTransmission != -1)
    {
        if (spiOperation_mem[g_SPI_activeTransmission].bytesToProcess > 0)
        {
            if (spiOperation_mem[g_SPI_activeTransmission].bytesReceived == 0)
            {
                SPI_nextByte_Write();
            }
            else
            {
                SPI_nextByte_Read();

                BufferBuffer_uint8_increment(spiOperation_mem[g_SPI_activeTransmission].bufferbuffer);

                SPI_nextByte_Write();
            }
            spiOperation_mem[g_SPI_activeTransmission].bytesToProcess -= 1;
            spiOperation_mem[g_SPI_activeTransmission].bytesReceived += 1;
            retVal = 1;
        }
        else
        {
            if (spiOperation_mem[g_SPI_activeTransmission].operationMode & STROBE_ON_TRANSFER_END)
            {
                scheduleTask(g_SPI_task_strobeSet);
            }
            else if (spiOperation_mem[g_SPI_activeTransmission].operationMode & STROBE_ON_TRANSFER)
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
static inline int8_t SPI_activateSPIOperation(SPIOperation* sr, uint8_t bytesToProcess) __attribute__((always_inline));
static inline int8_t SPI_activateSPIOperation(SPIOperation* sr, uint8_t bytesToProcess)
{
    int8_t retVal = -1;
    if (g_SPI_activeTransmission == -1)
    {
        g_SPI_activeTransmission = sr - spiOperation_mem;
        sr->bytesToProcess = bytesToProcess;
        sr->bytesReceived = 0;
        if (sr->operationMode & STROBE_ON_TRANSFER_START || sr->operationMode & STROBE_ON_TRANSFER)
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
