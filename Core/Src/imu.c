#include "imu.h"
#include <math.h>

// Адрес устройства MPU-6050 на I2C
#define MPU6050_ADDR         0x68

// Регистры MPU-6050
#define MPU6050_SMPLRT_DIV   0x19
#define MPU6050_CONFIG       0x1A
#define MPU6050_GYRO_CONFIG  0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_FIFO_EN      0x23
#define MPU6050_INT_ENABLE   0x38
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_TEMP_OUT_H   0x41
#define MPU6050_GYRO_XOUT_H  0x43
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_WHO_AM_I     0x75

// Данные IMU
static IMU_Data imu_data;

// Калибровочные данные
static struct {
    float gyro_offset[3];
    float accel_offset[3];
} calibration;

// Инициализация I2C
static void I2C_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

// Инициализация MPU-6050
static void MPU6050_Init(void) {
    uint8_t data;
    
    // Проверка устройства
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_WHO_AM_I, 1, &data, 1, 100);
    if(data != 0x68) {
        // Ошибка инициализации
        return;
    }
    
    // Сброс устройства
    data = 0x80;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_PWR_MGMT_1, 1, &data, 1, 100);
    HAL_Delay(100);
    
    // Включение и выбор источника тактирования
    data = 0x01; // PLL с X-axis гироскопом
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_PWR_MGMT_1, 1, &data, 1, 100);
    
    // Настройка частоты дискретизации
    data = 0x07; // 1kHz / (1 + 7) = 125Hz
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_SMPLRT_DIV, 1, &data, 1, 100);
    
    // Настройка фильтра
    data = 0x03; // Фильтр 44Hz для гироскопа и акселерометра
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_CONFIG, 1, &data, 1, 100);
    
    // Настройка гироскопа
    data = 0x18; // ±2000°/с
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_GYRO_CONFIG, 1, &data, 1, 100);
    
    // Настройка акселерометра
    data = 0x18; // ±16g
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_ACCEL_CONFIG, 1, &data, 1, 100);
}

void IMU_Init(void) {
    // Инициализация I2C
    I2C_Init();
    
    // Инициализация MPU-6050
    MPU6050_Init();
    
    // Инициализация калибровочных данных
    for(int i = 0; i < 3; i++) {
        calibration.gyro_offset[i] = 0.0f;
        calibration.accel_offset[i] = 0.0f;
    }
}

void IMU_Update(void) {
    uint8_t data[14];
    int16_t raw;
    
    // Чтение всех данных за один раз (акселерометр, температура, гироскоп)
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_ACCEL_XOUT_H, 1, data, 14, 100);
    
    // Обработка данных акселерометра
    raw = (data[0] << 8) | data[1];
    imu_data.accel_x = (raw / 2048.0f) - calibration.accel_offset[0]; // для ±16g
    raw = (data[2] << 8) | data[3];
    imu_data.accel_y = (raw / 2048.0f) - calibration.accel_offset[1];
    raw = (data[4] << 8) | data[5];
    imu_data.accel_z = (raw / 2048.0f) - calibration.accel_offset[2];
    
    // Обработка данных температуры
    raw = (data[6] << 8) | data[7];
    imu_data.temp = (raw / 340.0f) + 36.53f;
    
    // Обработка данных гироскопа
    raw = (data[8] << 8) | data[9];
    imu_data.gyro_x = (raw / 16.4f) - calibration.gyro_offset[0]; // для ±2000°/с
    raw = (data[10] << 8) | data[11];
    imu_data.gyro_y = (raw / 16.4f) - calibration.gyro_offset[1];
    raw = (data[12] << 8) | data[13];
    imu_data.gyro_z = (raw / 16.4f) - calibration.gyro_offset[2];
    
    // Вычисление углов ориентации
    // Roll (крен)
    imu_data.roll = atan2f(imu_data.accel_y, imu_data.accel_z) * 180.0f / 3.14159f;
    
    // Pitch (тангаж)
    imu_data.pitch = atan2f(-imu_data.accel_x, 
        sqrtf(imu_data.accel_y * imu_data.accel_y + 
              imu_data.accel_z * imu_data.accel_z)) * 180.0f / 3.14159f;
    
    // Yaw не может быть точно определен без магнитометра
    imu_data.yaw = 0.0f;
}

const IMU_Data* IMU_GetData(void) {
    return &imu_data;
}

void IMU_Calibrate(void) {
    const int samples = 1000;
    float gyro_sum[3] = {0};
    float accel_sum[3] = {0};
    
    // Сбор данных в неподвижном состоянии
    for(int i = 0; i < samples; i++) {
        uint8_t data[14];
        int16_t raw;
        
        HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_ACCEL_XOUT_H, 1, data, 14, 100);
        
        // Акселерометр
        raw = (data[0] << 8) | data[1];
        accel_sum[0] += raw / 2048.0f;
        raw = (data[2] << 8) | data[3];
        accel_sum[1] += raw / 2048.0f;
        raw = (data[4] << 8) | data[5];
        accel_sum[2] += raw / 2048.0f;
        
        // Гироскоп
        raw = (data[8] << 8) | data[9];
        gyro_sum[0] += raw / 16.4f;
        raw = (data[10] << 8) | data[11];
        gyro_sum[1] += raw / 16.4f;
        raw = (data[12] << 8) | data[13];
        gyro_sum[2] += raw / 16.4f;
        
        HAL_Delay(2);
    }
    
    // Вычисление средних значений
    for(int i = 0; i < 3; i++) {
        calibration.gyro_offset[i] = gyro_sum[i] / samples;
        calibration.accel_offset[i] = accel_sum[i] / samples;
    }
    
    // Корректировка смещения акселерометра по оси Z
    calibration.accel_offset[2] -= 1.0f; // Учитываем гравитацию
}

void IMU_ProcessI2C(void) {}
void IMU_ProcessError(void) {} 