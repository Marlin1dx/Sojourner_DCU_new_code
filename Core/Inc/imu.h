#ifndef IMU_H
#define IMU_H

#include "main.h"

#include "imu_types.h"

// Инициализация IMU
void IMU_Init(void);

// Обновление данных IMU
void IMU_Update(void);

// Получение текущих данных IMU
const IMU_Data* IMU_GetData(void);

// Калибровка IMU
void IMU_Calibrate(void);

// Обработка I2C в прерывании
void IMU_ProcessI2C(void);

// Обработка ошибок I2C
void IMU_ProcessError(void);

#endif // IMU_H 