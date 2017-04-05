/*
 * SevenSegmentDisplay.c
 *
 *  Created on: 26.09.2016
 *      Author: Richard
 */

#include "SevenSegmentDisplay.h"
#include "../SerialInterface/SPIOperation.h"

/* exclude everything if not used */
#ifdef SEVENSEGMENTDIGITS

uint8_t sevenSegmentBuffer[SEVENSEGMENTDIGITS];
uint8_t sevenSegmentMode;

Buffer_uint8* buffer[1]
BufferBuffer_uint8* bufferbuffer;

const uint8_t sevenSegment_digits[16] = {0x84, 0xEE, 0x45, 0x46, 0x2E, 0x16, 0x14, 0xCE, 0x04, 0x06, 0x08, 0x30, 0x91, 0x60, 0x11, 0x19};
const uint8_t sevenSegment_digit_Input = 0xB9;
const uint8_t sevenSegment_digit_mute = 0x7F;
const uint8_t sevenSegment_digit_dot = 0xFB;
const uint8_t sevenSegment_digit_off = 0xFF;

ShiftRegisterOperation* sevenSegmentSR;

void SevenSegmentInit(volatile uint8_t * port, uint8_t pin, uint8_t mode)
{
    buffer[0] = (Buffer_uint8*) initBuffer((void*) sevenSegmentBuffer, SEVENSEGMENTDIGITS);
    bufferbuffer = (BufferBuffer_uint8*) initBuffer((void*) buffer, 1);
	sevenSegmentSR = SR_initShiftRegister(pin, port, bufferbuffer, STROBE_ON_TRANSFER_END | STROBE_POLARITY_LOW);
	sevenSegmentMode = mode;
}

int8_t SevenSegment_DisplayNumber(uint16_t number, uint8_t numberOfDigits)
{
	int8_t retVal = -1;
	if (numberOfDigits > SEVENSEGMENTDIGITS)
		return -1;

	switch (sevenSegmentMode) {
case SEVENSEGMENT_MODE_HEX: {
	uint8_t n = 0;
	for ( ; n < numberOfDigits; n+=1)
	{
		sevenSegmentBuffer[n] = sevenSegment_digits[(number & 0x000F)];
		number >>= 4;
	}
	SR_activateShiftRegister(sevenSegmentSR, numberOfDigits);
	retVal = n;
	break;
}
case SEVENSEGMENT_MODE_DEC: {
	uint8_t n = 0;
	for ( ; n < numberOfDigits; n+=1)
	{
		sevenSegmentBuffer[n] = sevenSegment_digits[(number % 10)];
		number /= 10;
	}
	SR_activateShiftRegister(sevenSegmentSR, numberOfDigits);
	retVal = n;
	break;
}
default: retVal = -1; break;
	}
	return retVal;
}

#endif /* SEVENSEGMENTDIGITS */

