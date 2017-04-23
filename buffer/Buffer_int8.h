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
static inline void Buffer_uint8_get(Buffer_uint8* buffer, volatile uint8_t* destination) __attribute__((always_inline));
static inline void Buffer_uint8_get(Buffer_uint8* buffer, volatile uint8_t* destination)
{
    *destination = buffer->buffer[buffer->size.readBytes];
}

/**
 * reads a byte from the buffer from the current position without incrementing to the next position
 * @param buffer the buffer to read from
 * @param destination the destination to write the read byte
 */
static inline void Buffer_int8_get(Buffer_int8* buffer, volatile int8_t* destination) __attribute__((always_inline));
static inline void Buffer_int8_get(Buffer_int8* buffer, volatile int8_t* destination)
{
    *destination = buffer->buffer[buffer->size.readBytes];
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte to
 * @param source the source of the byte
 */
static inline void Buffer_uint8_set(Buffer_uint8* buffer, volatile uint8_t* source) __attribute__((always_inline));
static inline void Buffer_uint8_set(Buffer_uint8* buffer, volatile uint8_t* source)
{
    buffer->buffer[buffer->size.readBytes] = *source;
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte to
 * @param source the source of the byte
 */
static inline void Buffer_int8_set(Buffer_int8* buffer, volatile int8_t* source) __attribute__((always_inline));
static inline void Buffer_int8_set(Buffer_int8* buffer, volatile int8_t* source)
{
    buffer->buffer[buffer->size.readBytes] = *source;
}

/**
 * increments the buffer position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining values after incrementing (0: no values available)
 */
static inline int8_t Buffer_uint8_increment(Buffer_uint8* buffer) __attribute__((always_inline));
static inline int8_t Buffer_uint8_increment(Buffer_uint8* buffer)
{
    if (buffer->size.readBytes >= buffer->size.size)
    {
        return -1;
    }
    buffer->size.readBytes += 1;
    return buffer->size.size - buffer->size.readBytes;
}

/**
 * increments the buffer position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining values after incrementing (0: no values available)
 */
static inline int8_t Buffer_int8_increment(Buffer_int8* buffer) __attribute__((always_inline));
static inline int8_t Buffer_int8_increment(Buffer_int8* buffer)
{
    return Buffer_uint8_increment((Buffer_uint8*) buffer);
}

/**
 * read and set a byte to the next buffer position, position is incremented
 * @param buffer the buffer to write to / read from
 * @param source the byte to be written to the buffer, can be 0
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return the number of bytes available or -1 if the buffer is full (no operation done)
 */
static inline int8_t Buffer_uint8_set_getNext(Buffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination) __attribute__((always_inline));
static inline int8_t Buffer_uint8_set_getNext(Buffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination)
{

    if (buffer->size.readBytes >= buffer->size.size)
    {
        return -1;
    }

    if (destination != 0) {
        *destination = buffer->buffer[buffer->size.readBytes];
    }

    if (source != 0) {
        buffer->buffer[buffer->size.readBytes] = *source;
    }

    buffer->size.readBytes++;

    return buffer->size.size - buffer->size.readBytes;
}

/**
 * read and set a byte to the next buffer position, position is incremented
 * @param buffer the buffer to write to / read from
 * @param source the byte to be written to the buffer, can be 0
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return the number of bytes available or -1 if the buffer is full (no operation done)
 */
static inline int8_t Buffer_int8_set_getNext(Buffer_int8* buffer, volatile int8_t* source, volatile int8_t* destination) __attribute__((always_inline));
static inline int8_t Buffer_int8_set_getNext(Buffer_int8* buffer, volatile int8_t* source, volatile int8_t* destination)
{
    return (Buffer_uint8_set_getNext((Buffer_uint8*) buffer, (volatile uint8_t *) source, (volatile uint8_t *) destination));
}

/**
 * reads a byte from the next buffer position, position is incremented
 * @param buffer the buffer to write to / read from
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return 1 if read / write is successful, -1 if no read / write operation could be done
 */
static inline int8_t Buffer_uint8_getNext(Buffer_uint8* buffer, volatile uint8_t* destination)  __attribute__((always_inline));
static inline int8_t Buffer_uint8_getNext(Buffer_uint8* buffer, volatile uint8_t* destination) {
    return Buffer_uint8_set_getNext(buffer, 0, destination);
}

/**
 * reads a byte from the next buffer position, position is incremented
 * @param buffer the buffer to write to / read from
 * @param destination the destination for the byte read from the buffer, can be 0
 * @return 1 if read / write is successful, -1 if no read / write operation could be done
 */
static inline int8_t Buffer_int8_getNext(Buffer_int8* buffer, volatile int8_t* destination)  __attribute__((always_inline));
static inline int8_t Buffer_int8_getNext(Buffer_int8* buffer, volatile int8_t* destination) {
    return Buffer_int8_set_getNext(buffer, 0, destination);
}


#endif /* BUFFER_BUFFER_INT8_H_ */
