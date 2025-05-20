/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_cdc.h
  * @brief          : USB CDC header file
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef USB_CDC_H
#define USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"
#include "imu_types.h"
#include "gps.h"

// Инициализация USB CDC
void USB_CDC_Init(void);

// Отправка данных через USB CDC
void USB_CDC_SendData(const uint8_t* data, uint16_t size);

// Отправка телеметрии
void USB_CDC_SendTelemetry(const IMU_Data* imu, const GPS_Data* gps);

// Обработка полученных данных
void USB_CDC_ProcessReceivedData(void);

// Callback для приема данных
void USB_CDC_ReceiveCallback(uint8_t* Buf, uint32_t *Len);

// Внутренняя функция обработки команд
void USB_CDC_ProcessCommand(const uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* USB_CDC_H */