#include "stdint.h"
#include "stm32f1xx_hal.h"

#ifndef __ss495a_H
#define __ss495a_H

  #ifdef __cplusplus
    extern "C" {
  #endif

    void ss495a_init(void);
    extern volatile uint32_t SS495_ADC_buffer;  //сделать 16битной

  #ifdef __cplusplus
    }
  #endif

#endif /*__ss495h_H */
