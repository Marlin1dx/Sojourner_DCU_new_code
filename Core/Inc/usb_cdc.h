#ifndef USB_CDC_H
#define USB_CDC_H

#include "main.h"

// Инициализация USB CDC
void USB_CDC_Init(void);

// Отправка данных через USB CDC
void USB_CDC_SendData(const uint8_t* data, uint16_t size);

// Обработка полученных данных
void USB_CDC_ProcessReceivedData(const uint8_t* data, uint16_t size);

// Отправка телеметрии
void USB_CDC_SendTelemetry(void);

#endif // USB_CDC_H 