#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

  #ifdef __cplusplus
  extern "C" {
  #endif 

  #include "stm32f1xx_hal.h"
  #include "main.h"

  extern DMA_HandleTypeDef hdma_adc1;
  extern DMA_HandleTypeDef hdma_i2c1_tx;
  extern TIM_HandleTypeDef htim3;

  void NMI_Handler(void);
  void HardFault_Handler(void);
  void MemManage_Handler(void);
  void BusFault_Handler(void);
  void UsageFault_Handler(void);
  void SVC_Handler(void);
  void DebugMon_Handler(void);
  void PendSV_Handler(void);
  void SysTick_Handler(void);
  void DMA1_Channel1_IRQHandler(void);
  void DMA1_Channel6_IRQHandler(void);
  void TIM3_IRQHandler(void);

  #ifdef __cplusplus
  }
  #endif

#endif /* __STM32F1xx_IT_H */
