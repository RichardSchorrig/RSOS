/*
 * Communication.c
 *
 *  Created on: 12.09.2015
 *      Author: Richard
 */

#include "Communication.h"
#include "../Path.h"
#include <msp430.h>

#include PATH_RSOSDEFINES_H

/**
 * inits the USCI_B module to be configured as master
 */
void initUSCI_B_SPI_MASTER()
{
    UCB0CTL1 |= UCSWRST;                    //Disable USCI for initialization
    UCB0CTL0 |= UCCKPH | UCMST | UCMODE_0 | UCSYNC | UCMSB; //data is changed on following edge, Master Mode, 3-Wire Spi, MSB fist out
    UCB0CTL1 |= UCSSEL_2 | UCSWRST;         //SMCLK selected
    UCB0BR0 = 2;                            //Divide by 2, Clock Speed is 750kHz
    UCB0BR1 = 0;

//    P1SEL |= USCI_B_CLOCK | USCI_B_MOSI;    //P1.5: Clock out, P1.7: MOSI
//    P1SEL2 |= USCI_B_CLOCK | USCI_B_MOSI;

//  IE2 |= UCB0TXIE | UCB0RXIE;             //Enable both TX and RX interrupts

    UCB0CTL1 &= ~UCSWRST;                   //Enable USCI
}

/**
 * inits the USCI_A module to be configured as master
 */
void initUSCI_A_SPI_MASTER()
{
	UCA0CTL1 |= UCSWRST;					//Disable USCI for initialization
	UCA0CTL0 |= UCCKPH | UCMST | UCMODE_0 | UCSYNC | UCMSB;	//data is changed on following edge, Master Mode, 3-Wire Spi, MSB first
	UCA0CTL1 |= UCSSEL_2 | UCSWRST;			//SMCLK selected
	UCA0BR0 = 4;							//Divide by 4, Clock Speed is 655.36kHz
	UCA0BR1 = 0;

//	P2SEL |= UCA0CLK;
//	P3SEL |= UCA0SOMI | UCA0SIMO;

	UCA0CTL1 &= ~UCSWRST;					//Enable USCI

	UCA0IE |= UCTXIE;// | UCRXIE;				//Enable both TX and RX interrupts
}

/**
 * inits the USCI_A module as slave
 */
void initUSCI_A_SPI_SLAVE()
{
	UCA0CTL1 |= UCSWRST;					//Disable USCI for initialization
	UCA0CTL0 |= UCCKPH | UCMODE_0 | UCSYNC | UCMSB;	//data is changed on following edge, Slave Mode, 3-Wire Spi, MSB fist out
	UCA0CTL1 |= UCSSEL_2 | UCSWRST;			//SMCLK selected


//	P1SEL |= USCI_A_CLOCK | USCI_A_SIMO;			//P1.4: Clock out, P1.1: SOMI, P1.2: MOSI
//	P1SEL2 |= USCI_A_CLOCK | USCI_A_SIMO;

	UCA0CTL1 &= ~UCSWRST;					//Enable USCI
}

/**
 * length of the receive buffer
 */
const unsigned char SPIBuffSize = SPIBUFFSIZE;

/**
 * the number of unread bytes in the buffer
 */
static unsigned char spiUnreadData = 0;

/**
 * the spi receive buffer
 */
static unsigned char spiReceiveData[SPIBUFFSIZE];

/**
 * the spi buffer position
 */
static unsigned char spiWriteBuffPosition = 0;
static unsigned char spiReadBuffPosition = 0;

/**
 * Stores data into a ring buffer which is SPIBUFFSIZE long
 */
void storeToSPIBuff(unsigned char data)
{
	spiReceiveData[spiWriteBuffPosition] = data;

	if (++spiWriteBuffPosition >= SPIBuffSize)
		spiWriteBuffPosition = 0;

	spiUnreadData += 1;
}

/**
 * reads the data at a position smaller than the buffer size
 */
unsigned char readSPIBuffFromPos(unsigned char pos)
{
	if (spiWriteBuffPosition - ++pos < 0)
	{
		return spiReceiveData[SPIBuffSize - (pos-spiWriteBuffPosition)];
	}
	else
	{
		return spiReceiveData[spiWriteBuffPosition-pos];
	}
}

unsigned char readSPIBuff()
{
	unsigned char returnValue;
	if (spiUnreadData > 0)
	{
		returnValue = spiReceiveData[spiReadBuffPosition];
		if (++spiReadBuffPosition >= SPIBuffSize)
			spiReadBuffPosition = 0;

		spiUnreadData -= 1;
	}
	else
	{
		returnValue = 0x00;
	}
	return returnValue;
}

/**
 * returns the number of unread data inside the spi receive buffer
 */
signed char numberOfUnreadData()
{
	return spiUnreadData;
}

void resetBuffer()
{
	spiReadBuffPosition = spiWriteBuffPosition;
}



