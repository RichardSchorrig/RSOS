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

/* exclude everything if not used */
#ifdef MAXBUFFER_VOID

/**
 * structure of the buffer_int8 buffer
 * takes up 4 byte in memory
 */
typedef struct BufferBuffer_int8_t {
    Buffer_int8** buffer;
    BufferData data;
    BufferIndex index;
} BufferBuffer_int8;

#define getBufferBuffer_int8(n) ((BufferBuffer_int8*)&buffer_mem[n])

/**
 * structure of the buffer_uint8 buffer
 * takes up 4 byte in memory
 */
typedef struct BufferBuffer_uint8_t {
    Buffer_uint8** buffer;
    BufferData data;
    BufferIndex index;
} BufferBuffer_uint8;

#define getBufferBuffer_uint8(n) ((BufferBuffer_uint8*)&buffer_mem[n])

/**
 * reads a byte from the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to read from
 * @param destination the destination to write the read byte
 */
static inline int8_t BufferBuffer_uint8_get(BufferBuffer_uint8* buffer, volatile uint8_t* destination) __attribute__((always_inline));
static inline int8_t BufferBuffer_uint8_get(BufferBuffer_uint8* buffer, volatile uint8_t* destination)
{
    if (buffer->index.index_pop >= buffer->data.size)
    {
        return -1;
    }
    else
    {
        return Buffer_uint8_get(buffer->buffer[buffer->index.index_pop], destination);
    }
}

/**
 * reads a byte from the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to read from
 * @param destination the destination to write the read byte
 */
static inline int8_t BufferBuffer_int8_get(BufferBuffer_int8* buffer, volatile int8_t* destination) __attribute__((always_inline));
static inline int8_t BufferBuffer_int8_get(BufferBuffer_int8* buffer, volatile int8_t* destination)
{
    return BufferBuffer_uint8_get((BufferBuffer_uint8*) buffer, (volatile uint8_t*) destination);
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte
 * @param source the source of the byte
 */
static inline int8_t BufferBuffer_uint8_set(BufferBuffer_uint8* buffer, const volatile uint8_t* source) __attribute__((always_inline));
static inline int8_t BufferBuffer_uint8_set(BufferBuffer_uint8* buffer, const volatile uint8_t* source)
{
    if (buffer->index.index_put >= buffer->data.size)
    {
        return -1;
    }
    else
    {
        return Buffer_uint8_set(buffer->buffer[buffer->index.index_put], source);
    }
}

/**
 * put a byte to the buffer at the current position without incrementing to the next position
 * @param buffer the buffer to put the byte
 * @param source the source of the byte
 */
static inline int8_t BufferBuffer_int8_set(BufferBuffer_int8* buffer, const volatile int8_t* source) __attribute__((always_inline));
static inline int8_t BufferBuffer_int8_set(BufferBuffer_int8* buffer, const volatile int8_t* source)
{
    return BufferBuffer_uint8_set((BufferBuffer_uint8*) buffer, (const volatile uint8_t*) source);
}

/**
 * increments the buffer write position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining buffers after incrementing (0: no buffers available, -1: index out of bounds)
 */
static inline int8_t BufferBuffer_uint8_increment_index_put(BufferBuffer_uint8* buffer) __attribute__((always_inline));
static inline int8_t BufferBuffer_uint8_increment_index_put(BufferBuffer_uint8* buffer)
{
    if (buffer->index.index_put >= buffer->data.size)
    {
        return -1;
    }

    if (Buffer_uint8_increment_index_put(buffer->buffer[buffer->index.index_put]) == 0)
    {
        buffer->index.index_put += 1;
    }

    if (buffer->data.type & BUFFER_TYPE_RING)
    {
        if (buffer->index.index_put >= buffer->data.size)
        {
            buffer->index.index_put = 0;
        }
    }

    return buffer->data.size - buffer->index.index_put;
}

/**
 * increments the buffer write position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining buffers after incrementing (0: no buffers available, -1: index out of bounds)
 */
static inline int8_t BufferBuffer_int8_increment_index_put(BufferBuffer_int8* buffer) __attribute__((always_inline));
static inline int8_t BufferBuffer_int8_increment_index_put(BufferBuffer_int8* buffer)
{
    return BufferBuffer_uint8_increment_index_put((BufferBuffer_uint8*) buffer);
}

/**
 * increments the buffer read position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining buffers after incrementing (0: no buffers available, -1: index out of bounds)
 */
static inline int8_t BufferBuffer_uint8_increment_index_pop(BufferBuffer_uint8* buffer) __attribute__((always_inline));
static inline int8_t BufferBuffer_uint8_increment_index_pop(BufferBuffer_uint8* buffer)
{
    if (buffer->index.index_pop >= buffer->data.size)
    {
        return -1;
    }

    if (Buffer_uint8_increment_index_pop(buffer->buffer[buffer->index.index_pop]) == 0)
    {
        buffer->index.index_pop += 1;
    }

    if (buffer->data.type & BUFFER_TYPE_RING)
    {
        if (buffer->index.index_pop >= buffer->data.size)
        {
            buffer->index.index_pop = 0;
        }
    }

    return buffer->data.size - buffer->index.index_pop;
}

/**
 * increments the buffer read position without reading or writing
 * @param buffer the buffer to increment
 * @return the remaining buffers after incrementing (0: no buffers available, -1: index out of bounds)
 */
static inline int8_t BufferBuffer_int8_increment_index_pop(BufferBuffer_int8* buffer) __attribute__((always_inline));
static inline int8_t BufferBuffer_int8_increment_index_pop(BufferBuffer_int8* buffer)
{
    return BufferBuffer_uint8_increment_index_pop((BufferBuffer_uint8*) buffer);
}

#endif /* MAXBUFFER_VOID */
#endif /* BUFFER_BUFFERBUFFER_INT8_H_ */
