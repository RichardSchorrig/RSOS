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

/* exclude everything if not used */
#ifdef MAXBUFFER_VOID

/**
 * structure of the int8_t buffer
 * takes up 4 Byte in memory
 */
typedef struct Buffer_int8_t {
    int8_t * buffer;
    BufferData data;
    BufferIndex index;
} Buffer_int8;

#define getBuffer_int8(n) ((Buffer_int8*)&buffer_mem[n])

/**
 * structure of the uint8_t buffer
 * takes up 4 Byte in memory
 */
typedef struct Buffer_uint8_t {
    uint8_t * buffer;
    BufferData data;
    BufferIndex index;
} Buffer_uint8;

#define getBuffer_uint8(n) ((Buffer_uint8*)&buffer_mem[n])

/**
 * reads a byte from the buffer from the current position without incrementing to the next position
 * and puts it to destination
 * @param buffer the buffer to read from
 * @param destination the destination to write the read byte
 * @return the remaining read positions in the buffer
 */
static inline int8_t Buffer_uint8_get(Buffer_uint8* buffer, volatile uint8_t* destination) __attribute__((always_inline));
static inline int8_t Buffer_uint8_get(Buffer_uint8* buffer, volatile uint8_t* destination)
{
    if (buffer->index.index_pop >= buffer->data.size)
    {
        return -1;
    }
    else
    {
        *destination = buffer->buffer[buffer->index.index_pop];
        return buffer->data.size - buffer->index.index_pop;
    }
}

/**
 * reads a byte from the buffer from the current position without incrementing to the next position
 * and puts it to destination
 * @param buffer the buffer to read from
 * @param destination the destination to write the read byte
 * @return the remaining read positions in the buffer
 */
static inline int8_t Buffer_int8_get(Buffer_int8* buffer, volatile int8_t* destination) __attribute__((always_inline));
static inline int8_t Buffer_int8_get(Buffer_int8* buffer, volatile int8_t* destination)
{
    return Buffer_uint8_get((Buffer_uint8*) buffer, (volatile uint8_t*) destination);
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte to
 * @param source the source of the byte
 * @return the remaining write positions in the buffer
 */
static inline int8_t Buffer_uint8_set(Buffer_uint8* buffer, const volatile uint8_t* source) __attribute__((always_inline));
static inline int8_t Buffer_uint8_set(Buffer_uint8* buffer, const volatile uint8_t* source)
{
    if (buffer->index.index_put >= buffer->data.size)
    {
        return -1;
    }
    else
    {
        buffer->buffer[buffer->index.index_put] = *source;
        return buffer->data.size - buffer->index.index_put;
    }
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte to
 * @param source the source of the byte
 * @return the remaining write positions in the buffer
 */
static inline int8_t Buffer_int8_set(Buffer_int8* buffer, const volatile int8_t* source) __attribute__((always_inline));
static inline int8_t Buffer_int8_set(Buffer_int8* buffer, const volatile int8_t* source)
{
    return Buffer_uint8_set((Buffer_uint8*) buffer, (const volatile uint8_t*) source);
}

/**
 * increments the buffer write position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining values after incrementing (0: no values available)
 */
static inline int8_t Buffer_uint8_increment_index_put(Buffer_uint8* buffer) __attribute__((always_inline));
static inline int8_t Buffer_uint8_increment_index_put(Buffer_uint8* buffer)
{
    if (buffer->index.index_put >= buffer->data.size)
    {
        return -1;
    }

    buffer->index.index_put += 1;

    if (buffer->data.type & BUFFER_TYPE_RING)
    {
        if (buffer->index.index_put == buffer->data.size)
        {
            buffer->index.index_put = 0;
        }
    }

    return buffer->data.size - buffer->index.index_put;
}

/**
 * increments the buffer write position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining values after incrementing (0: no values available)
 */
static inline int8_t Buffer_int8_increment_index_put(Buffer_int8* buffer) __attribute__((always_inline));
static inline int8_t Buffer_int8_increment_index_put(Buffer_int8* buffer)
{
    return Buffer_uint8_increment_index_put((Buffer_uint8*) buffer);
}

/**
 * increments the buffer read position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining values after incrementing (0: no values available)
 */
static inline int8_t Buffer_uint8_increment_index_pop(Buffer_uint8* buffer) __attribute__((always_inline));
static inline int8_t Buffer_uint8_increment_index_pop(Buffer_uint8* buffer)
{
    if (buffer->index.index_pop >= buffer->data.size)
    {
        return -1;
    }

    buffer->index.index_pop += 1;

    if (buffer->data.type & BUFFER_TYPE_RING)
    {
        if (buffer->index.index_pop == buffer->data.size)
        {
            buffer->index.index_pop = 0;
        }
    }

    return buffer->data.size - buffer->index.index_pop;
}

/**
 * increments the buffer read position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining values after incrementing (0: no values available)
 */
static inline int8_t Buffer_int8_increment_index_pop(Buffer_int8* buffer) __attribute__((always_inline));
static inline int8_t Buffer_int8_increment_index_pop(Buffer_int8* buffer)
{
    return Buffer_uint8_increment_index_pop((Buffer_uint8*) buffer);
}

#endif /* MAXBUFFER_VOID */
#endif /* BUFFER_BUFFER_INT8_H_ */
