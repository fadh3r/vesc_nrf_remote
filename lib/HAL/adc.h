#ifndef __adc_H
#define __adc_H

  #ifdef __cplusplus
    extern "C" {
  #endif

  #include "stm32f1xx_hal.h"

  extern DMA_HandleTypeDef hdma_adc1;
  extern ADC_HandleTypeDef hadc1;

  extern void _Error_Handler(char *, int);

  void MX_ADC1_Init(void);

  #ifdef __cplusplus
    }
  #endif

#endif /*__ adc_H */
