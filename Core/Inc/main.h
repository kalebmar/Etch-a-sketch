/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ClownButton_Pin GPIO_PIN_13
#define ClownButton_GPIO_Port GPIOC
#define ClownButton_EXTI_IRQn EXTI4_15_IRQn
#define RED_LED_Pin GPIO_PIN_1
#define RED_LED_GPIO_Port GPIOF
#define BLUE_LED_Pin GPIO_PIN_2
#define BLUE_LED_GPIO_Port GPIOC
#define POT1_Pin GPIO_PIN_0
#define POT1_GPIO_Port GPIOA
#define POT2_Pin GPIO_PIN_1
#define POT2_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LCD_DB0_Pin GPIO_PIN_5
#define LCD_DB0_GPIO_Port GPIOA
#define LCD_DB1_Pin GPIO_PIN_6
#define LCD_DB1_GPIO_Port GPIOA
#define LCD_DB2_Pin GPIO_PIN_7
#define LCD_DB2_GPIO_Port GPIOA
#define LCD_CS2_Pin GPIO_PIN_5
#define LCD_CS2_GPIO_Port GPIOC
#define LCD_CS1_Pin GPIO_PIN_11
#define LCD_CS1_GPIO_Port GPIOB
#define BTN1_Pin GPIO_PIN_13
#define BTN1_GPIO_Port GPIOB
#define BTN2_Pin GPIO_PIN_14
#define BTN2_GPIO_Port GPIOB
#define LCD_RW_Pin GPIO_PIN_6
#define LCD_RW_GPIO_Port GPIOC
#define LCD_ENABLE_Pin GPIO_PIN_8
#define LCD_ENABLE_GPIO_Port GPIOC
#define LED_PWM_Pin GPIO_PIN_9
#define LED_PWM_GPIO_Port GPIOC
#define LCD_DB4_Pin GPIO_PIN_8
#define LCD_DB4_GPIO_Port GPIOA
#define LCD_DB3_Pin GPIO_PIN_9
#define LCD_DB3_GPIO_Port GPIOA
#define LCD_DB5_Pin GPIO_PIN_10
#define LCD_DB5_GPIO_Port GPIOA
#define LCD_DB6_Pin GPIO_PIN_11
#define LCD_DB6_GPIO_Port GPIOA
#define LCD_DB7_Pin GPIO_PIN_12
#define LCD_DB7_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define _245OE_Pin GPIO_PIN_12
#define _245OE_GPIO_Port GPIOC
#define LCD_DI_Pin GPIO_PIN_8
#define LCD_DI_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_9
#define LCD_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
