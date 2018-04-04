#ifndef __i2c_H
#define __i2c_H

  #ifdef __cplusplus
    extern "C" {
  #endif

    #include "stm32f1xx_hal.h"

    extern I2C_HandleTypeDef hi2c1;
    extern DMA_HandleTypeDef hdma_i2c1_tx;

    extern void _Error_Handler(char *, int);

    void MX_I2C1_Init(void);

  #ifdef __cplusplus
    }
  #endif

#endif /*__ i2c_H */
