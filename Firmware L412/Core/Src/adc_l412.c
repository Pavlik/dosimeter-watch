/*
 * adc_l412.c
 *
 *  Created on: Nov 3, 2026
 *      Author: Navumchyk
 */

#include "adc_l412.h"

static volatile uint8_t adc_dma_ready = 0;

void ADC_Init(ADC_Mode_t mode)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; // ADC CLOCK ENABLE

	ADC1->CR &= ~ADC_CR_ADEN; // SWITCH OF ADC1
	while (ADC1->CR & ADC_CR_ADEN);

	ADC1->CR &= ~ADC_CR_DEEPPWD;

	ADC1->CR |= ADC_CR_ADVREGEN;
	for(volatile int i=0;i<1000;i++);

	ADC1->CR &= ~ADC_CR_ADCALDIF;
	ADC1->CR |= ADC_CR_ADCAL;
	while (ADC1->CR & ADC_CR_ADCAL);

	ADC1->CFGR = 0;

	switch(mode)
	{
	    case ADC_MODE_POLLING:
	        break;

	    case ADC_MODE_DMA_NORMAL:

	        ADC1->CFGR |= ADC_CFGR_DMAEN;
	        break;

	    case ADC_MODE_DMA_CIRCULAR:

	        ADC1->CFGR |= ADC_CFGR_DMAEN;
	        ADC1->CFGR |= ADC_CFGR_DMACFG;
	        break;
	}
}
void ADC_EnableVrefInt()
{
	/*ENABLE VREFINT INTERNAL CHANNEL*/
	ADC12_COMMON->CCR |= ADC_CCR_VREFEN;
}
void ADC_EnableTempSensor()
{
	/*ENABLE TEMPERATURE CH17 INTERNAL CHANNEL*/
	ADC12_COMMON->CCR |= ADC_CCR_TSEN;
}

void ADC_SetSampleTime(uint8_t channel, Adc_Sample_cycles sample_time)
{

	if(channel <= 9)
	{
		uint32_t pos = channel * 3U;
		ADC1->SMPR1 &= ~(0x7U << pos);
		ADC1->SMPR1 |= ((uint32_t)sample_time << pos);
	}
	else if(channel > 9 )
	{
		uint32_t pos = (channel - 10U) * 3U;
		ADC1->SMPR2 &= ~(0x7U << pos);
		ADC1->SMPR2 |= sample_time << pos;
	}

}

void ADC_SetSequenceLength(uint8_t len)
{
    if ((len < 1) || (len > 16))
        return;
    ADC1->SQR1 &= ~ADC_SQR1_L;
    ADC1->SQR1 |= ((len - 1U) << ADC_SQR1_L_Pos);
}

void ADC_SetSequenceChannel(uint8_t rank, uint8_t channel)
{
    volatile uint32_t *sqr;
    uint32_t pos;
    if ((rank < 1) || (rank > 16))
        return;
    if (rank <= 4)
    {
        sqr = &ADC1->SQR1;
        pos = 6U + ((rank - 1U) * 6U);
    }
    else if (rank <= 9)
    {
        sqr = &ADC1->SQR2;
        pos = (rank - 5U) * 6U;
    }
    else if (rank <= 14)
    {
        sqr = &ADC1->SQR3;
        pos = (rank - 10U) * 6U;
    }
    else
    {
        sqr = &ADC1->SQR4;
        pos = (rank - 15U) * 6U;
    }
    *sqr &= ~(0x1FU << pos);
    *sqr |= ((uint32_t)channel << pos);
}

void ADC_SetTrigger(ADC_Trigger_t trigger)
{
	ADC1->CFGR &= ~ADC_CFGR_EXTSEL;
	ADC1->CFGR &= ~ADC_CFGR_EXTEN;
	switch(trigger)
	{
		case ADC_TRIGGER_SOFTWARE:
			break;
		case ADC_TRIGGER_TIM15_TRGO:
		    ADC1->CFGR |= (14U << ADC_CFGR_EXTSEL_Pos); // source of adc_start
		    ADC1->CFGR |= ADC_CFGR_EXTEN_0; // ENABLE START FROM RISING EDGE
		    break;
	}
}

void ADC_Enable()
{
    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;

    while(!(ADC1->ISR & ADC_ISR_ADRDY));
}

void ADC_Disable()
{
    if (ADC1->CR & ADC_CR_ADEN)
    {
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN);
    }
}

void ADC_Start(void)
{
    ADC1->ISR |= ADC_ISR_EOC; // end of conversion flag
    ADC1->ISR |= ADC_ISR_EOS; // end of sequence flag
    ADC1->ISR |= ADC_ISR_OVR; // ADC overrun flag 0 - no overrum occurs/ 1 - overrun has occurred

    ADC1->CR |= ADC_CR_ADSTART;
}

void ADC_Stop(void)
{
    if (ADC1->CR & ADC_CR_ADSTART)
    {
        ADC1->CR |= ADC_CR_ADSTP;

        while (ADC1->CR & ADC_CR_ADSTP);
    }
}

void ADC_DMA_Enable()
{
	DMA1_Channel1->CCR |= DMA_CCR_EN; // Start DMA
}

void ADC_DMA_Disable()
{
	DMA1_Channel1->CCR &= ~DMA_CCR_EN; // Stop DMA
}


//only for  Transfer complete interrupt yet
void ADC_DMA_EnableIRQ(uint8_t priority)
{
	DMA1_Channel1->CCR |= DMA_CCR_TCIE;
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	NVIC_SetPriority(DMA1_Channel1_IRQn, priority);
}

//only for  Transfer complete interrupt yet
void ADC_DMA_DisableIRQ()
{
	DMA1_Channel1->CCR &= ~DMA_CCR_TCIE;
	NVIC_DisableIRQ(DMA1_Channel1_IRQn);
}


void ADC_DMA_Config(uint16_t *buffer, uint16_t length, DMA_Mode mode)
{
	ADC_DMA_Disable();
	DMA1_Channel1->CCR = 0;

	/* Enable circular mode*/
	if(mode == DMA_CIRCULAR_MODE)
		DMA1_Channel1->CCR |= DMA_CCR_CIRC;
	else
	{
		DMA1_Channel1->CCR &= ~DMA_CCR_CIRC;
	}

	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR; // data source
	DMA1_Channel1->CMAR = (uint32_t)buffer; // memory address !!!!& - only for one variable
	/*Because of adc is 12 bits wee need uint16_t data / Half Word for both sides*/
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0; // Peripheral - Half Word
	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0; // Memory     - Half Word

	DMA1_Channel1->CNDTR = length; // amount of transfer elements

	/*in one shot mode both stay in non-incremental mode*/
	/*in  circular mode peripheral stay in non-incremental mode and the memory become incremental*/

	DMA1_Channel1->CCR &= ~DMA_CCR_PINC; // disable increment of Peripheral
	if(length > 1)
		DMA1_Channel1->CCR |= DMA_CCR_MINC;  // enable memory increment3
	else
		DMA1_Channel1->CCR &= ~DMA_CCR_MINC;  // disable memory increment
	DMA1_Channel1->CCR &= ~DMA_CCR_DIR; // dirrection is from Peripheral to Memory
}


// Only for NORMAL DMA mode
void ADC_Start_DMA_Conversion(uint16_t length)
{
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    DMA1_Channel1->CNDTR = length;

    DMA1_Channel1->CCR |= DMA_CCR_EN;

    ADC_Start();
}


bool ADC_WaitConversionComplete(uint32_t timeout)
{
    while (!(ADC1->ISR & ADC_ISR_EOC))
    {
        if (--timeout == 0)
            return false;
    }

    return true;
}

bool ADC_IsConversionComplete(void)
{
    return (ADC1->ISR & ADC_ISR_EOC);
}

uint16_t ADC_GetValue(void)
{
    return (uint16_t)ADC1->DR;
}


uint16_t ADC_ReadChannel(uint8_t channel) // blocking method of reading channel
{
    ADC_SelectChannel(channel);
    ADC_SetSampleTime(channel, ADC_SAMPLE_640_5CYCLES);

    ADC_Start();

    //while(!ADC_IsConversionComplete());
    while(ADC_WaitConversionComplete(100000));

    return ADC_GetValue();
}

bool ADC_IsBusy(void)
{
    return (ADC1->CR & ADC_CR_ADSTART);
}

uint8_t ADC_DMA_IsReady()
{
	uint8_t buf_adc_dma_ready = adc_dma_ready;
	adc_dma_ready = 0;
	return buf_adc_dma_ready;
}

void DMA1_Channel1_IRQHandler(void)
{
    if(DMA1->ISR & DMA_ISR_TCIF1)
    {
    	DMA1->IFCR =
    	      DMA_IFCR_CGIF1
    	    | DMA_IFCR_CTCIF1
    	    | DMA_IFCR_CHTIF1
    	    | DMA_IFCR_CTEIF1;

        adc_dma_ready = 1;
    }
}





