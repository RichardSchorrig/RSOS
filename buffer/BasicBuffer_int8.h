/*
 * BasicBuffer_int8.h
 *
 *  Created on: 02.05.2017
 *      Author: Richard
 */

#ifndef BUFFER_BASICBUFFER_INT8_H_
#define BUFFER_BASICBUFFER_INT8_H_

#include "BasicBuffer.h"
#include "Buffer_int8.h"
#include "BufferBuffer_int8.h"

/* exclude everything if not used */
#ifdef MAXBUFFER_VOID

static inline int8_t BasicBuffer_uint8_get(Buffer_void* buffer, volatile uint8_t* destination) __attribute__((always_inline));
static inline int8_t BasicBuffer_uint8_get(Buffer_void* buffer, volatile uint8_t* destination)
{
    if (buffer->data.type & BUFFER_TYPE_BUFFERBUFFER)
    {
        return BufferBuffer_uint8_get((BufferBuffer_uint8*) buffer, destination);
    }
    else
    {
        return Buffer_uint8_get((Buffer_uint8*) buffer, destination);
    }
}

static inline int8_t BasicBuffer_int8_get(Buffer_void* buffer, volatile uint8_t* destination) __attribute__((always_inline));
static inline int8_t BasicBuffer_int8_get(Buffer_void* buffer, volatile uint8_t* destination)
{
    return BasicBuffer_uint8_get(buffer, (volatile uint8_t*) destination);
}

static inline int8_t BasicBuffer_uint8_set(Buffer_void* buffer, const volatile uint8_t* source) __attribute__((always_inline));
static inline int8_t BasicBuffer_uint8_set(Buffer_void* buffer, const volatile uint8_t* source)
{
    if (buffer->data.type & BUFFER_TYPE_BUFFERBUFFER)
    {
        return BufferBuffer_uint8_set((BufferBuffer_uint8*) buffer, source);
    }
    else
    {
        return Buffer_uint8_set((Buffer_uint8*) buffer, source);
    }
}

static inline int8_t BasicBuffer_int8_set(Buffer_void* buffer, const volatile uint8_t* source) __attribute__((always_inline));
static inline int8_t BasicBuffer_int8_set(Buffer_void* buffer, const volatile uint8_t* source)
{
    return BasicBuffer_uint8_get(buffer, (const volatile uint8_t*) source);
}

static inline int8_t BasicBuffer_increment_index_put(Buffer_uint8* buffer) __attribute__((always_inline));
static inline int8_t BasicBuffer_increment_index_put(Buffer_uint8* buffer)
{
    if (buffer->data.type & BUFFER_TYPE_BUFFERBUFFER)
    {
        return BufferBuffer_increment_index_put((BufferBuffer_uint8*) buffer);
    }
    else
    {
        return Buffer_increment_index_put((Buffer_uint8*) buffer);
    }
}

static inline int8_t BasicBuffer_increment_index_pop(Buffer_uint8* buffer) __attribute__((always_inline));
static inline int8_t BasicBuffer_increment_index_pop(Buffer_uint8* buffer)
{
    if (buffer->data.type & BUFFER_TYPE_BUFFERBUFFER)
    {
        return BufferBuffer_increment_index_pop((BufferBuffer_uint8*) buffer);
    }
    else
    {
        return Buffer_increment_index_pop((Buffer_uint8*) buffer);
    }
}

#endif /* MAXBUFFER_VOID */
#endif /* BUFFER_BASICBUFFER_INT8_H_ */
