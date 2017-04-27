/*
 * BasicBuffer.h
 *
 *  Created on: 11.02.2017
 *      Author: Richard
 */

#ifndef BUFFER_BASICBUFFER_H_
#define BUFFER_BASICBUFFER_H_

#include <RSOSDefines.h>

/* exclude everything if not used */
#ifdef MAXBUFFER_VOID

#include <stdint.h>

typedef struct BufferSize_t {
    uint8_t size;
    uint8_t readBytes;
} BufferSize;

typedef struct Buffer_void_t {
    void * buffer;
    BufferSize size;
} Buffer_void;

extern Buffer_void buffer_mem[MAXBUFFER_VOID];
extern int8_t buffer_size;

#define getBuffer_void(n) (&buffer_mem[n])

/**
 * initialize a buffer containing the pointer data
 * the buffer size is set to length, the read bytes is set to 0
 *
 * @param data: a pointer to a data array. for full functionality,
 * the returned buffer must be casted to the corresponding buffer type.
 * For example, the buffer containing data of type uint8_t must be cast to Buffer_uint8
 * @param length: the length of the buffer, depending on the data type, i.e. max number of elements in the buffer
 *
 * @return the initialized buffer
 */
Buffer_void* initBuffer(void* data, uint8_t length);

/**
 * set a new memory location to the buffer, along with the size (of total elements storable)
 * @param buffer the buffer to set the new memory to
 * @param length the maximum number of elements to store
 */
void setBuffer(Buffer_void* buffer, void* data, uint8_t length);

/**
 * set a new length to an existing buffer
 * (only useful to read /write a certain amount of bytes, or if the buffer belongs to a bufferbuffer)
 * @param buffer the buffer to set the new length to
 * @param length the new length which should be smaller than the allocated memory
 */
void setBufferLength(Buffer_void* buffer, uint8_t length);


/**
 * resets the buffer's position to 0
 * @param buffer the buffer to reset
 */
static inline void resetBuffer(Buffer_void* buffer) __attribute__((always_inline));
static inline void resetBuffer(Buffer_void* buffer) {
    buffer->size.readBytes = 0;
}

/**
 * returns the position of the buffer inside the buffer_mem array
 * @param buffer the buffer
 * @return the position of the buffer, -1 in case an invalid pointer was given
 */
int8_t BasicBuffer_getNumber(Buffer_void* buffer);

#endif /* MAXBUFFER_VOID */
#endif /* BUFFER_BASICBUFFER_H_ */
