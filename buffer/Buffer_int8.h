/*
 * Buffer_int8.h
 *
 * a buffer for int8 / uint8 data
 *
 *  Created on: 14.03.2017
 *      Author: Richard
 */

#ifndef BUFFER_BUFFER_INT8_H_
#define BUFFER_BUFFER_INT8_H_

#include "BasicBuffer.h"

/**
 * structure of the int8_t buffer
 * takes up 4 Byte in memory
 */
typedef struct Buffer_int8_t {
    int8_t * buffer;
    BufferSize size;
} Buffer_int8;

/**
 * structure of the uint8_t buffer
 * takes up 4 Byte in memory
 */
typedef struct Buffer_uint8_t {
    uint8_t * buffer;
    BufferSize size;
} Buffer_uint8;


/**
 * reads a byte from the buffer from the current position without incrementing to the next position
 * @param buffer the buffer to read from
 * @param destination the destination to write the read byte
 */
static inline void get_uint8(Buffer_uint8* buffer, volatile uint8_t* destination);
static void get_uint8(Buffer_uint8* buffer, volatile uint8_t* destination) {
    *destination = buffer->buffer[buffer->size.readBytes];
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte to
 * @param source the source of the byte
 */
static inline void set_uint8(Buffer_uint8* buffer, volatile uint8_t* source);
static void set_uint8(Buffer_uint8* buffer, volatile uint8_t* source) {
    buffer->buffer[buffer->size.readBytes] = *source;
}

/**
 * read and set a byte to the next buffer position, position is incremented
 * @param buffer the buffer to write to / read from
 * @param source the byte to be written to the buffer, can be 0
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return the number of bytes available or -1 if the buffer is full (no operation done)
 */
static inline int8_t set_getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination);
static int8_t set_getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination) {

    if (buffer->size.readBytes >= buffer->size.size)
    {
        return -1;
    }
/*
    if (source != 0 && buffer->size.readBytes > 0) {
        buffer->buffer[buffer->size.readBytes-1] = *source;
    }
*/
    if (destination != 0) {
        *destination = buffer->buffer[buffer->size.readBytes];
    }

    if (source != 0) {
        buffer->buffer[buffer->size.readBytes-1] = *source;
    }

    buffer->size.readBytes++;
/*
    if (buffer->size.readBytes >= buffer->size.size) {
//        buffer->size.readBytes = 0;
        return -1;
    }
*/
    return buffer->size.size - buffer->size.readBytes;
}

/**
 * reads a byte from the next buffer position, position is incremented
 * @param buffer the buffer to write to / read from
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return 1 if read / write is successful, -1 if no read / write operation could be done
 */
static inline int8_t getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* destination);
static int8_t getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* destination) {
    return set_getNext_uint8(buffer, 0, destination);
}


#endif /* BUFFER_BUFFER_INT8_H_ */
