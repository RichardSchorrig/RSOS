/*
 * I2C_Operation.c
 *
 *  Created on: 12.03.2017
 *      Author: Richard
 */

#include "I2C_Operation.h"

#ifdef I2CDATASIZE

volatile uint8_t g_I2C_dummyReadByte = 0;

void I2C_initOperation(volatile unsigned char * writeAddress, volatile unsigned char * readAddress, volatile unsigned char * controlAddress)
{
    I2C_initWriteAddress(writeAddress);
    I2C_initReadAddress(readAddress);
    I2C_initControlAddress(controlAddress);
}

I2C_Data* I2C_initData(Buffer_void* buffer, uint8_t slaveAddress)
{
    i2c_data_mem[i2c_data_size].buffer = BasicBuffer_getNumber(buffer);
    i2c_data_mem[i2c_data_size].slaveAddress = slaveAddress;
    i2c_data_mem[i2c_data_size].bytesToRead = 0;
    i2c_data_mem[i2c_data_size].bytesToWrite = 0;

    i2c_data_size += 1;
    return & i2c_data_mem[i2c_data_size - 1];
}


#endif /* I2CDATASIZE */
