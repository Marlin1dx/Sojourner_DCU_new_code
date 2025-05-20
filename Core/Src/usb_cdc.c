#include "usb_cdc.h"
#include "motor_control.h"
#include "hall_sensors.h"
#include "imu.h"
#include "gps.h"
#include <string.h>
#include <stdio.h>

// Дескриптор USB CDC
static USBD_HandleTypeDef hUsbDeviceFS;

// Буфер для приема данных
#define USB_CDC_BUFFER_SIZE 64
static uint8_t usb_cdc_buffer[USB_CDC_BUFFER_SIZE];

// Инициализация USB CDC
void USB_CDC_Init(void) {
    // Инициализация USB
    USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
    USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
    USBD_Start(&hUsbDeviceFS);
}

// Отправка данных через USB CDC
void USB_CDC_SendData(const uint8_t* data, uint16_t size) {
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)data, size);
    USBD_CDC_TransmitPacket(&hUsbDeviceFS);
}

// Обработка полученных данных
void USB_CDC_ProcessReceivedData(const uint8_t* data, uint16_t size) {
    char cmd[32];
    int motor;
    int state;
    
    // Копируем команду в буфер
    if(size >= sizeof(cmd)) size = sizeof(cmd) - 1;
    memcpy(cmd, data, size);
    cmd[size] = '\0';
    
    // Парсим команду M<номер>:<состояние>
    if(sscanf(cmd, "M%d:%d", &motor, &state) == 2) {
        if(motor >= 0 && motor < MOTOR_COUNT) {
            MotorControl_SetMotorState(motor, state);
        }
    }
}

// Отправка телеметрии
void USB_CDC_SendTelemetry(void) {
    char buffer[256];
    const IMU_Data* imu = IMU_GetData();
    const GPS_Data* gps = GPS_GetData();
    
    // Формируем строку телеметрии
    snprintf(buffer, sizeof(buffer),
        "IMU:%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f|"
        "GPS:%.6f,%.6f,%.2f,%.2f,%.2f,%d,%d,%02d:%02d:%02d,%02d/%02d/%04d|"
        "HALL:%.2f,%.2f,%.2f,%.2f\n",
        imu->gyro_x, imu->gyro_y, imu->gyro_z,
        imu->accel_x, imu->accel_y, imu->accel_z,
        imu->mag_x, imu->mag_y, imu->mag_z,
        imu->roll, imu->pitch, imu->yaw,
        imu->temp,
        gps->latitude, gps->longitude,
        gps->altitude, gps->speed, gps->course,
        gps->satellites, gps->fix,
        gps->hour, gps->minute, gps->second,
        gps->day, gps->month, gps->year,
        HallSensors_GetAngle(HALL_ALF),
        HallSensors_GetAngle(HALL_ALR),
        HallSensors_GetAngle(HALL_ARF),
        HallSensors_GetAngle(HALL_ARR)
    );
    
    // Отправляем данные
    USB_CDC_SendData((uint8_t*)buffer, strlen(buffer));
}

// Обработчик прерывания USB CDC
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    if(epnum == CDC_OUT_EP) {
        USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    }
}

// Обработчик прерывания USB CDC
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    if(epnum == CDC_IN_EP) {
        USBD_CDC_TransmitPacket(&hUsbDeviceFS);
    }
} 