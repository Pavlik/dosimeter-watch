/*
 * ring_buffer.h
 *
 *  Created on: Sep 21, 2026
 *      Author: Pavel
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 128

typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
} RingBuffer;

uint8_t ring_buffer_next_index(uint8_t index);
bool ring_buffer_is_full(RingBuffer *buf);
bool ring_buffer_is_empty(RingBuffer *buf);
uint8_t ring_buffer_count(RingBuffer *buf);
bool ring_buffer_get(RingBuffer *buf, uint8_t *data);
bool ring_buffer_put(RingBuffer *buf, uint8_t data);





#endif /* INC_RING_BUFFER_H_ */
