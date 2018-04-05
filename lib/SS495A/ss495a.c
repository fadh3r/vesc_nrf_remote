#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"
#include "stm32f1xx_hal_adc.h"
#include "ss495a.h"


void ss495a_init() {
    MX_TIM3_Init();
    MX_ADC1_Init();
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &SS495_ADC_buffer, 1);
    // HAL_TIM_Base_Start_IT(&htim3);
}

/* вызывается по завершению конверсии ADC по прерыванию или оконачанию передачи DMA -> написать сюда отправку значения в VESC??? */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1) {
    if (hadc1->Instance==ADC1) {
        // HAL_TIM_Base_Start(&htim3);
        return 1;
    }
    
}

