/*
 * BufferBuffer_int8.h
 *
 * a buffer storing int8 / uint8 buffer
 * ->Buffer buffer
 *
 * A buffer buffer contains an array of buffers.
 * when storing / reading a byte, the first buffer is read.
 * when the first buffer reaches it's end, the next one is read /written to and so on
 *
 *  Created on: 14.03.2017
 *      Author: Richard
 */

#ifndef BUFFER_BUFFERBUFFER_INT8_H_
#define BUFFER_BUFFERBUFFER_INT8_H_

#include "BasicBuffer.h"
#include "Buffer_int8.h"

/**
 * structure of the buffer_int8 buffer
 * takes up 4 byte in memory
 */
typedef struct BufferBuffer_int8_t {
    Buffer_int8** buffer;
    BufferSize size;
} BufferBuffer_int8;

/**
 * structure of the buffer_uint8 buffer
 * takes up 4 byte in memory
 */
typedef struct BufferBuffer_uint8_t {
    Buffer_uint8** buffer;
    BufferSize size;
} BufferBuffer_uint8;

/**
 * reads a byte from the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to read from
 * @param destination the destination to write the read byte
 */
static inline void get_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination);
static void get_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination) {
    get_uint8(buffer->buffer[buffer->size.readBytes], destination);
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte
 * @param source the source of the byte
 */
static inline void set_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source);
static void set_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source) {
    set_uint8(buffer->buffer[buffer->size.readBytes], source);
}

/**
 * read and set a byte to the next buffer position, position is incremented
 * if a buffer is full, the next one in the bufferbuffer is written to.
 * if no buffer is available, nothing is read / written
 * @param buffer the buffer to write to / read from
 * @param source the byte to be written to the buffer, can be 0
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return 1 if read / write is successful, -1 if no read / write operation could be done
 */
static inline int8_t set_getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination);
static int8_t set_getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination) {
    if (buffer->size.readBytes >= buffer->size.size) {
        return -1;
    }
    if (set_getNext_uint8(buffer->buffer[buffer->size.readBytes], source, destination) == 0) {
        buffer->size.readBytes++;
    }
    return buffer->size.size - buffer->size.readBytes;
}

/**
 * reads a byte from the next buffer position, position is incremented
 * @param buffer the buffer to write to / read from
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return 1 if read / write is successful, -1 if no read / write operation could be done
 */
static inline int8_t getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination);
static int8_t getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination) {
    return set_getNext_bufferbuffer_uint8(buffer, 0, destination);
}



#endif /* BUFFER_BUFFERBUFFER_INT8_H_ */
