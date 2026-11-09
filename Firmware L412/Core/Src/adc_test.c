/*
 * test.c
 *
 *  Created on: Nov 8, 2026
 *      Author: Pavel
 */
#include "adc_test.h"
 	char buf[32] = {0,};
    uint16_t true_VDDA;
    int32_t true_temp;

    volatile uint8_t tim16_it_flag = 0;
    uint16_t* adc_value = 0;

void ADC_Test_Polling_Init()
{
	ADC_Init(ADC_MODE_POLLING);
	ADC_Enable();
	ADC_SetSequenceLength(1);
	ADC_SetSequenceChannel(1, ADC_CH5);
	ADC_SetSampleTime(ADC_CH5, ADC_SAMPLE_640_5CYCLES);
}

void ADC_TEST_DMA_Circular_TRGO_Init()
{
	ADC_Init(ADC_MODE_DMA_CIRCULAR);
	ADC_EnableVrefInt();
	ADC_EnableTempSensor();
	ADC_SetTrigger(ADC_TRIGGER_TIM15_TRGO);

	ADC_SetSequenceLength(ADC_DMA_AMOUNT_TRANSFER);
	ADC_SetSequenceChannel(1, ADC_CH5);
	ADC_SetSequenceChannel(2, ADC_CH0);
	ADC_SetSequenceChannel(3, ADC_CH17);


    ADC_SetSampleTime(ADC_CH5, ADC_SAMPLE_12_5CYCLES);
    ADC_SetSampleTime(ADC_CH0, ADC_SAMPLE_640_5CYCLES);
    ADC_SetSampleTime(ADC_CH17, ADC_SAMPLE_640_5CYCLES);

	ADC_Enable();
	ADC_Start();
	ADC_DMA_Config(CIRCULAR_MODE);
}

void ADC_Test_Polling_While()
{
	  if(tim16_it_flag)
	  {

		  ADC_Start();
		  if(ADC_WaitConversionComplete(100000))
		  {
			  uint16_t val = ADC_GetValue();
			  snprintf(buf, sizeof(buf),  "CH5 = %u\r\n", val);
			  uart_send_string_it(buf);

		  }
		  tim16_it_flag = 0;
	  }
}

void ADC_Test_DMA_Circular_TRGO_While()
{
	  if(tim16_it_flag)
	  {
		  tim16_it_flag = 0;
	  }
	  if(ADC_DMA_IsReady())
	  {
		  adc_value = ADC_DMA_Read_Sequence();
		  true_VDDA = ADC_CalcVDDA(adc_value[1]);
		  true_temp = ADC_CalcTemperature_x100(adc_value[2], adc_value[1]);

		  snprintf(buf, sizeof(buf),  "CH5 = %u\r\n", adc_value[0]);
		  uart_send_string_it(buf);
		  snprintf(buf, sizeof(buf),  "VREFINT = %u\r\n", true_VDDA);
		  uart_send_string_it(buf);
		  snprintf(buf, sizeof(buf),  "TEMP  = %ld\r\n", true_temp);
		  uart_send_string_it(buf);
		  snprintf(buf, sizeof(buf),  "---------------\r\n");
		  uart_send_string_it(buf);
	  }
}

void TIM1_UP_TIM16_IRQHandler(void)

{
    if (TIM16->SR & TIM_SR_UIF) {
        TIM16->SR &= ~TIM_SR_UIF; // Сброс флага

        tim16_it_flag = 1;

        GPIOA->ODR ^= GPIO_ODR_OD3;// Ваш код

    }
}
