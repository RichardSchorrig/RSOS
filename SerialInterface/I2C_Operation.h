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

#include <msp430.h>

#include "../buffer/BasicBuffer.h"
#include "../buffer/Buffer_int8.h"
#include "../buffer/BufferBuffer_int8.h"
#include <stdint.h>

#include <HardwareAdaptionLayer.h>

/**
 * structure definition for I2C transmit / receive data
 * Fields:
 *      buffer: the buffer for data to read from / write to
 *      bytesToWrite: the number of bytes to transmit
 *      bytesToRead: the number of bytes to read
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
    BufferBuffer_uint8* buffer;
    uint8_t bytesToWrite;
    uint8_t bytesToRead;
    uint16_t slaveAddress;
//    uint8_t status;
} I2C_Data;

extern I2C_Data i2c_data_mem[I2CDATASIZE];
extern int8_t i2c_data_size;

extern int8_t activeI2CTransmission;

extern volatile unsigned char * i2c_readAddress;
extern volatile unsigned char * i2c_writeAddress;
extern volatile unsigned char * i2c_controlAddress;

/**
 * set the address to write to.
 * MSP Devices: something like UCA0TXBUF
 */
static inline void I2C_initWriteAddress(volatile unsigned char * address);
static void I2C_initWriteAddress(volatile unsigned char * address) {
    i2c_writeAddress = address;
}

/**
 * set the address to read from.
 * MSP Devices: something like UCA0RXBUF
 */
static inline void I2C_initReadAddress(volatile unsigned char * address);
static void I2C_initReadAddress(volatile unsigned char * address) {
    i2c_readAddress = address;
}

/**
 * set the address to control the interface.
 * MSP Devices: something like UCA0CTL1
 */
static inline void I2C_initControlAddress(volatile unsigned char * address);
void I2C_initControlAddress(volatile unsigned char * address) {
    i2c_controlAddress = address;
}

/**
 * initialize the Shift Register Operation with the read and write address
 * of the I2C interface
 * @param writeAddress: the address bytes are written into the interface
 * @param readAddress: the address to read from the interface
 * @param controlAddress: the control register address
 */
void I2C_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress, volatile unsigned char * controlAddress);

/**
 * initializes a new I2C_Data structure
 * @param buffer the buffer to use
 * @param slaveAddress the 7 digit right-justified address of the slave device
 */
I2C_Data* I2C_initData(BufferBuffer_uint8* buffer, uint8_t slaveAddress);

static inline void I2C_error();
static void I2C_error()
{
    if (activeI2CTransmission != -1)
    {
        i2c_data_mem[activeI2CTransmission].bytesToRead = 0;
        i2c_data_mem[activeI2CTransmission].bytesToWrite = 0;
        activeI2CTransmission = -1;
    }
}
/**
 * interrupt service routine call
 * handles the next byte in the buffer (either write the byte to the transfer address
 * or store the byte from the receive address to the buffer)
 * returns 1 in case another byte is available for reading / writing
 * also handles repeated start, stop, NACK/ACK actions
 */
static inline int8_t I2C_nextByte();
static int8_t I2C_nextByte() {

    if (activeI2CTransmission == -1)
    {
        uint8_t readByte = *i2c_readAddress;
        UCB1IFG = 0x00;
        return -1;
    }

    if (i2c_data_mem[activeI2CTransmission].bytesToWrite > 0)
    {
        set_getNext_bufferbuffer_uint8(i2c_data_mem[activeI2CTransmission].buffer, 0, i2c_writeAddress);
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
            set_getNext_bufferbuffer_uint8(i2c_data_mem[activeI2CTransmission].buffer, &readByte, 0);
//            set_getNext_bufferbuffer_uint8(i2c_data_mem[activeI2CTransmission].buffer, i2c_readAddress, 0);
//            UCB1IFG &= ~UCRXIFG;
            i2c_data_mem[activeI2CTransmission].bytesToRead -= 1;
        }
    }

    if (i2c_data_mem[activeI2CTransmission].bytesToWrite == 0 && i2c_data_mem[activeI2CTransmission].bytesToRead == 0)
    {
        *i2c_controlAddress |= UCTXSTP;
        UCB1IFG &= ~UCTXIFG;
        activeI2CTransmission = -1;
        return -1;
    }
    return 1;
}

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
static inline int8_t I2C_activateData(I2C_Data* data, uint8_t bytesToWrite, uint8_t bytesToRead);
static int8_t I2C_activateData(I2C_Data* data, uint8_t bytesToWrite, uint8_t bytesToRead)
{
    if (activeI2CTransmission != -1)
    {
        return -1;
    }
    else
    {
        if (UCB1STAT & UCBBUSY)
        {
//            uint8_t b = *i2c_readAddress;
            UCB1CTL1 |= UCTXSTP;
//            while (UCB1CTL1 & UCTXSTP)
            {
                ;
            }
        }
        activeI2CTransmission = data - i2c_data_mem;
        data->bytesToRead = bytesToRead;
        data->bytesToWrite = bytesToWrite;

        I2C_setSlaveAddress(data->slaveAddress);
        if (bytesToWrite != 0)
        {
            *i2c_controlAddress |= UCTR;
        }
        else
        {
            *i2c_controlAddress &= ~UCTR;
        }

        *i2c_controlAddress |= UCTXSTT;

        return activeI2CTransmission;
    }
}
#endif /* I2CDATASIZE */
#endif /* I2C_OPERATION_H_ */
