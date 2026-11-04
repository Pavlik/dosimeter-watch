/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart_transmit.h"
#include "adc_l412.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t tim16_it_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

 	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

 	/*GPIO SYSTEM LED CONFIG START*/

 	GPIOA->MODER &= ~GPIO_MODER_MODE3;
 	GPIOA->MODER |= GPIO_MODER_MODE3_0;//|= (1Ul << GPIO_MODER_MODE3_Pos); // GPIO_MODER_MODE3_0;

 	/*GPIO SYSTEM LED CONFIG END*/

 	/*TIM16 CONFIG START*/

 	RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;

 	TIM16->PSC = 24000-1;
 	TIM16->ARR = 1000-1;
 	TIM16->CNT = 0;

 	TIM16->DIER |= TIM_DIER_UIE;
 	NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 0);
 	NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

 	TIM16->CR1 |= TIM_CR1_CEN;

 	/*TIM16 CONFIG END*/

 	/*GPIO UART RX/TX PA9-PA10 CONFIG START*/

 	 GPIOA->MODER &= ~GPIO_MODER_MODE10;
 	 GPIOA->MODER |= GPIO_MODER_MODE10_1; // 10 - af
 	 GPIOA->MODER &= ~GPIO_MODER_MODE9;
 	 GPIOA->MODER |= GPIO_MODER_MODE9_1; // 10 - af

 	 GPIOA->AFR[1] |= (7UL << GPIO_AFRH_AFSEL10_Pos) | (7UL << GPIO_AFRH_AFSEL9_Pos); // set PA9 and PA10 as AF7

 	/*GPIO UART RX/TX PA9-PA10 CONFIG END*/

 	/*UART1 CONFIG START*/

 	 RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
 	 RCC->CCIPR &= ~RCC_CCIPR_USART1SEL;
 	 USART1->CR1 &= ~USART_CR1_UE;

 	 /*Baudrate = 115200* BRR = 24000000 / 115200 = 208.33*/

 	 USART1->BRR = 208;

 	 USART1->CR1 = USART_CR1_TE | USART_CR1_RE; // ENABLE UART1 RX/TX
 	 USART1->CR1 |= USART_CR1_UE; // ENABLE UART1


 	ADC_Init();
 	ADC_DMA_Config_non_circular_mode();
 	uart_send_init();



 	/*UART1 CONFIG END*/

 	char buf[32] = {0,};
 	uint16_t adc_ch5_value = 0;

	ADC_SetSequenceLength(1);
	ADC_SetSequenceChannel(1, ADC_CH5);

    ADC_SetSampleTime(ADC_CH5, ADC_SAMPLE_640_5CYCLES);

 	HAL_Delay(1000);
 	snprintf(buf, sizeof(buf),  "CR   = %08lX\r\n", ADC1->CR);
 	uart_send_string_it(buf);
 	snprintf(buf, sizeof(buf),  "ISR  = %08lX\r\n", ADC1->ISR);
 	uart_send_string_it(buf);
 	snprintf(buf, sizeof(buf),  "CFGR = %08lX\r\n", ADC1->CFGR);
 	uart_send_string_it(buf);
 	snprintf(buf, sizeof(buf),  "SQR1 = %08lX\r\n", ADC1->SQR1);
 	uart_send_string_it(buf);



 	uint8_t adc_buf[2];


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if(tim16_it_flag)
	  {

		  ADC_Start();
		  while(DMA1_Channel1->CNDTR != 0);
		  adc_ch5_value = ADC_Read_DMA();
		  tim16_it_flag = 0;
//		  static uint8_t adc_busy = 0;
//		  if (!adc_busy)
//		  {
//		      if(!ADC_IsBusy())
//		    	  ADC_Start();
//		      adc_busy = 1;
//		  }
//		  else
//		  {
//		      if (ADC_IsConversionComplete())
//		      {
//		    	  adc_ch5_value = ADC_GetValue();
//		          adc_busy = 0;
//				  tim16_it_flag = 0;
//		      }
//		  }


		  adc_buf[0] =(uint8_t)(adc_ch5_value >> 8);
		  adc_buf[1] =(uint8_t)(adc_ch5_value & 0x00FF);
		  uart_send_data_it(adc_buf, 2);
		 	snprintf(buf, sizeof(buf),  "ADC = %d\r\n", adc_ch5_value);
		 	uart_send_string_it(buf);
		  //uart_send_byte_it('\n');
	  }
//	  snprintf(buf, sizeof(buf), "Hello UART1! \n");
//	  uart_send_string_it(buf);
	  //uart_send_byte_it(0x58);
	  //uart_send_byte_it('\n');

	  //GPIOA->ODR ^= GPIO_ODR_OD3;
	  //HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 9;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void TIM1_UP_TIM16_IRQHandler(void)

{
    if (TIM16->SR & TIM_SR_UIF) {
        TIM16->SR &= ~TIM_SR_UIF; // Сброс флага

        tim16_it_flag = 1;

        GPIOA->ODR ^= GPIO_ODR_OD3;// Ваш код

    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
