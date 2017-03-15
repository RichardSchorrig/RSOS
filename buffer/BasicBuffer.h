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
static inline Buffer_void* initBuffer(void* data, uint8_t length);
static Buffer_void* initBuffer(void* data, uint8_t length) {
    buffer_mem[buffer_size].buffer = data;
    buffer_mem[buffer_size].size.size = length;
    buffer_mem[buffer_size].size.readBytes = 0;
    buffer_size += 1;
    return &buffer_mem[buffer_size - 1];
}


/**
 * set a new memory location to the buffer, along with the size (of total elements storable)
 * @param buffer the buffer to set the new memory to
 * @param length the maximum number of elements to store
 */
static inline void setBuffer(Buffer_void* buffer, void* data, uint8_t length);
static void setBuffer(Buffer_void* buffer, void* data, uint8_t length) {
    buffer->buffer = data;
    buffer->size.size = length;
    buffer->size.readBytes = 0;
}

/**
 * set a new length to an existing buffer
 * (only useful to read /write a certain amount of bytes, or if the buffer belongs to a bufferbuffer)
 * @param buffer the buffer to set the new length to
 * @param length the new length which should be smaller than the allocated memory
 */
static inline void setBufferLength(Buffer_void* buffer, uint8_t length);
static void setBufferLength(Buffer_void* buffer, uint8_t length) {
    buffer->size.size = length;
    buffer->size.readBytes = 0;
}

/**
 * resets the buffer's position to 0
 * @param buffer the buffer to reset
 */
static inline void resetBuffer(Buffer_void* buffer);
static void resetBuffer(Buffer_void* buffer) {
    buffer->size.readBytes = 0;
}


/**
 * resets the number of read bytes to 0
 * @param buffer the buffer to reset

static inline void reset_uint8(Buffer_uint8* buffer);
static void reset_uint8(Buffer_uint8* buffer) {
    buffer->size.readBytes = 0;
}
*/
/**
 * resets a bufferbuffer

static inline void reset_bufferbuffer_uint8(BufferBuffer_uint8* buffer);
static void reset_bufferbuffer_uint8(BufferBuffer_uint8* buffer) {

}
*/
#endif /* MAXBUFFER_VOID */
#endif /* BUFFER_BASICBUFFER_H_ */
