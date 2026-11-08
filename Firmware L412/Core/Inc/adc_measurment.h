/*
 * adc_measurment.h
 *
 *  Created on: Nov 8, 2026
 *      Author: Pavel
 */

#ifndef INC_ADC_MEASURMENT_H_
#define INC_ADC_MEASURMENT_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include <stdint.h>

#define VREFINT_CAL_ADDR ((uint16_t*)0x1FFF75AA)
#define TEMP30_CAL_ADDR ((uint16_t*)0x1FFF75A8)
#define TEMP130_CAL_ADDR ((uint16_t*)0x1FFF75CA)

uint16_t ADC_CalcVDDA(uint16_t vref_raw);
int32_t ADC_CalcTemperature_x100(uint16_t temp_raw, uint16_t vref_raw);



#endif /* INC_ADC_MEASURMENT_H_ */
