/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "../leaf/leaf.h"
#include <math.h>
#include <stdint.h>
#include "B3_Voice.h"
#include "midi.h"
#include "tracks.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_Pin GPIO_PIN_0
#define CS_GPIO_Port GPIOB
#define DC_Pin GPIO_PIN_1
#define DC_GPIO_Port GPIOB
#define BTN_LEFT_Pin GPIO_PIN_10
#define BTN_LEFT_GPIO_Port GPIOD
#define BTN_LEFT_EXTI_IRQn EXTI15_10_IRQn
#define ENCODER_BTN_Pin GPIO_PIN_11
#define ENCODER_BTN_GPIO_Port GPIOD
#define ENCODER_BTN_EXTI_IRQn EXTI15_10_IRQn
#define BTN_RIGHT_Pin GPIO_PIN_14
#define BTN_RIGHT_GPIO_Port GPIOD
#define BTN_RIGHT_EXTI_IRQn EXTI15_10_IRQn
#define BTN_MID_Pin GPIO_PIN_15
#define BTN_MID_GPIO_Port GPIOD
#define BTN_MID_EXTI_IRQn EXTI15_10_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
