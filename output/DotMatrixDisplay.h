/*
 * DotMatrixDisplay.h
 *
 *  Created on: 02.10.2016
 *      Author: Richard
 */

#ifndef DOTMATRIXDISPLAY_H_
#define DOTMATRIXDISPLAY_H_

#include <stdint.h>
#include <RSOSDefines.h>

/* exclude everything if not used */
#ifdef DOTMATRIX_MEMSIZE

#include "../Task.h"
#include "../buffer/BasicBuffer.h"

/**
 * sizes
 */
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

/**
 * Display Element structure
 * Fields:
 *  pos_x, pos_y: position of the upper left corner in display
 *  len_x: the length of the elemnt in x direction
 *  height_y: the height of the element, in negative y direction
 *  status: bit field containing:
 *      AIxx xxxx LLLL MMMM
 *      A: active bit
 *      I: bit if this element should be displayed inverted
 *      L: the line corresponding to the y position. In case the height of the element interfere
 *         with more than one line, this is the first line
 *      M: additional lines, i.e. the other lines if element extends over more lines
 *         example: LLLL = 4, MMMM = 3 -> the element extends over line 4 through 7
 */
typedef struct DisplayElement_t {
	uint8_t pos_x;
	uint8_t pos_y;
	uint8_t len_x;
	uint8_t height_y;
//	uint8_t * buffer;
	uint16_t status;
} DisplayElement;

extern int8_t dotMatrix_size;
extern DisplayElement dotMatrix_mem[DOTMATRIX_MEMSIZE];

/**
 * resets the display, initializes the dot matrix display function
 * (must be called prior any other dot matrix function call)
 * to operate, the following structures must be available:
 *      1x Task
 *      1x ShiftRegisterOperation
 *      3x Buffer_void
 *      1x DisplayElement
 * inits a shift register operation structure. make sure that one structure is available
 * also, you must first call SR_initOperation() to ensure operation
 * todo: add header files for I2C operation, SPI operation is not optimal
 */
void DotMatrix_initDisplay(volatile uint8_t * port, uint8_t pin);

/**
 * task function: checks all display elements for active bit
 * if an active element is found, it cycles until all lines of the element are transferred
 * it also checks the next element if it is active. If the next active element refers to the same
 * display line and is not too far apart (10 positions) it will be added to the transfer
 */
void DotMatrix_transferElement();

/**
 * initializes a new Display Element with the given positions and sizes
 * the position defines the upper left corner
 * the size defines the size, or, the lower right corner along with the position
 *
 * to make display operation more efficient, you should init the display elements in the order
 * of their position on the display
 *
 * @param xpos: the x position of the element on the screen in pixel
 * @param ypos: the y position of the element on the screen in pixel
 * @param xsize: the x size of the element in pixel
 * @param ysize: the y size of the element in pixel
 * @param isInverted: displays the element inverted (can be used for list selections)
 *                    0: not inverted, 1: invert element
 * @return: the initialized DisplayElement structure
 */
DisplayElement* DotMatrix_newDisplayElement(uint8_t xpos, uint8_t ypos, uint8_t xsize, uint8_t ysize, uint8_t showInverted);

/**
 * cleans the entire display
 */
int8_t DotMatrix_CleanDisplay();

/**
 * cleans the entire display element delm, i.e. sets all pixel to 0 (even when element is inverted)
 * @return 1 on success, -1 on failure
 */
int16_t DotMatrix_cleanElement(DisplayElement* delm);

int8_t DotMatrix_scroll(uint8_t line);

/**
 * changes display data according to data till the end of the element or datalen is reached
 * position is relative to the display element, so 0,0 describes the top left corner of the element
 * data leaking out of boundary is discarded
 *
 * @param delm: the display element to change
 * @param xpos: the x position inside the display element, at which point it is to change. can be negative, data will be cut to fit
 * @param ypos: the y position inside the display element, at which point it is to change. can be negative
 * @param data: the data buffer to copy to the display element's memory
 * @param datalen: the maximum number of bytes to read of data
 *
 * @return: -1 if changes were not possible (dimensions out of bounds) or element is currently transferred, 0 if no changes where made,
 *          or the number of changed columns
 */
int8_t DotMatrix_changeElementN(DisplayElement* delm, int16_t xpos, int16_t ypos, const uint8_t * data, uint8_t datalen);

/**
 * changes one column of display data
 * position is relative to the display element, so 0,0 describes the top left corner of the element
 * data leaking out of boundary is discarded
 *
 * @param delm: the display element to change
 * @param xpos: the x position inside the display element, at which point it is to change. can be negative, data will be cut to fit
 * @param ypos: the y position inside the display element, at which point it is to change. can be negative
 * @param data: a data byte to copy to the display element's memory
 *
 * @return: -1 if changes were not possible (dimensions out of bounds) or element is currently transferred, 0 if no changes where made,
 *          or 1 if change was successful
 */
int8_t DotMatrix_changeElement(DisplayElement* delm, int16_t xpos, int16_t ypos, const uint8_t data);

/**
 * activates the element for update on the display. the content corresponding to the position of the
 * Display Element is transferred to the display.
 * @param delm: the display element to be activated
 */
void DotMatrix_activateElement(DisplayElement* delm);


#endif /* DOTMATRIX_MEMSIZE */
#endif /* DOTMATRIXDISPLAY_H_ */
