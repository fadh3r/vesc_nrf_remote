#ifndef __tim_H
#define __tim_H

  #ifdef __cplusplus
    extern "C" {
  #endif

  #include "stm32f1xx_hal.h"

  extern TIM_HandleTypeDef htim3;

  extern void _Error_Handler(char *, int);
  void MX_TIM3_Init(void);

  #ifdef __cplusplus
    }
  #endif

#endif /*__ tim_H */
