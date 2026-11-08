/*
 * adc_measurment.c
 *
 *  Created on: Nov 8, 2026
 *      Author: Pavel
 */
#include "adc_measurment.h"

//returns in mV
uint16_t ADC_CalcVDDA(uint16_t vref_raw)
{
	return
		(uint16_t)
		(
			(3000UL *(*VREFINT_CAL_ADDR))
			/ vref_raw
		);
}


int32_t ADC_CalcTemperature_x100(uint16_t temp_raw, uint16_t vref_raw)
{
	int32_t correctd_temp_raw;
	correctd_temp_raw =
		(
			(int32_t)temp_raw *
			(*VREFINT_CAL_ADDR)
		)
		/vref_raw;
	return
		3000 +
		(
			(correctd_temp_raw - (*TEMP30_CAL_ADDR)) * 10000
		)
		/ ((*TEMP130_CAL_ADDR) - (*TEMP30_CAL_ADDR));
}
