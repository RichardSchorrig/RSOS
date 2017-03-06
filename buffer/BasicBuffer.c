/*
 * BasicBuffer.c
 *
 *  Created on: 11.02.2017
 *      Author: Richard
 */

#include "BasicBuffer.h"

/* exclude everything if not used */
#ifdef MAXBUFFER_VOID

Buffer_void* initBuffer(void* data, uint8_t length) {
    buffer_mem[buffer_size].buffer = data;
    buffer_mem[buffer_size].size.size = length;
    buffer_mem[buffer_size].size.readBytes = 0;
    buffer_size += 1;
    return &buffer_mem[buffer_size - 1];
}

void setBuffer(Buffer_void* buffer, void* data, uint8_t length) {
    buffer->buffer = data;
    buffer->size.size = length;
    buffer->size.readBytes = 0;
}

void setBufferLength(Buffer_void* buffer, uint8_t length) {
    buffer->size.size = length;
    buffer->size.readBytes = 0;
}

void resetBuffer(Buffer_void* buffer) {
    buffer->size.readBytes = 0;
}

void get_uint8(Buffer_uint8* buffer, volatile uint8_t* destination) {
    *destination = buffer->buffer[buffer->size.readBytes];
}

int8_t set_getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination) {

    if (source != 0 && buffer->size.readBytes > 0) {
        buffer->buffer[buffer->size.readBytes-1] = *source;
    }

    if (destination != 0) {
        *destination = buffer->buffer[buffer->size.readBytes];
    }

    buffer->size.readBytes++;
    if (buffer->size.readBytes >= buffer->size.size) {
//        buffer->size.readBytes = 0;
        return -1;
    }

    return buffer->size.size - buffer->size.readBytes;
}

int8_t getNext_uint8(Buffer_uint8* buffer, volatile uint8_t* destination) {
    return set_getNext_uint8(buffer, 0, destination);
}

void set_uint8(Buffer_uint8* buffer, volatile uint8_t* source) {
    buffer->buffer[buffer->size.readBytes] = *source;
}

void reset_uint8(Buffer_uint8* buffer) {
    buffer->size.readBytes = 0;
}

void get_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination) {
    get_uint8(buffer->buffer[buffer->size.readBytes], destination);
}

int8_t set_getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source, volatile uint8_t* destination) {
    if (buffer->size.readBytes >= buffer->size.size) {
        return -1;
    }
    if (set_getNext_uint8(buffer->buffer[buffer->size.readBytes], source, destination) == -1) {
        buffer->size.readBytes++;
    }
    return buffer->size.size - buffer->size.readBytes;
}

int8_t getNext_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* destination) {
    return set_getNext_bufferbuffer_uint8(buffer, 0, destination);
}

void set_bufferbuffer_uint8(BufferBuffer_uint8* buffer, volatile uint8_t* source) {
    set_uint8(buffer->buffer[buffer->size.readBytes], source);
}

void reset_bufferbuffer_uint8(BufferBuffer_uint8* buffer) {

}

#endif /* MAXBUFFER_VOID */

