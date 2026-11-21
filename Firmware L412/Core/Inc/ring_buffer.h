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
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer;


bool ring_buffer_is_full(const RingBuffer *buf);
bool ring_buffer_is_empty(const RingBuffer *buf);
uint16_t ring_buffer_count(const RingBuffer *buf);
bool ring_buffer_get(RingBuffer *buf, uint8_t *data);
bool ring_buffer_put(RingBuffer *buf, uint8_t data);





#endif /* INC_RING_BUFFER_H_ */
