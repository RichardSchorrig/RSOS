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
 *  2017 04 26
 *      no mixed read write possible, split up interrupts, changed some bits
 */

#ifndef SHIFTREGISTEROPERATION_H_
#define SHIFTREGISTEROPERATION_H_

#include <RSOSDefines.h>
#include <HardwareAdaptionLayer.h>

/* exclude everything if not used */
#ifdef MAXSHIFTREGISTER

#include <stdint.h>
#include "../Task.h"
#include "../buffer/BasicBuffer_int8.h"

extern volatile unsigned char * SR_SPIinterface_readAddress;
extern volatile unsigned char * SR_SPIinterface_writeAddress;

extern int8_t g_SPI_activeTransmission;

extern volatile uint8_t dummyReadByte;

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
 *      buffer: the position in the buffer_void_mem to be used
 *      operationMode: indicates the type of strobe signal and read/write operation:
 *          PTSE xxRW
 *              P: strobe polarity (1: active high, 0: active low)
 *              T: strobe while transmission is active (Chip Enable)
 *              S: short strobe on start
 *              E: short strobe on end
 *
 *              - no strobe signal when T, S, E are low
 *              R: read from interface (1: Read enabled, bytes overwrite the contents of the buffer; 0: Write only, no bytes are put to the buffer)
 *              W: write to the interface (1: Write enabled, bytes from the buffer are written to the interface; 0: for every cycle, 0x00 is transferred)
 *
 *      bytesReceived: the number of bytes received while this SR is active, is reset to 0 when activated
 *      bytesToRead: the number of bytes to receive (only valid in read mode)
 *      bytesToWrite: the number of bytes to write (valid in read and write mode)
 *      strobePin: structure with the pin and port to set
 *
 *  MEMORY:
 *      this structure takes up 6 bytes + 1 pointer
 */
typedef struct SPIOperation_t {
    int8_t buffer;
	uint8_t operationMode;
	uint8_t bytesReceived;
	uint8_t bytesToRead;
	uint8_t bytesToWrite;
	SPIStrobe strobePin;
} SPIOperation;

extern SPIOperation spiOperation_mem[MAXSHIFTREGISTER];
extern int8_t spiOperation_size;

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
#define STROBE_POLARITY_HIGH 0x80

/**
 * strobe polarity active low
 * strobe active is logic low, strobe inactive is pulled high
 */
#define STROBE_POLARITY_LOW 0x00

/**
 * do Read operation: when being transceived, the bytes read by the
 * interface do overwrite the buffer
 */
#define SPI_READ 0x02

/**
 * do Write operation: the bytes are read from the buffer and written
 * to the interface
 */
#define SPI_WRITE 0x01

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
 * @param buffer: the buffer memory index to be used, can be a bufferbuffer, ring buffer or regular buffer
 * @param bufferLength: the maximum length of the buffer
 * @param operationMode: the operation mode for the strobe pin and read /write handling.
 *  The byte contains the mode and polarity of the strobe signal:
 *      - STROBE_ON_TRANSFER
 *      - STROBE_ON_TRANSFER_START
 *      - STROBE_ON_TRANSFER_END
 *      - STROBE_NO_STROBE
 *  logically or'd with:
 *      - STROBE_POLARITY_HIGH
 *      - STROBE_POLARITY_LOW
 *  also contais read / write mode:
 *      - Read enable
 *      - Write enable
 * @return: the initialized structure pointer
 */
SPIOperation* SPI_initSPIOperation(uint8_t strobePin, volatile uint8_t * strobePort, Buffer_void* buffer, uint8_t operationMode);

/**
 * changes the SPI operation strobe mode
 * @param spiop: the spi operation structure to change
 * @param strobeOperation: the operation mode for the strobe pin
 * @return -1 if the structure is being processed, 0 on success
 */
static inline int8_t SPI_changeStrobeOperaton(SPIOperation* spiop, uint8_t strobeOperation) __attribute__((always_inline));
static inline int8_t SPI_changeStrobeOperaton(SPIOperation* spiop, uint8_t strobeOperation)
{
    if (0 == spiop->bytesToWrite)
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
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode | SPI_READ);
    }
    else
    {
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode & ~SPI_READ);
    }
}

/**
 * enables / disables the write Operation
 * @param spiop the spi operation structure to change
 * @param enable 0: for each transmit interrupt, 0x00 is written to the interface
 *               1: the bytes from the buffer are written to the interface
 * @return -1 if the structure is being processed, 0 on success
 */
static inline int8_t SPI_changeWriteOperation(SPIOperation* spiop, uint8_t enable) __attribute__((always_inline));
static inline int8_t SPI_changeWriteOperation(SPIOperation* spiop, uint8_t enable)
{
    if (enable)
    {
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode | SPI_WRITE);
    }
    else
    {
        return SPI_changeStrobeOperaton(spiop, spiop->operationMode & ~SPI_WRITE);
    }
}

/**
 * !only to be called internally!
 * schedules the strobe operation, depending on the operation mode of the active SPI operation
 */
static inline void SPI_scheduleStrobe() __attribute__((always_inline));
static inline void SPI_scheduleStrobe()
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

/**
 * reads from the buffer of the active operation or writes 0x00 to the interface,
 * depending on the Write-Status of the active operation.
 * Does not write to the interface if buffer is empty. in this case, -1 is returned.
 * any positive integer including 0 identify a successful write
 */
static inline int8_t SPI_nextByte_Write() __attribute__((always_inline));
static inline int8_t SPI_nextByte_Write()
{
    if (spiOperation_mem[g_SPI_activeTransmission].operationMode & SPI_WRITE)
    {
        if (BasicBuffer_uint8_get(getBuffer_void(spiOperation_mem[g_SPI_activeTransmission].buffer),
                               SR_SPIinterface_writeAddress) != -1)
        {
            BasicBuffer_increment_index_pop(getBuffer_void(spiOperation_mem[g_SPI_activeTransmission].buffer));
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        *SR_SPIinterface_writeAddress = 0x00;
        return 0;
    }
}

static inline void SPI_nextByte_Read() __attribute__((always_inline));
static inline void SPI_nextByte_Read()
{
    if (spiOperation_mem[g_SPI_activeTransmission].operationMode & SPI_READ)
    {
        if (BasicBuffer_uint8_set(getBuffer_void(spiOperation_mem[g_SPI_activeTransmission].buffer),
                               SR_SPIinterface_readAddress) != -1)
        {
            spiOperation_mem[g_SPI_activeTransmission].bytesReceived += 1;
            BasicBuffer_increment_index_put(getBuffer_void(spiOperation_mem[g_SPI_activeTransmission].buffer));
            return;
        }
    }
    dummyReadByte = *SR_SPIinterface_readAddress;
}

/**
 * interrupt service routine call
 * reads the next byte and puts it into the active SPIOperation's buffer
 * if the buffer is full, a dummy read is initiated
 */
static inline void SPI_nextByte_ISR_read() __attribute__((always_inline));
static inline void SPI_nextByte_ISR_read()
{
    if (g_SPI_activeTransmission != -1)
    {
        SPI_nextByte_Read();
        spiOperation_mem[g_SPI_activeTransmission].bytesToRead -= 1;
        if (spiOperation_mem[g_SPI_activeTransmission].bytesToRead == 0)
        {
            SPI_scheduleStrobe();
        }
    }
    else
    {
        dummyReadByte = *SR_SPIinterface_readAddress;
    }
}

/**
 * interrupt service routine call
 * writes the next byte of the active shift register into the address specified
 * by the init function @see SR_initWriteReadAddress
 * @return -1 in case no byte is written, 1 in case byte was available
 */
static inline int8_t SPI_nextByte_ISR_write() __attribute__((always_inline));
static inline int8_t SPI_nextByte_ISR_write()
{
    if (g_SPI_activeTransmission != -1)
    {
        if (spiOperation_mem[g_SPI_activeTransmission].bytesToWrite > 0)
        {
            if (SPI_nextByte_Write() == -1)
            {
                g_SPI_activeTransmission = -1;
                return -1;
            }
            spiOperation_mem[g_SPI_activeTransmission].bytesToWrite -= 1;
            return 0;
        }
        else
        {
            if (!(spiOperation_mem[g_SPI_activeTransmission].operationMode & SPI_READ))
            {
                SPI_scheduleStrobe();
            }
            return -1;
        }
    }
    return -1;
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
        sr->bytesToWrite = bytesToProcess;
        if (sr->operationMode & SPI_READ)
        {
            sr->bytesToRead = bytesToProcess;
        }
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
