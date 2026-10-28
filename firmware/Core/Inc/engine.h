/*
 * engine.h
 *
 *  Created on: Oct 27, 2026
 *      Author: Pavel
 */

#ifndef INC_ENGINE_H_
#define INC_ENGINE_H_

#include "stm32g0xx.h"

#define VREFINT_CAL_ADDR  	((uint16_t*)0x1FFF75AA)
#define VREFINT_CHANNEL 	13
#define BAT_ADC_CHANNEL 	7



#define IMPULSE_COUNT 1

void configEngine();
void mainEngine();

#endif /* INC_ENGINE_H_ */
