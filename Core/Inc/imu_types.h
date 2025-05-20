#ifndef IMU_TYPES_H
#define IMU_TYPES_H

#include <stdint.h>

// Структура для хранения данных IMU
typedef struct {
    // Гироскоп (ITG3205)
    float gyro_x;    // Угловая скорость по оси X (град/с)
    float gyro_y;    // Угловая скорость по оси Y (град/с)
    float gyro_z;    // Угловая скорость по оси Z (град/с)
    float temp;      // Температура (°C)
    
    // Акселерометр (ADXL345)
    float accel_x;   // Ускорение по оси X (g)
    float accel_y;   // Ускорение по оси Y (g)
    float accel_z;   // Ускорение по оси Z (g)
    
    // Магнитометр (HMC5883L)
    float mag_x;     // Магнитное поле по оси X (Гаусс)
    float mag_y;     // Магнитное поле по оси Y (Гаусс)
    float mag_z;     // Магнитное поле по оси Z (Гаусс)
    
    // Ориентация (вычисленная)
    float roll;      // Крен (град)
    float pitch;     // Тангаж (град)
    float yaw;       // Рыскание (град)
} IMU_Data;

#endif /* IMU_TYPES_H */
