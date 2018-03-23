#include "adc.h"
#include "dma.h"
#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"


//РЕШЕНИЕ: возможно continues mode отключить и настроить по прерыванию таймера раз в 100 милисекунд системным таймером
//НАСТРОИТЬ: sConfig.SamplingTime в adc.c
//НАСТРОИТЬ: PeriphClkInit.AdcClockSelection в main.c 
//TODO: ЧИТАТЬ И ОПТИМИЗИРОВАТЬ https://community.st.com/thread/36558-adc-values-reading-with-dma
//TODO: разобраться в ADC PRESCAILER
//TODO: adc watchdog для аккума с прерыванием
//TODO: разобраться с двойным напряжением(+-) на ss495а, сейчас используется от 0 до 3.3
extern volatile uint32_t ADC_Data;  //сделать 16битной



void ss495a_init() {
    MX_ADC1_Init();
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_Data, 1);
    // ADC_ResetCalibration(ADC1); // Reset previous calibration
    // while(ADC_GetResetCalibrationStatus(ADC1));
    // ADC_StartCalibration(ADC1); // Start new calibration (ADC must be off at that time)
    // while(ADC_GetCalibrationStatus(ADC1));
}

