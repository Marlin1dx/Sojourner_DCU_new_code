/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_cdc.c
  * @brief          : USB CDC implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usb_cdc.h"
#include "motor_control.h"
#include "hall_sensors.h"
#include "imu.h"
#include "gps.h"
#include <string.h>
#include <stdio.h>
#include "usbd_desc.h"
#include "main.h"

/* USER CODE BEGIN PV */
#define USB_CDC_BUFFER_SIZE APP_RX_DATA_SIZE
#define USB_CDC_TX_BUFFER_SIZE 512

static uint8_t usb_cdc_buffer[USB_CDC_BUFFER_SIZE];
static uint16_t usb_cdc_buffer_len = 0;
static uint8_t tx_buffer[USB_CDC_TX_BUFFER_SIZE];

// Внешнее объявление hUsbDeviceFS
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */
#define CDC_IN_EP  0x81U  /* EP1 for data IN */
#define CDC_OUT_EP 0x01U  /* EP1 for data OUT */
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
void USB_CDC_Init(void) {
    MX_USB_DEVICE_Init();
}

void USB_CDC_SendData(const uint8_t* data, uint16_t size) {
    if (size > USB_CDC_TX_BUFFER_SIZE) {
        size = USB_CDC_TX_BUFFER_SIZE;
    }
    
    memcpy(tx_buffer, data, size);
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, tx_buffer, size);
    USBD_CDC_TransmitPacket(&hUsbDeviceFS);
}

void USB_CDC_ProcessReceivedData(void) {
    // Проверка наличия данных в буфере
    if (usb_cdc_buffer_len > 0) {
        // Обработка команд
        // Здесь можно добавить логику обработки полученных данных
        
        // Очистка буфера
        usb_cdc_buffer_len = 0;
        memset(usb_cdc_buffer, 0, USB_CDC_BUFFER_SIZE);
    }
}

void USB_CDC_SendTelemetry(const IMU_Data* imu, const GPS_Data* gps) {
    int len = snprintf((char*)tx_buffer, USB_CDC_TX_BUFFER_SIZE,
        "IMU:%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g|"
        "GPS:%g,%g,%g,%g,%g,%d,%d,%02d:%02d:%02d,%02d/%02d/%04d|"
        "HALL:%g,%g,%g,%g\n",
        imu->accel_x, imu->accel_y, imu->accel_z,
        imu->gyro_x, imu->gyro_y, imu->gyro_z,
        imu->mag_x, imu->mag_y, imu->mag_z,
        imu->temp,
        imu->pitch, imu->roll, imu->yaw,
        gps->latitude, gps->longitude, gps->altitude,
        gps->speed, gps->course,
        gps->satellites, gps->fix,
        gps->hour, gps->minute, gps->second,
        gps->day, gps->month, gps->year,
        HallSensors_GetAngle(HALL_ALF),
        HallSensors_GetAngle(HALL_ALR),
        HallSensors_GetAngle(HALL_ARF),
        HallSensors_GetAngle(HALL_ARR)
    );
    
    if (len > 0 && len < USB_CDC_TX_BUFFER_SIZE) {
        USB_CDC_SendData(tx_buffer, len);
    }
}

void USB_CDC_ReceiveCallback(uint8_t* Buf, uint32_t *Len) {
    // Копируем полученные данные в буфер
    usb_cdc_buffer_len = (*Len < USB_CDC_BUFFER_SIZE) ? *Len : USB_CDC_BUFFER_SIZE;
    memcpy(usb_cdc_buffer, Buf, usb_cdc_buffer_len);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    if (epnum == CDC_OUT_EP) {
        USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    }
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    if (epnum == CDC_IN_EP) {
        USBD_CDC_TransmitPacket(&hUsbDeviceFS);
    }
}
/* USER CODE END 1 */
