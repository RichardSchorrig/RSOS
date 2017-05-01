/*
 * BasicBuffer.c
 *
 *  Created on: 11.02.2017
 *      Author: Richard
 */

#include "BasicBuffer.h"

/* exclude everything if not used */
#ifdef MAXBUFFER_VOID

Buffer_void buffer_mem[MAXBUFFER_VOID] = {0};
int8_t buffer_size = 0;

Buffer_void* initBuffer(void* data, uint8_t length, uint8_t type)
{
    buffer_mem[buffer_size].buffer = data;
    buffer_mem[buffer_size].data.size = length;
    buffer_mem[buffer_size].data.type = type;
    buffer_mem[buffer_size].index.index_pop = 0;
    buffer_mem[buffer_size].index.index_put = 0;
    buffer_size += 1;
    return &buffer_mem[buffer_size - 1];
}

void setBuffer(Buffer_void* buffer, void* data, uint8_t length)
{
    buffer->buffer = data;
    buffer->data.size = length;
    resetBuffer(buffer);
}

void setBufferLength(Buffer_void* buffer, uint8_t length)
{
    buffer->data.size = length;
    resetBuffer(buffer);
}

int8_t BasicBuffer_getNumber(Buffer_void* buffer)
{
    int8_t i;
    for (i=buffer_size; i>0; i-=1)
    {
        if (buffer == &buffer_mem[i-1])
        {
            return (i-1);
        }
    }
    return -1;
}

#endif /* MAXBUFFER_VOID */

