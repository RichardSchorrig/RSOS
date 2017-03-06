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

typedef struct Buffer_int8_t {
    int8_t * buffer;
    BufferSize size;
} Buffer_int8;

typedef struct Buffer_uint8_t {
    uint8_t * buffer;
    BufferSize size;
} Buffer_uint8;

typedef struct BufferBuffer_uint8_t {
    Buffer_uint8** buffer;
    BufferSize size;
} BufferBuffer_uint8;

extern Buffer_void buffer_mem[MAXBUFFER_VOID];
extern int8_t buffer_size;

/**
 * initialize a buffer containing the pointer data
 * the buffer size is set to length, the read bytes is set to 0
 *
 * @param data: a pointer to a data array. for full functionality,
 * the returned buffer must be casted to the corresponding buffer type.
 * For example, the buffer containing data of type uint8_t must be cast to Buffer_uint8
 * @param length: the length of the buffer, depending on the data type
 *
 * @return the initialized buffer
 */
Buffer_void* initBuffer(void* data, uint8_t length);

void setBuffer(Buffer_void* buffer, void* data, uint8_t length);
void setBufferLength(Buffer_void* buffer, uint8_t length);

void resetBuffer(Buffer_void* buffer);

void get_uint8(Buffer_uint8* buffer, volatile uint8_t* destination);
int8_t set_getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination);
int8_t getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* destination);
void set_uint8(Buffer_uint8* buffer, volatile uint8_t* source);
void reset_uint8(Buffer_uint8* buffer);

void get_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination);
int8_t getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination);
int8_t set_getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination);
void set_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source);
void reset_bufferbuffer_uint8(BufferBuffer_uint8* buffer);

#endif /* MAXBUFFER_VOID */
#endif /* BUFFER_BASICBUFFER_H_ */
