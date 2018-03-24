#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"
#include "stm32f1xx_hal_adc.h"

//РЕШЕНИЕ: возможно continues mode отключить и настроить по прерыванию таймера раз в 100 милисекунд системным таймером
//НАСТРОИТЬ: sConfig.SamplingTime в adc.c
//НАСТРОИТЬ: PeriphClkInit.AdcClockSelection в main.c 
//TODO: ЧИТАТЬ И ОПТИМИЗИРОВАТЬ https://community.st.com/thread/36558-adc-values-reading-with-dma
//TODO: разобраться в ADC PRESCAILER
//TODO: adc watchdog для аккума с прерыванием
//TODO: разобраться с двойным напряжением(+-) на ss495а, сейчас используется от 0 до 3.3
extern volatile uint32_t ADC_Data;  //сделать 16битной



void ss495a_init() {
    MX_TIM3_Init();
    MX_ADC1_Init();
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_Data, 1);
    HAL_TIM_Base_Start_IT(&htim3);
}

// void start_ss495a_adc_conversion(void) {
//     HAL_ADC_Start_IT(&hadc1);
// }

/* вызывает по завершению конверсии start_ss495a_adc_conversion - написать сюда отправку триггера в VESC??? */
// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1) {
//     if (hadc1->Instance==ADC1) {
//     // HAL_ADC_Stop_DMA(&hadc1);
//     }
// }