/*
 * SevenSegmentDisplay.h
 *
 *  Created on: 26.09.2016
 *      Author: Richard
 */

#ifndef SEVENSEGMENTDISPLAY_H_
#define SEVENSEGMENTDISPLAY_H_

#include <RSOSDefines.h>
#include <stdint.h>

/* exclude everything if not used */
#ifdef SEVENSEGMENTDIGITS

#define SEVENSEGMENT_MODE_HEX 0
#define SEVENSEGMENT_MODE_DEC 1

/**
 * 		___
 * 		 A
 * |F	   		|B
 * 		___
 *		 G
 * |E	   		|C
 * 		___
 * 		 D		   #
 * 				   H
 * BAFGCEHD
 */


void SevenSegmentInit(volatile uint8_t * port, uint8_t pin, uint8_t mode);
int8_t SevenSegment_DisplayNumber(uint16_t number, uint8_t numberOfDigits);


#endif /* SEVENSEGMENTDIGITS */
#endif /* SEVENSEGMENTDISPLAY_H_ */
