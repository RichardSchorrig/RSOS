/*
 * DotMatrixDisplay.c
 *
 *  Created on: 02.10.2016
 *      Author: Richard
 */

#include "DotMatrixDisplay.h"

#include "ShiftRegisterOperation.h"

ShiftRegisterOperation* dotMatrixSR;

/**
 * buffers
 */
uint8_t dotMatrix_displayBuffer[DOTMATRIX_DISPLAY_XRES][DOTMATRIX_DISPLAY_LINES];
uint8_t dotMatrix_displayCommandBuffer[DOTMATRIX_DISPLAY_XRES + 5];

void DotMatrix_initDisplay()
{
	dotMatrixSR = SR_initShiftRegister(2, dotMatrix_displayCommandBuffer, DOTMATRIX_DISPLAY_XRES + 5);

	dotMatrix_displayCommandBuffer[0] = COMMAND_START | COMMAND_START_CS1 | COMMAND_START_CS2 | COMMAND_START_A0;
	dotMatrix_displayCommandBuffer[1] = 6;
	dotMatrix_displayCommandBuffer[2] = COMMAND_RESET;
	dotMatrix_displayCommandBuffer[3] = COMMAND_DISPLAYDRIVER_ON;
	dotMatrix_displayCommandBuffer[4] = COMMAND_STATICDRIVE_OFF;
	dotMatrix_displayCommandBuffer[5] = COMMAND_SETLINE;
	dotMatrix_displayCommandBuffer[6] = COMMAND_SETPAGE;
	dotMatrix_displayCommandBuffer[7] = 0;

	SR_activateShiftRegister(dotMatrixSR, 8);
}

DisplayElement* DotMatrix_newDisplayElement(uint8_t xpos, uint8_t ypos, uint8_t xsize, uint8_t ysize)
{
	DotMatrix_mem[DotMatrix_size].pos_x = xpos;
	DotMatrix_mem[DotMatrix_size].pos_y = ypos;
	DotMatrix_mem[DotMatrix_size].len_x = xsize;
	DotMatrix_mem[DotMatrix_size].height_y = ysize;
	DotMatrix_mem[DotMatrix_size].buffer = &dotMatrix_displayBuffer[xpos][(ypos >> 3)];

	DotMatrix_size += 1;
	return DotMatrix_mem[DotMatrix_size - 1];
}

inline int8_t DotMatrix_changeElement_betweenLines(DisplayElement* delm,
												   uint8_t xpos, uint8_t ypos,
												   uint8_t * data, uint8_t datalen,
												   uint8_t line_upper, uint8_t offset_upper)
{
	uint8_t line_lower;
	uint8_t offset_lower;
	uint8_t mask_upper = 0;
	uint8_t mask_lower = 0;

	if (delm->height_y > 8)
	{
		line_lower = line_upper + 1;
	}
	else
	{
		line_lower = (ypos + delm->height_y) >> 3;
	}

	if (delm->height_y > 8)
	{
		offset_lower = 8 - offset_upper;
	}
	else
	{
		offset_lower = 8 - (ypos + delm->height_y) & 0x07;
	}

	int8_t i;
	for (i=offset_upper; i>0;i-=1)
	{
		mask_upper <<= 1;
		mask_upper += 0x01;
	}
	for (i=offset_lower; i>0;i-=1)
	{
		mask_upper >>= 1;
		mask_upper += 0x80;
	}
	i = datalen > delm->len_x ? delm->len_x : datalen;
	for ( ; i>0; i-=1)
	{
		dotMatrix_displayBuffer[xpos][line_upper] &= ~mask_upper;
		dotMatrix_displayBuffer[xpos][line_upper] |= ((data >> offset_upper) & mask_upper);
		xpos += 1;
	}

	if (line_upper != line_lower)	//dont do it if no additional line must be written
	{
		i = datalen > delm->len_x ? delm->len_x : datalen;
		xpos -= i;
		for ( ; i>0; i-=1)
		{
			dotMatrix_displayBuffer[xpos][line_lower] &= ~mask_lower;
			dotMatrix_displayBuffer[xpos][line_lower] |= ((data << (8-offset_upper)) & mask_lower);
			xpos += 1;
		}
	}
}

int8_t DotMatrix_changeElementN(DisplayElement* delm, uint8_t xpos, uint8_t ypos, uint8_t * data, uint8_t datalen)
{
	uint8_t line = delm->height_y >> 3;
	uint8_t offset = delm->heigt % 0x07;

	if (offset != 0)
	{
		return DotMatrix_changeElement_betweenLines(delm, xpos, ypos, data, datalen, line, offset);
	}
	else
	{

	}

}

int8_t DotMatrix_changeElement(DisplayElement* delm, uint8_t xpos, uint8_t ypos, uint8_t * data)
{

}


