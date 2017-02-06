/*
 * DotMatrixDisplay.h
 *
 *  Created on: 02.10.2016
 *      Author: Richard
 */

#ifndef DOTMATRIXDISPLAY_H_
#define DOTMATRIXDISPLAY_H_

#include <stdint.h>
#include "../Path.h"
#include PATH_RSOSDEFINES_H

/**
 * sizes
 */
#define DOTMATRIX_DISPLAY_XRES 128
#define DOTMATRIX_DISPLAY_YRES 64
#define DOTMATRIX_DISPLAY_LINES (DOTMATRIX_DISPLAY_YRES / 8)

/**
 * commands
 */

/**
 * Command Start Byte:
 * 11xB AxCC
 *
 * B: brightness bit, if set change brightness of LCD backlight
 * A, C: A bit sets instruction mode/ data mode; other bits the according CS signals (CS1/ CS2)
 */
#define COMMAND_START 0xC0
#define COMMAND_BRIGHTNESS 0x10

#define COMMAND_START_CS1 0x01
#define COMMAND_START_CS2 0x02
#define COMMAND_START_A0 0x08

#define ADDRESS_PINS_MASK 0x0B


#define COMMAND_DISPLAYDRIVER_ON 0xAF
#define COMMAND_DISPLAYDRIVER_OFF 0xAE

#define COMMAND_STATICDRIVE_ON 0xA5
#define COMMAND_STATICDRIVE_OFF 0xA4

#define COMMAND_RESET 0xE2

#define COMMAND_SETPAGE 0xB8
#define COMMAND_PAGE0 0x00
#define COMMAND_PAGE1 0x01
#define COMMAND_PAGE2 0x02
#define COMMAND_PAGE3 0x03

#define COMMAND_SETLINE 0xC0 /*Line 0: 0xC0, Line 1: 0xC1 ... Line 31: 0xDF*/

typedef struct DisplayElement_t {
	uint8_t pos_x;
	uint8_t pos_y;
	uint8_t len_x;
	uint8_t height_y;
	uint8_t * buffer;
	uint16_t status;
} DisplayElement;

extern int8_t DotMatrix_size;
extern DisplayElement DotMatrix_mem[DOTMATRIX_MEMSIZE];

/**
 * resets the display, initializes the dot matrix display function
 * (must be called prior any other dot matrix function call)
 */
void DotMatrix_initDisplay();

/**
 * initializes a new Display Element with the given positions and sizes
 * the position defines the upper left corner
 * the size defines the size, or, the lower right corner along with the position
 * @param xpos: the x position of the element in pixel
 * @param ypos: the y position of the element in pixel
 * @param xsize: the x size of the element in pixel
 * @param ysize: the y size of the element in pixel
 * @return: the initialized DisplayElement structure
 * 	which contains a 2dimensional buffer of size length (buffer[xsize][ysize])
 */
DisplayElement* DotMatrix_newDisplayElement(uint8_t xpos, uint8_t ypos, uint8_t xsize, uint8_t ysize);

/**
 * activates the element for update on the display
 *
 */
int8_t DotMatrix_setElementChanged(DisplayElement* delm);

/**
 * changes display data according to data till the end of the element or datalen is reached
 * position is relative to the display element, so 0,0 describes the top left corner of the element
 * data leaking out of boundary is discarded
 *
 * @return: -1 if changes were not possible (dimensions out of bounds), 1 changes successful
 */
int8_t DotMatrix_changeElementN(DisplayElement* delm, uint8_t xpos, uint8_t ypos, uint8_t * data, uint8_t datalen);

/**
 * changes one row of display data
 * position is relative to the display element, so 0,0 describes the top left corner of the element
 * data leaking out of boundary is discarded
 *
 * @return: -1 if changes were not possible (dimensions out of bounds), 1 changes successful
 */
int8_t DotMatrix_changeElement(DisplayElement* delm, uint8_t xpos, uint8_t ypos, uint8_t data);



#endif /* DOTMATRIXDISPLAY_H_ */
