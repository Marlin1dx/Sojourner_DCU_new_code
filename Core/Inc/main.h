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
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_i2c.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_pcd.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f103xb.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor_control.h"
#include "hall_sensors.h"
#include "imu.h"
#include "gps.h"
#include "usb_cdc.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart2;
extern PCD_HandleTypeDef hpcd_USB_FS;
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
#define LED_PIN GPIO_PIN_8
#define LED_GPIO_PORT GPIOA
#define HALL_SENSORS_PINS (GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7)
#define SHIFT_REGISTER_PINS (GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14)
#define TELEMETRY_INTERVAL 100

/* USER CODE BEGIN Private defines */
#define HSE_VALUE 8000000U
#define HSE_STARTUP_TIMEOUT 100U
#define HSI_VALUE 8000000U
#define LSI_VALUE 40000U
#define LSE_VALUE 32768U
#define LSE_STARTUP_TIMEOUT 5000U
#define VDD_VALUE 3300U
#define USE_SD_TRANSCEIVER 0

/* RCC definitions */
#define RCC_CR_HSEON                    ((uint32_t)0x00010000)
#define RCC_CR_HSION                    ((uint32_t)0x00000001)
#define RCC_CFGR_PLLSRC                 ((uint32_t)0x00010000)
#define RCC_CFGR_PLLMULL6               ((uint32_t)0x00140000)
#define RCC_CFGR_SW_PLL                 ((uint32_t)0x00000002)
#define RCC_CFGR_HPRE_DIV1              ((uint32_t)0x00000000)
#define RCC_CFGR_PPRE1_DIV2             ((uint32_t)0x00000400)
#define RCC_CFGR_PPRE1_DIV1             ((uint32_t)0x00000000)

/* FLASH definitions */
#define FLASH_LATENCY_1                 ((uint32_t)0x00000001)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
