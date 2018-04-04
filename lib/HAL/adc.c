#include "adc.h"
#include "gpio.h"
#include "dma.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

void MX_ADC1_Init(void) {
  ADC_ChannelConfTypeDef sConfig;

    /* Common config */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

    /* Configure Regular Channel */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  /* ######## ADC SAMPLETIME ########
  ADC_CLK cycles to sample ADC input voltage
  Some time required to charge the ADC input capacitance and
  as long sampling time as slower ADC speed will be.
  https://electronics.stackexchange.com/questions/311326/stm32f20x-adc-sampling-time-rate/311338#311338
  https://stackoverflow.com/questions/42168388/stm32f103-adc-sampling-rate
  */
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5 ;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle) {

  GPIO_InitTypeDef GPIO_InitStruct;
  if(adcHandle->Instance==ADC1) {

    __HAL_RCC_ADC1_CLK_ENABLE();
  
    /* ADC1 GPIO Configuration: PA1 -> ADC1_IN1 */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle) {

  if(adcHandle->Instance==ADC1) {
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();
  
    /* ADC1 GPIO Configuration: PA1 -> ADC1_IN1 */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  }
} 
