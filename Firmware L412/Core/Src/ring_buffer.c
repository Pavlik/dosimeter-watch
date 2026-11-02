/*
 * ring_buffer.c
 *
 *  Created on: Sep 21, 2026
 *      Author: Pavel
 */

#include  "ring_buffer.h"

uint8_t ring_buffer_next_index(uint8_t index)
{
    index++;

    if (index == BUFFER_SIZE)
        index = 0;

    return index;
}

bool ring_buffer_is_full(RingBuffer *buf)
{
    uint8_t next_head = ring_buffer_next_index(buf->head);

    return next_head == buf->tail;
}

bool ring_buffer_is_empty(RingBuffer *buf)
{
    return buf->head == buf->tail;
}

uint8_t ring_buffer_count(RingBuffer *buf)
{
    if (buf->head >= buf->tail)
        return buf->head - buf->tail;

    return BUFFER_SIZE - buf->tail + buf->head;
}

bool ring_buffer_get(RingBuffer *buf, uint8_t *data)
{
    if (ring_buffer_is_empty(buf))
        return false;

    *data = buf->buffer[buf->tail];

    buf->tail++;

    if (buf->tail == BUFFER_SIZE)
        buf->tail = 0;

    return true;
}

bool ring_buffer_put(RingBuffer *buf, uint8_t data)
{
    if (ring_buffer_is_full(buf))
        return false;

    buf->buffer[buf->head] = data;
    buf->head = ring_buffer_next_index(buf->head);

    return true;
}




