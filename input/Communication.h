/*
 * Communication.h
 *
 *  Created on: 12.09.2015
 *      Author: Richard
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

void initUSCI_A_SPI_MASTER();
void initUSCI_B_SPI_MASTER();
void initUSCI_A_SPI_SLAVE();

void storeToSPIBuff(unsigned char data);
unsigned char readSPIBuffFromPos(unsigned char pos);
unsigned char readSPIBuff();
signed char numberOfUnreadData();
void resetBuffer();



#endif /* COMMUNICATION_H_ */
