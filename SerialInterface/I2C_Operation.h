/*
 * I2C_Operation.h
 *
 *  Created on: 12.03.2017
 *      Author: Richard
 */

#ifndef I2C_OPERATION_H_
#define I2C_OPERATION_H_

#include <RSOSDefines.h>

#ifdef I2CDATASIZE

#include "../buffer/BasicBuffer_int8.h"
#include <stdint.h>

#include <HardwareAdaptionLayer.h>

/**
 * structure definition for I2C transmit / receive data
 * Fields:
 *      buffer: the buffer for data to read from / write to
 *      bytesToWrite: the number of bytes to transmit
 *      bytesToRead: the number of bytes to read
 *      slaveAddress: the address of the slave. only 7Bit address is supported
 *
 * if an instance is activated, the slave address is transferred to the slave address register.
 * if bytes to write is not equal 0, the interface is set to write.
 * the number of bytes to write is read from the buffer and transferred.
 * when bytesToWrite is 0, and bytesToRead is not 0, a repeated start condition will be transferred,
 * else a stop condition is transferred.
 * after the repeated start condition, the slave address is transferred again with the last bit set to read
 * as long as bytesToRead is not 0, the incoming bytes are acknowledged.
 * when bytesToRead is 0, a NACK and stop condition is transferred.
 */
typedef struct I2C_Data_t {
    int8_t buffer;
    uint8_t bytesToWrite;
    uint8_t bytesToRead;
    uint8_t slaveAddress;
//    uint8_t status;
} I2C_Data;

extern I2C_Data i2c_data_mem[I2CDATASIZE];
extern int8_t i2c_data_size;

extern int8_t activeI2CTransmission;

extern volatile unsigned char * i2c_readAddress;
extern volatile unsigned char * i2c_writeAddress;
extern volatile unsigned char * i2c_controlAddress;
extern volatile uint8_t g_I2C_dummyReadByte;

/**
 * identifier is active: the currently transferred I2COperation is
 * marked with this bit in the field slaveAddress (because the address
 * is only 7 Bit)
 */
#define I2C_ISACTIVE 0x80

/**
 * the address mask to mask the address from the slaveAddress field
 */
#define I2C_addressMask 0x7F

/**
 * set the address to write to.
 * MSP Devices: something like UCA0TXBUF
 */
static inline void I2C_initWriteAddress(volatile unsigned char * address) __attribute__((always_inline));
static inline void I2C_initWriteAddress(volatile unsigned char * address)
{
    i2c_writeAddress = address;
}

/**
 * set the address to read from.
 * MSP Devices: something like UCA0RXBUF
 */
static inline void I2C_initReadAddress(volatile unsigned char * address) __attribute__((always_inline));
static inline void I2C_initReadAddress(volatile unsigned char * address)
{
    i2c_readAddress = address;
}

/**
 * set the address to control the interface.
 * MSP Devices: something like UCA0CTL1
 */
static inline void I2C_initControlAddress(volatile unsigned char * address) __attribute__((always_inline));
static inline void I2C_initControlAddress(volatile unsigned char * address)
{
    i2c_controlAddress = address;
}

/**
 * initialize the Shift Register Operation with the read and write address
 * of the I2C interface
 * @param writeAddress: the address bytes are written into the interface
 * @param readAddress: the address to read from the interface
 * @param controlAddress: the control register address
 */
__EXTERN_C
void I2C_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress, volatile unsigned char * controlAddress);

/**
 * initializes a new I2C_Data structure
 * @param buffer the buffer to use
 * @param slaveAddress the 7 digit right-justified address of the slave device
 */
__EXTERN_C
I2C_Data* I2C_initData(Buffer_void* buffer, uint8_t slaveAddress);

/**
 * checks if the I2C operation is currently active
 * @param data: the operation to check
 * @return 0 if not active, 1 if active
 */
static inline uint8_t I2C_isActive(I2C_Data* data) __attribute__((always_inline));
static inline uint8_t I2C_isActive(I2C_Data* data)
{
    if (data->slaveAddress & I2C_ISACTIVE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static inline void I2C_error() __attribute__((always_inline));
static inline void I2C_error()
{
    if (activeI2CTransmission != -1)
    {
        i2c_data_mem[activeI2CTransmission].bytesToRead = 0;
        i2c_data_mem[activeI2CTransmission].bytesToWrite = 0;
        i2c_data_mem[activeI2CTransmission].slaveAddress &= ~I2C_ISACTIVE;
        activeI2CTransmission = -1;
    }
    I2C_setStop();
}

/**
 * interrupt service routine call
 * writes the next byte from the buffer to the interface
 * @return:
 *      1 in case another byte is available
 *      0 if no byte is transferred (if bytesToRead is not 0, then direction of the
 *        interface is changed and a repeated start condition is sent)
 *     -1 in case of error (no active Transmission, buffer is empty), Transfer interrupt flag is not cleared
 */
static inline int8_t I2C_nextByte_ISR_write() __attribute__((always_inline));
static inline int8_t I2C_nextByte_ISR_write()
{
    if (activeI2CTransmission != -1)
    {
        if (i2c_data_mem[activeI2CTransmission].bytesToWrite > 0)
        {
            if (BasicBuffer_uint8_get( getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer), &I2C_WRITEADDRESS) != -1)
            {
                BasicBuffer_increment_index_pop(getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer));
                i2c_data_mem[activeI2CTransmission].bytesToWrite -= 1;
                return 1;
            }
            else
            {
                i2c_data_mem[activeI2CTransmission].slaveAddress &= ~I2C_ISACTIVE;
                activeI2CTransmission = -1;
            }
        }
        else if (i2c_data_mem[activeI2CTransmission].bytesToRead != 0)
        {
            I2C_unsetInterruptFlag(I2C_IFG_TX);
            I2C_setReceive();
            I2C_setStart();
            return 0;
        }
        else
        {
            I2C_unsetInterruptFlag(I2C_IFG_TX);
            I2C_setStop();
            i2c_data_mem[activeI2CTransmission].slaveAddress &= ~I2C_ISACTIVE;
            activeI2CTransmission = -1;
            return 0;
        }
    }
//    I2C_unsetInterruptFlag(I2C_IFG_TX);
    I2C_setStop();
    return -1;
}

/**
 * interrupt service routine call
 * reads the next byte from the interface into the buffer
 * @return:
 *      1 in case another byte is to read
 *      0 if transmission is done (stop condition is initiated)
 *     -1 on error, the received byte is still read
 */
static inline int8_t I2C_nextByte_ISR_read() __attribute__((always_inline));
static inline int8_t I2C_nextByte_ISR_read()
{
    if (activeI2CTransmission != -1)
    {
        if (i2c_data_mem[activeI2CTransmission].bytesToRead > 0)
        {
            if (BasicBuffer_uint8_set(getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer), &I2C_READADDRESS) != -1)
            {
                BasicBuffer_increment_index_put(getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer));
                i2c_data_mem[activeI2CTransmission].bytesToRead -= 1;
                if (i2c_data_mem[activeI2CTransmission].bytesToRead == 0)
                {
                    I2C_setStop();
                    i2c_data_mem[activeI2CTransmission].slaveAddress &= ~I2C_ISACTIVE;
                    activeI2CTransmission = -1;
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                i2c_data_mem[activeI2CTransmission].slaveAddress &= ~I2C_ISACTIVE;
                activeI2CTransmission = -1;
            }
        }
    }
    g_I2C_dummyReadByte = I2C_READADDRESS;
//    I2C_unsetInterruptFlag(I2C_IFG_RX);
    I2C_setStop();
    return -1;
}

/**
 * interrupt service routine call
 * handles the next byte in the buffer (either write the byte to the transfer address
 * or store the byte from the receive address to the buffer)
 * returns 1 in case another byte is available for reading / writing
 * also handles repeated start, stop, NACK/ACK actions

static inline int8_t I2C_nextByte() __attribute__((always_inline));
static inline int8_t I2C_nextByte()
{

    if (activeI2CTransmission == -1)
    {
        uint8_t readByte = *i2c_readAddress;
        UCB1IFG = 0x00;
        return -1;
    }

    if (i2c_data_mem[activeI2CTransmission].bytesToWrite > 0)
    {
        BasicBuffer_uint8_get( getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer), i2c_writeAddress);
        BasicBuffer_increment_index_pop(getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer));
        i2c_data_mem[activeI2CTransmission].bytesToWrite -= 1;
        return 1;
    }
    else if (i2c_data_mem[activeI2CTransmission].bytesToRead > 0)
    {
//        if (I2C_isTransmitting())  //check if transmitter mode
        if (UCB1CTL1 & UCTR)
        {
            *i2c_controlAddress &= ~UCTR;
            *i2c_controlAddress |= UCTXSTT;
            UCB1IFG &= ~UCTXIFG;
            return 1;
        }
        else
        {
            uint8_t readByte = *i2c_readAddress;
            BasicBuffer_uint8_set(getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer), &readByte);
            BasicBuffer_increment_index_put(getBuffer_void(i2c_data_mem[activeI2CTransmission].buffer));
            i2c_data_mem[activeI2CTransmission].bytesToRead -= 1;
        }
    }

    if (i2c_data_mem[activeI2CTransmission].bytesToWrite == 0 && i2c_data_mem[activeI2CTransmission].bytesToRead == 0)
    {
        *i2c_controlAddress |= UCTXSTP;
        UCB1IFG &= ~UCTXIFG;
        i2c_data_mem[activeI2CTransmission].slaveAddress &= ~I2C_ISACTIVE;
        activeI2CTransmission = -1;
        return -1;
    }
    return 1;
}
*/
/**
 * activates a specific I2C_Data structure
 * @param data: the data to be activated, in other words which bytes
 * are written to the I2C interface / where the bytes are written to read from the interface
 * @param bytesToWrite: the number of bytes to be written.
 * @param bytesToRead: the number of bytes to read. The buffer must be at least (bytesToWrite + bytesToRead) in size
 * @return: -1 if another data is activated and being processed or a positive number (including zero) which
 * identifies the activated data.
 * you want to check the return value. if -1 is returned, try again in the next cycle
 */
static inline int8_t I2C_activateData(I2C_Data* data, uint8_t bytesToWrite, uint8_t bytesToRead) __attribute__((always_inline));
static inline int8_t I2C_activateData(I2C_Data* data, uint8_t bytesToWrite, uint8_t bytesToRead)
{
    if (activeI2CTransmission != -1)
    {
        return -1;
    }
    else
    {
        if (I2C_isBusy())
        {
        	I2C_setStop();
        }

        activeI2CTransmission = data - i2c_data_mem;
        data->bytesToRead = bytesToRead;
        data->bytesToWrite = bytesToWrite;

        I2C_setSlaveAddress(data->slaveAddress & I2C_addressMask);

        data->slaveAddress |= I2C_ISACTIVE;

        if (bytesToWrite != 0)
        {
        	I2C_setTransmit();
        }
        else
        {
            I2C_setReceive();
        }

        I2C_setStart();

        return activeI2CTransmission;
    }
}
#endif /* I2CDATASIZE */
#endif /* I2C_OPERATION_H_ */
