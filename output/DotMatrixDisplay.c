/*
 * DotMatrixDisplay.c
 *
 *  Created on: 02.10.2016
 *      Author: Richard
 */

#include "DotMatrixDisplay.h"

/* exclude everything if not used */
#ifdef DOTMATRIX_MEMSIZE

#include "../SerialInterface/SPIOperation.h"
#include <DisplayHardware.h>

SPIOperation* dotMatrixSR;

static const uint16_t dotMatrix_activeBit = 0x8000;
static const uint16_t dotMatrix_isInverted = 0x4000;
static const uint16_t dotMatrix_lineMask = 0x00F0;
static const uint16_t dotMatrix_additionalLineMask = 0x000F;

/**
 * buffers
 */
uint8_t dotMatrix_displayBuffer[DOTMATRIX_DISPLAY_LINES][DOTMATRIX_DISPLAY_XRES];
uint8_t dotMatrix_displayCommandBuffer[10];

static BufferBuffer_uint8 * dotMatrix_dataBufferBuffer;
static Buffer_uint8 * dotMatrix_command_and_data_Buffer[2];

static Task * dotMatrix_task_transferElement = 0;

//static DisplayElement* displayElement_entireDisplay = 0;

void DotMatrix_initDisplay(volatile uint8_t * port, uint8_t pin)
{

    dotMatrix_command_and_data_Buffer[0] = (Buffer_uint8*) initBuffer((void*)dotMatrix_displayCommandBuffer, 10, BUFFER_TYPE_REGULAR);
    dotMatrix_command_and_data_Buffer[1] = (Buffer_uint8*) initBuffer((void*)&dotMatrix_displayBuffer[0][0], DOTMATRIX_DISPLAY_XRES, BUFFER_TYPE_REGULAR);
    dotMatrix_dataBufferBuffer = (BufferBuffer_uint8*) initBuffer((void*) dotMatrix_command_and_data_Buffer, 2, BUFFER_TYPE_BUFFERBUFFER);
	dotMatrixSR = SPI_initSPIOperation(pin, port, (Buffer_void*)dotMatrix_dataBufferBuffer, STROBE_ON_TRANSFER | STROBE_POLARITY_LOW);

	dotMatrix_task_transferElement = addTask(1, DotMatrix_transferElement);
	setTaskCyclic(dotMatrix_task_transferElement, 2);

	dotMatrix_displayCommandBuffer[0] = COMMAND_START | COMMAND_START_CS1 | COMMAND_START_CS2 | COMMAND_START_A0;
	dotMatrix_displayCommandBuffer[1] = 6;
	dotMatrix_displayCommandBuffer[2] = COMMAND_RESET;
	dotMatrix_displayCommandBuffer[3] = COMMAND_DISPLAYDRIVER_ON;
	dotMatrix_displayCommandBuffer[4] = COMMAND_STATICDRIVE_OFF;
	dotMatrix_displayCommandBuffer[5] = COMMAND_SETLINE;
	dotMatrix_displayCommandBuffer[6] = COMMAND_SETPAGE;
	dotMatrix_displayCommandBuffer[7] = 0;

	setBufferLength((Buffer_void*) dotMatrix_command_and_data_Buffer[0], 8);
	setBufferLength((Buffer_void*) dotMatrix_command_and_data_Buffer[1], 0);

	SPI_activateSPIOperation(dotMatrixSR, 8);   //todo: check return value, might not be activated
}

/*
inline DisplayElement* getIfActive(uint8_t lineToMatch, uint8_t index) {
    DisplayElement* retVal = 0;
    if (DotMatrix_mem[index].status & activeBit) {
        if ( ( (DotMatrix_mem[index].status & lineMask) >> 4 ) == lineToMatch) {
            retVal = &DotMatrix_mem[index];
        }
    }
    return retVal;
}
*/

static DisplayElement* currentDisplayElement = 0;

void DotMatrix_transferElement() {
    static int8_t noSROperation = 0;
    static int8_t currentLine = -1;
    static int8_t totalLines = -1;
//    static int8_t setPosition = 0;
    static uint8_t bufferlength = 0;
    int8_t noDelms = 0;

    if (noSROperation != -1) {
        if (currentDisplayElement == 0) {
            int8_t i = dotMatrix_size;
            for (; i>0; i-=1) {

                if (dotMatrix_mem[i-1].status & dotMatrix_activeBit) {
                    currentDisplayElement = &dotMatrix_mem[i-1];
                    bufferlength = 0;

                    totalLines = (currentDisplayElement->status & dotMatrix_additionalLineMask) + 1;
                    currentLine = ((currentDisplayElement->status & dotMatrix_lineMask) >> 4);


                    break;
                }

                else {
                    noDelms += 1;
                }
            }
            if (noDelms == dotMatrix_size)
            {
                // signal scheduler to stop running this task
                task_mem[currentRunningTask].currentCycle = 0;

                currentDisplayElement = 0;
                return;
            }
        }

        if (currentDisplayElement != 0) {

//            if ((dotMatrix_command_and_data_Buffer[0]->size.readBytes == dotMatrix_command_and_data_Buffer[0]->size.size) &&
//                (dotMatrix_command_and_data_Buffer[1]->size.readBytes == dotMatrix_command_and_data_Buffer[1]->size.size))

            if (dotMatrixSR->bytesToWrite == 0)
            {
                bufferlength = 0;
                bufferlength += setCommandPosition(currentLine, currentDisplayElement->pos_x, dotMatrix_displayCommandBuffer, 0, 10);
                bufferlength += setCommandData(currentDisplayElement->len_x, currentDisplayElement->pos_x, dotMatrix_displayCommandBuffer, bufferlength, 10);
                setBufferLength((Buffer_void*) dotMatrix_command_and_data_Buffer[0], bufferlength);
                setBuffer((Buffer_void*) dotMatrix_command_and_data_Buffer[1],
                          (void*) &dotMatrix_displayBuffer[currentLine][currentDisplayElement->pos_x],
                          currentDisplayElement->len_x);
                resetBuffer((Buffer_void*)dotMatrix_dataBufferBuffer);
                bufferlength += currentDisplayElement->len_x;
                currentLine += 1;
                totalLines -= 1;

                if (totalLines <= 0) {
                    currentDisplayElement->status &= ~dotMatrix_activeBit;
                    currentDisplayElement = 0;
                    currentLine = -1;
                    totalLines = -1;
                }

                noSROperation = SPI_activateSPIOperation(dotMatrixSR, bufferlength);
            }

            task_mem[currentRunningTask].currentCycle = 2;
        }
    }
    else
    {
        // add some cycle (it doesn't matter how many cycles since it is repeated and checked each time
        task_mem[currentRunningTask].currentCycle = 2;
        noSROperation = SPI_activateSPIOperation(dotMatrixSR, bufferlength);
    }
}

DisplayElement* DotMatrix_newDisplayElement(uint8_t xpos, uint8_t ypos, uint8_t xsize, uint8_t ysize, uint8_t showInverted)
{
	dotMatrix_mem[dotMatrix_size].pos_x = xpos;
	dotMatrix_mem[dotMatrix_size].pos_y = ypos;
	dotMatrix_mem[dotMatrix_size].len_x = xsize;
	dotMatrix_mem[dotMatrix_size].height_y = ysize;
//	dotMatrix_mem[dotMatrix_size].buffer = &dotMatrix_displayBuffer[xpos][(ypos >> 3)];

	dotMatrix_mem[dotMatrix_size].status = 0;
	uint8_t baseline = (ypos >> 3);
	uint8_t furtherlines = 0;
	if (ysize > 8) {
	    furtherlines = (ysize-1) >> 3;
	}
	dotMatrix_mem[dotMatrix_size].status |= (baseline << 4) & dotMatrix_lineMask;
	dotMatrix_mem[dotMatrix_size].status |= furtherlines;
	if (showInverted) {
	    dotMatrix_mem[dotMatrix_size].status |= dotMatrix_isInverted;
	}
	dotMatrix_size += 1;
	return &dotMatrix_mem[dotMatrix_size - 1];
}

inline int8_t DotMatrix_changeElement_inLine(DisplayElement* delm,
                                             int16_t xpos, int16_t ypos,
                                             const uint8_t * data, uint8_t datalen,
                                             uint8_t line, uint8_t offset);
int8_t DotMatrix_changeElement_inLine(DisplayElement* delm,
                                             int16_t xpos, int16_t ypos,
                                             const uint8_t * data, uint8_t datalen,
                                             uint8_t line, uint8_t offset)
{
    int8_t pos = -1;
    if (xpos < 0) {
        pos = 0-xpos - 1;
        datalen += xpos;
    }
    if (offset == 0) {
        int8_t i = datalen > (delm->len_x - xpos) ? (delm->len_x - xpos) : datalen;
        for (; i>0; i-=1) {
            pos += 1;
            if (delm->status & dotMatrix_isInverted) {
                dotMatrix_displayBuffer[line][delm->pos_x+xpos+pos] = ~data[pos];
            }
            else {
                dotMatrix_displayBuffer[line][delm->pos_x+xpos+pos] = data[pos];
            }
        }
    }
    else {
        int8_t i;
        uint8_t mask = 0;
        for (i=8-offset; i>0;i-=1)
        {
            mask >>= 1;
            mask |= 0x80;
        }
        i = datalen > delm->len_x ? delm->len_x : datalen;
        for (; i>0; i-=1) {
            pos += 1;
            dotMatrix_displayBuffer[line][delm->pos_x+xpos+pos] &= ~mask;
            if (delm->status & dotMatrix_isInverted) {
                dotMatrix_displayBuffer[line][delm->pos_x+xpos+pos] |= ( (~(data[pos]<<offset)) & mask );
            }
            else {
                dotMatrix_displayBuffer[line][delm->pos_x+xpos+pos] |= ( (data[pos]<<offset) & mask );
            }
        }
    }
    return pos;
}

inline int8_t DotMatrix_changeElement_betweenLines(DisplayElement* delm,
                                                   int16_t xpos, int16_t ypos,
                                                   const uint8_t * data, uint8_t datalen,
                                                   int8_t line_upper, uint8_t offset_upper,
                                                   uint8_t line_lower, uint8_t offset_lower);
int8_t DotMatrix_changeElement_betweenLines(DisplayElement* delm,
                                                   int16_t xpos, int16_t ypos,
												   const uint8_t * data, uint8_t datalen,
												   int8_t line_upper, uint8_t offset_upper,
												   uint8_t line_lower, uint8_t offset_lower)
{
	uint8_t mask_upper = 0;
	uint8_t mask_lower = 0;

	int8_t i;
    if (line_upper >= 0) {
        for (i=8-offset_upper; i>0;i-=1)
        {
            mask_upper >>= 1;
            mask_upper |= 0x80;
        }
    }
    for (i=offset_lower; i>0;i-=1)
    {
        mask_lower <<= 1;
        mask_lower |= 0x01;
    }

	int8_t pos = -1;
    if (xpos < 0) {
        pos = 0-xpos - 1;
        datalen += xpos;
    }
    i = datalen > (delm->len_x - xpos) ? (delm->len_x - xpos) : datalen;
	for ( ; i>0; i-=1)
	{
        pos += 1;
		dotMatrix_displayBuffer[line_upper][delm->pos_x+xpos+pos] &= ~mask_upper;
        dotMatrix_displayBuffer[line_lower][delm->pos_x+xpos+pos] &= ~mask_lower;
        if (delm->status & dotMatrix_isInverted) {
            if (line_upper >= 0) {
                dotMatrix_displayBuffer[line_upper][delm->pos_x+xpos+pos] |= ( (~(data[pos] << offset_upper)) & mask_upper);
            }
            dotMatrix_displayBuffer[line_lower][delm->pos_x+xpos+pos] |= ( (~(data[pos] >> (8-offset_upper))) & mask_lower);
        }
        else {
            if (line_upper >= 0) {
                dotMatrix_displayBuffer[line_upper][delm->pos_x+xpos+pos] |= ( ((data[pos] << offset_upper)) & mask_upper);
            }
            dotMatrix_displayBuffer[line_lower][delm->pos_x+xpos+pos] |= ( ((data[pos] >> (8-offset_upper))) & mask_lower);
        }
	}

	return pos;
}

int8_t DotMatrix_scroll(uint8_t line) {
    if (line > 31)
        return -1;

    dotMatrix_displayCommandBuffer[0] = COMMAND_START | COMMAND_START_CS1 | COMMAND_START_CS2 | COMMAND_START_A0;
    dotMatrix_displayCommandBuffer[1] = 1;
    dotMatrix_displayCommandBuffer[2] = COMMAND_SETLINE + line;

    setBufferLength((Buffer_void*) dotMatrix_command_and_data_Buffer[0], 3);
    setBufferLength((Buffer_void*) dotMatrix_command_and_data_Buffer[1], 0);
    resetBuffer((Buffer_void*) dotMatrix_dataBufferBuffer);

    return SPI_activateSPIOperation(dotMatrixSR, 3);
}

int8_t DotMatrix_CleanDisplay() {
    uint8_t i = DOTMATRIX_DISPLAY_LINES;
    for (; i>0; i-=1) {
        uint8_t j = DOTMATRIX_DISPLAY_XRES;
        for (; j>0; j-=1)
        dotMatrix_displayBuffer[i-1][j-1] = 0;
    }

    i = dotMatrix_size;
    for (; i>0; i-=1) {
        DotMatrix_activateElement(&dotMatrix_mem[i-1]);
    }
    return dotMatrix_size;
/*
    dotMatrix_displayCommandBuffer[0] = COMMAND_START | COMMAND_START_CS1 | COMMAND_START_CS2 | COMMAND_START_A0;
    dotMatrix_displayCommandBuffer[1] = 2;
    dotMatrix_displayCommandBuffer[2] = COMMAND_RESET;

    setBufferLength((Buffer_void*) command_and_data_Buffer[0], 3);
    setBufferLength((Buffer_void*) command_and_data_Buffer[1], 0);
    resetBuffer((Buffer_void*) buffer);

    return SR_activateShiftRegister(dotMatrixSR, 3);
*/
}

inline int8_t clearLine(uint8_t mask, uint8_t invert, uint8_t line, uint8_t pos_x, uint8_t len);
int8_t clearLine(uint8_t mask, uint8_t invert, uint8_t line, uint8_t pos_x, uint8_t len) {
    int8_t pos = -1;
    for (; len>0; len-=1) {
        pos += 1;
        if (invert == 1) {
            dotMatrix_displayBuffer[line][pos_x + pos] |= mask;
        }
        else {
            dotMatrix_displayBuffer[line][pos_x + pos] &= ~mask;
        }
    }
    return pos;
}

int16_t DotMatrix_cleanElement(DisplayElement* delm) {
    int16_t columns = -1;
    uint8_t line = (delm->status & dotMatrix_lineMask) >> 4;
    uint8_t linesTodo = (delm->status & dotMatrix_additionalLineMask) + 1;
    uint8_t lastLine = line + linesTodo - 1;
    uint8_t upperMask = 0xFF;
    uint8_t lowerMask = 0xFF;

    if (delm->pos_y & 0x07) {
        uint8_t i = delm->pos_y & 0x07;
        for (; i>0; i-=1) {
            upperMask <<= 1;
        }
        if (linesTodo > 1) {
            columns += clearLine(upperMask, delm->status & dotMatrix_isInverted, line, delm->pos_x, delm->len_x);
            line += 1;
            linesTodo -= 1;
        }
    }

    if ((delm->pos_y + delm->height_y - 1) & 0x07) {
        uint8_t i = 8 - ((delm->pos_y + delm->height_y - 1) & 0x07);
        for (; i>0; i-=1) {
            lowerMask >>= 1;
        }
        if (linesTodo > 1) {
            columns += clearLine(lowerMask, delm->status & dotMatrix_isInverted, lastLine, delm->pos_x, delm->len_x);
            linesTodo -= 1;
        }
    }

    if (linesTodo == 1 && columns == -1) {
        upperMask &= lowerMask; // combine both masks
        return clearLine(upperMask, delm->status & dotMatrix_isInverted, line, delm->pos_x, delm->len_x);
    }

    for (; linesTodo > 0; linesTodo-=1) {
        uint8_t xpos = delm->len_x;
        for (; xpos > 0; xpos-=1) {
            columns += clearLine(0xFF, delm->status & dotMatrix_isInverted, line, delm->pos_x, delm->len_x);
            line += 1;
        }
    }

    return columns;
}

int8_t DotMatrix_changeElementN(DisplayElement* delm, int16_t xpos, int16_t ypos, const uint8_t * data, uint8_t datalen)
{

    if (delm->status & dotMatrix_activeBit) {
        return -1;
    }
    if (delm->height_y <= ypos || ypos < -7) {
        return -1;
    }
    if (delm->len_x <= xpos || 0-xpos >= datalen) {
        return -1;
    }

	uint8_t yEnd;
	if (delm->height_y > (ypos + 8)) {
	    yEnd = delm->pos_y+ypos + 8;
	}
	else {
	    yEnd = delm->pos_y+delm->height_y;
	}

	int8_t line_upper = (delm->pos_y + ypos) >> 3;
	uint8_t offset_upper = (delm->pos_y + ypos) & 0x07;
	int8_t line_lower = (yEnd-1) >> 3;

	if (line_upper != line_lower)
	{
	    uint8_t offset_lower = (yEnd) & 0x07;
		return DotMatrix_changeElement_betweenLines(delm, xpos, ypos, data, datalen, line_upper, offset_upper, line_lower, offset_lower);
	}
	else
	{
	    return DotMatrix_changeElement_inLine(delm, xpos, ypos, data, datalen, line_upper, offset_upper);
	}
}

int8_t DotMatrix_changeElement(DisplayElement* delm, int16_t xpos, int16_t ypos, const uint8_t data)
{
    return DotMatrix_changeElementN(delm, xpos, ypos, &data, 1);
}

void DotMatrix_activateElement(DisplayElement* delm) {
    delm->status |= dotMatrix_activeBit;
    scheduleTask(dotMatrix_task_transferElement);
}

#endif /* DOTMATRIX_MEMSIZE */
