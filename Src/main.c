/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <math.h>
#include <string.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

volatile uint16_t ADC_Buff[6];
int len;
char buff_R[100];
char buff_S[100];
char buff_T[100];
#define vR_Pin 0
#define vS_Pin 1
#define vT_Pin 2
#define iR_Pin 3
#define iS_Pin 4
#define iT_Pin 5

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/*
void voltage(unsigned int _inPinV, double _VCAL, double _PHASECAL);
void current(unsigned int _inPinI, double _ICAL);
void calcVI(unsigned int crossings, unsigned int timeout);
double calcIrms(unsigned int NUMBER_OF_SAMPLES);
*/
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
#define ADC_BITS    12
#define ADC_COUNTS  (1<<ADC_BITS)

void voltage(unsigned int _inPinV, double _VCAL, double _PHASECAL);
void current(unsigned int _inPinI, double _ICAL);
void calcVI(unsigned int crossings, unsigned int timeout);
double calcIrms(unsigned int NUMBER_OF_SAMPLES);
double realPower,apparentPower,powerFactor,Vrms,Irms;

unsigned int inPinV;
unsigned int inPinI;
double VCAL;
double ICAL;
double PHASECAL;
int sampleV;
int sampleI;
double lastFilteredV,filteredV;
double filteredI;
double offsetV;
double offsetI;
double phaseShiftedV;
double sqV,sumV,sqI,sumI,instP,sumP;
int startV;
bool lastVCross, checkVCross;

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *) ADC_Buff, 6);
	
	voltage(vR_Pin, 228.0, 1.7);
  current(iR_Pin, 23.9); //88.6
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		//************************************** PHASE_R **************************************************
		//calcIrms(64000);
		//sprintf(buff_R, "Irms : %.3f\r\n", Irms);
		
		calcVI(20, 2000);
		sprintf(buff_R, "Vrms : %.3f \t Irms : %.3f \t RP : %.3f \t AP : %.3f \t CosQ : %.3f \r\n", Vrms, Irms, realPower, apparentPower, powerFactor);
		
		len=sizeof(buff_R);
		HAL_UART_Transmit(&huart1, (uint8_t *)buff_R, len, 1000);
		memset(&buff_R[0], 0, sizeof(buff_R));
		//HAL_Delay(500);
		
		/*
		************************************** PHASE_S **************************************************
		voltage(vS_Pin, 220, 1.7);
		current(iS_Pin, 111.1);
		calcVI(20, 32000);
		sprintf(buff_S, "Vrms = %.2f \t Irms = %.2f \t CosQ = %.2f\r\n", Vrms, Irms, powerFactor);
		len=sizeof(buff_S);
		HAL_UART_Transmit(&huart1, (uint8_t *)buff_S, len, 1000);
		memset(&buff_S[0], 0, sizeof(buff_S));
		HAL_Delay(500);
		
		************************************** PHASE_T **************************************************
		voltage(vT_Pin, 220, 1.7);
		current(iT_Pin, 111.1);
		calcVI(20, 32000);
		sprintf(buff_T, "Vrms = %.2f \t Irms = %.2f \t CosQ = %.2f\r\n\r\n", Vrms, Irms, powerFactor);
		len=sizeof(buff_T);
		HAL_UART_Transmit(&huart1, (uint8_t *)buff_T, len, 1000);
		memset(&buff_T[0], 0, sizeof(buff_T));
		*/
  }
  /* USER CODE END 3 */

}

void voltage(unsigned int _inPinV, double _VCAL, double _PHASECAL)
{
  inPinV = _inPinV;
  VCAL = _VCAL;
  PHASECAL = _PHASECAL;
  offsetV = ADC_COUNTS>>1;
}

void current(unsigned int _inPinI, double _ICAL)
{
  inPinI = _inPinI;
  ICAL = _ICAL;
  offsetI = ADC_COUNTS>>1;
}

void calcVI(unsigned int crossings, unsigned int timeout)
{
  int SupplyVoltage = 3300.0;
  unsigned int crossCount = 0;
  unsigned int numberOfSamples = 0;
  bool st = false;
	unsigned long start = HAL_GetTick();
  while(st == false)
  {
    startV = ADC_Buff[inPinV];
    if ((startV < (ADC_COUNTS*0.55)) && (startV > (ADC_COUNTS*0.45))) st = true;
    if ((HAL_GetTick() - start) > timeout) st = true;
  }
  start = HAL_GetTick();
	while (crossCount < crossings)
	{
		while ((HAL_GetTick() - start) < timeout)
		{
			numberOfSamples++;
			lastFilteredV = filteredV;
			sampleV 			= ADC_Buff[inPinV];
			sampleI 			= ADC_Buff[inPinI];
			offsetV 			= offsetV + ((sampleV-offsetV)/4096);
			filteredV 		= sampleV - offsetV;
			offsetI 			= offsetI + ((sampleI-offsetI)/4096);
			filteredI 		= sampleI - offsetI;
			sqV						= filteredV * filteredV;
			sumV 				 += sqV;
			sqI 					= filteredI * filteredI;
			sumI 				 += sqI;
			phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV);
			instP 				= phaseShiftedV * filteredI;
			sumP    		 += instP;
			lastVCross 		= checkVCross;
			
			lastVCross = checkVCross;
			if (sampleV > startV) checkVCross = true;
			else checkVCross = false;
			if (numberOfSamples == 1) lastVCross = checkVCross;
			if (lastVCross != checkVCross) crossCount++;
		}
	}
	
  double V_RATIO 	= VCAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
	Vrms 						= V_RATIO * sqrt(sumV / numberOfSamples);
	double I_RATIO 	= ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms 						= I_RATIO * sqrt(sumI / numberOfSamples);
  realPower 			= V_RATIO * I_RATIO * (sumP / numberOfSamples);
  apparentPower 	= Vrms * Irms;
  powerFactor			= realPower / apparentPower;

  sumV = 0;
  sumI = 0;
  sumP = 0;
}

double calcIrms(unsigned int Number_of_Samples)
{
  int SupplyVoltage=3300.0;
  for (unsigned int n = 0; n < Number_of_Samples; n++)
  {
    sampleI = ADC_Buff[inPinI];
    offsetI = (offsetI + (sampleI-offsetI)/4096);
    filteredI = sampleI - offsetI;
    sqI = filteredI * filteredI;
    sumI += sqI;
  }
  double I_RATIO = ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms = I_RATIO * sqrt(sumI / Number_of_Samples);
  sumI = 0;
  return Irms;
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 6;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
