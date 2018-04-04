#include "tim.h"

TIM_HandleTypeDef htim3;

void MX_TIM3_Init(void) {
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 36000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 299;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  // htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE1; //TIM_CLOCKSOURCE_INTERNAL
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle) {

  if(tim_baseHandle->Instance==TIM3) {
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle) {

  if(tim_baseHandle->Instance==TIM3) {
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
  }
} 
