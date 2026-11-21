/*
 * ring_buffer.c
 *
 *  Created on: Sep 21, 2026
 *      Author: Pavel
 */

#include  "ring_buffer.h"

static inline uint16_t ring_buffer_next_index(uint16_t index)
{
    index++;

    if (index == BUFFER_SIZE)
        index = 0;

    return index;
}

bool ring_buffer_is_full(const RingBuffer *buf)
{
	if (!buf) return true;   // защита: «полон», чтобы put не писал

    return ring_buffer_next_index(buf->head) == buf->tail;
}

bool ring_buffer_is_empty(const RingBuffer *buf)
{
	if (!buf) return true;

    return buf->head == buf->tail;
}

uint16_t ring_buffer_count(const RingBuffer *buf)
{
    if (!buf) return 0;

    /* Читаем head/tail по одному разу — иначе race с ISR */
    uint16_t h = buf->head;
    uint16_t t = buf->tail;

    if (h >= t)
        return (uint16_t)(h - t);
    return (uint16_t)(BUFFER_SIZE - t + h);
}

bool ring_buffer_get(RingBuffer *buf, uint8_t *data)
{
	if (!buf || !data) return false;

    if (ring_buffer_is_empty(buf)) return false;

    *data = buf->buffer[buf->tail];

    uint16_t t = ring_buffer_next_index(buf->tail);
    buf->tail = t;   // сначала данные, потом tail (для consumer)

    return true;
}

bool ring_buffer_put(RingBuffer *buf, uint8_t data)
{
	if (!buf) return false;
    if (ring_buffer_is_full(buf)) return false;

    buf->buffer[buf->head] = data;

    uint16_t h = ring_buffer_next_index(buf->head);
    buf->head = h;   // сначала данные, потом head (для producer)

    return true;
}




