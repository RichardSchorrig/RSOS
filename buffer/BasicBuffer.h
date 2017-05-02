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

/**
 * regular buffer, supports push and pop operations
 * with two different indexes.
 * push (set) increment index_put,
 * pop (get) increment index_pop
 * if index reaches the end of buffer, nothing is pushed or popped,
 * the associated function will return -1
 */
#define BUFFER_TYPE_REGULAR (0x00)

/**
 * Buffer buffer, supports push and pop operations.
 * the difference to the regular buffer is that the linked buffer
 * itself contains more buffers. this can be used to connect different buffers
 * to one signle buffer without moving the data.
 * the set and get functions call the functions with the active buffer. if the buffer's
 * end is reached, the associated index is incremented, and the next buffer becomes active
 * if index reaches the end of buffer, nothing is pushed or popped,
 * the associated function will return -1
 */
#define BUFFER_TYPE_BUFFERBUFFER (0x01<<0)

/**
 * Ring buffer, supports push and pop operations
 * A ring buffer sets the index to zero if the end of the buffer is reached.
 * Can be combined with the type bufferbuffer. The referenced buffer of the
 * bufferbuffer may not contain the ring buffer flag, this would result in not
 * incrementing the bufferbuffer index, so in effect only one buffer is used
 */
#define BUFFER_TYPE_RING (0x01<<1)

typedef struct BufferData_t {
    uint8_t type;
    uint8_t size;
} BufferData;

typedef struct BufferIndex_t {
    uint8_t index_put;
    uint8_t index_pop;
} BufferIndex;

/**
 * Buffer structure definition
 * The structure is valid for all Buffers, they can be casted to the right type
 * Fields:
 *  - buffer: a pointer to a data array
 *  - data: structure contains type (type of the buffer) and the length of buffer
 *  - index: contains two indexes: one for read, one for write operation
 *
 * Memory: structure takes up 4 Byte + 1 Pointer
 */
typedef struct Buffer_void_t {
    void * buffer;
    BufferData data;
    BufferIndex index;
} Buffer_void;

typedef struct BufferBuffer_void_t {
    Buffer_void** buffer;
    BufferData data;
    BufferIndex index;
} BufferBuffer_void;

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
 * @param type: the type of the buffer
 *      - regular (BUFFER_TYPE_REGULAR)
 *      - Bufferbuffer (BUFFER_TYPE_BUFFERBUFFER)
 *      - Ring (BUFFER_TYPE_RING)
 *      Bufferbuffer and Ring types can be combined
 *
 * @return the initialized buffer
 */
Buffer_void* initBuffer(void* data, uint8_t length, uint8_t type);

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
void resetBuffer(Buffer_void* buffer);

/**
 * returns the position of the buffer inside the buffer_mem array
 * @param buffer the buffer
 * @return the position of the buffer, -1 in case an invalid pointer was given
 */
int8_t BasicBuffer_getNumber(Buffer_void* buffer);

#endif /* MAXBUFFER_VOID */
#endif /* BUFFER_BASICBUFFER_H_ */
