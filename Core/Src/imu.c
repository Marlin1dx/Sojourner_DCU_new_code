#include "imu.h"

// Адреса устройств на I2C
#define ITG3205_ADDR    0x68
#define ADXL345_ADDR    0x53
#define HMC5883L_ADDR   0x1E

// Регистры ITG3205
#define ITG3205_WHO_AM_I        0x00
#define ITG3205_SMPLRT_DIV      0x15
#define ITG3205_DLPF_FS         0x16
#define ITG3205_INT_CFG         0x17
#define ITG3205_INT_STATUS      0x1A
#define ITG3205_TEMP_OUT_H      0x1B
#define ITG3205_TEMP_OUT_L      0x1C
#define ITG3205_GYRO_XOUT_H     0x1D
#define ITG3205_GYRO_XOUT_L     0x1E
#define ITG3205_GYRO_YOUT_H     0x1F
#define ITG3205_GYRO_YOUT_L     0x20
#define ITG3205_GYRO_ZOUT_H     0x21
#define ITG3205_GYRO_ZOUT_L     0x22
#define ITG3205_PWR_MGM         0x3E

// Регистры ADXL345
#define ADXL345_DEVID           0x00
#define ADXL345_POWER_CTL       0x2D
#define ADXL345_DATA_FORMAT     0x31
#define ADXL345_DATAX0          0x32
#define ADXL345_DATAX1          0x33
#define ADXL345_DATAY0          0x34
#define ADXL345_DATAY1          0x35
#define ADXL345_DATAZ0          0x36
#define ADXL345_DATAZ1          0x37

// Регистры HMC5883L
#define HMC5883L_CONFIG_A       0x00
#define HMC5883L_CONFIG_B       0x01
#define HMC5883L_MODE           0x02
#define HMC5883L_DATA           0x03
#define HMC5883L_STATUS         0x09
#define HMC5883L_ID_A           0x0A
#define HMC5883L_ID_B           0x0B
#define HMC5883L_ID_C           0x0C

// Дескриптор I2C
static I2C_HandleTypeDef hi2c1;

// Данные IMU
static IMU_Data imu_data;

// Калибровочные данные
static struct {
    float gyro_offset[3];
    float accel_offset[3];
    float mag_offset[3];
    float mag_scale[3];
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

// Инициализация ITG3205
static void ITG3205_Init(void) {
    uint8_t data;
    
    // Проверка устройства
    HAL_I2C_Mem_Read(&hi2c1, ITG3205_ADDR << 1, ITG3205_WHO_AM_I, 1, &data, 1, 100);
    if(data != 0x68) {
        // Ошибка инициализации
        return;
    }
    
    // Настройка частоты дискретизации
    data = 0x07; // 1kHz / (1 + 7) = 125Hz
    HAL_I2C_Mem_Write(&hi2c1, ITG3205_ADDR << 1, ITG3205_SMPLRT_DIV, 1, &data, 1, 100);
    
    // Настройка фильтра и диапазона
    data = 0x18; // ±2000°/с, 256Hz фильтр
    HAL_I2C_Mem_Write(&hi2c1, ITG3205_ADDR << 1, ITG3205_DLPF_FS, 1, &data, 1, 100);
    
    // Включение питания
    data = 0x00; // Нормальный режим
    HAL_I2C_Mem_Write(&hi2c1, ITG3205_ADDR << 1, ITG3205_PWR_MGM, 1, &data, 1, 100);
}

// Инициализация ADXL345
static void ADXL345_Init(void) {
    uint8_t data;
    
    // Проверка устройства
    HAL_I2C_Mem_Read(&hi2c1, ADXL345_ADDR << 1, ADXL345_DEVID, 1, &data, 1, 100);
    if(data != 0xE5) {
        // Ошибка инициализации
        return;
    }
    
    // Включение измерения
    data = 0x08; // Измерение включено
    HAL_I2C_Mem_Write(&hi2c1, ADXL345_ADDR << 1, ADXL345_POWER_CTL, 1, &data, 1, 100);
    
    // Настройка формата данных
    data = 0x0B; // ±16g, правовыравнивание
    HAL_I2C_Mem_Write(&hi2c1, ADXL345_ADDR << 1, ADXL345_DATA_FORMAT, 1, &data, 1, 100);
}

// Инициализация HMC5883L
static void HMC5883L_Init(void) {
    uint8_t data;
    
    // Проверка устройства
    HAL_I2C_Mem_Read(&hi2c1, HMC5883L_ADDR << 1, HMC5883L_ID_A, 1, &data, 1, 100);
    if(data != 'H') {
        // Ошибка инициализации
        return;
    }
    
    // Настройка конфигурации A
    data = 0x70; // 8 средних, 15Hz, нормальное измерение
    HAL_I2C_Mem_Write(&hi2c1, HMC5883L_ADDR << 1, HMC5883L_CONFIG_A, 1, &data, 1, 100);
    
    // Настройка конфигурации B
    data = 0x20; // ±1.3 Гаусс
    HAL_I2C_Mem_Write(&hi2c1, HMC5883L_ADDR << 1, HMC5883L_CONFIG_B, 1, &data, 1, 100);
    
    // Настройка режима
    data = 0x00; // Непрерывное измерение
    HAL_I2C_Mem_Write(&hi2c1, HMC5883L_ADDR << 1, HMC5883L_MODE, 1, &data, 1, 100);
}

void IMU_Init(void) {
    // Инициализация I2C
    I2C_Init();
    
    // Инициализация датчиков
    ITG3205_Init();
    ADXL345_Init();
    HMC5883L_Init();
    
    // Инициализация калибровочных данных
    for(int i = 0; i < 3; i++) {
        calibration.gyro_offset[i] = 0.0f;
        calibration.accel_offset[i] = 0.0f;
        calibration.mag_offset[i] = 0.0f;
        calibration.mag_scale[i] = 1.0f;
    }
}

void IMU_Update(void) {
    uint8_t data[6];
    int16_t raw;
    
    // Чтение данных гироскопа
    HAL_I2C_Mem_Read(&hi2c1, ITG3205_ADDR << 1, ITG3205_GYRO_XOUT_H, 1, data, 6, 100);
    raw = (data[0] << 8) | data[1];
    imu_data.gyro_x = (raw / 14.375f) - calibration.gyro_offset[0];
    raw = (data[2] << 8) | data[3];
    imu_data.gyro_y = (raw / 14.375f) - calibration.gyro_offset[1];
    raw = (data[4] << 8) | data[5];
    imu_data.gyro_z = (raw / 14.375f) - calibration.gyro_offset[2];
    
    // Чтение температуры
    HAL_I2C_Mem_Read(&hi2c1, ITG3205_ADDR << 1, ITG3205_TEMP_OUT_H, 1, data, 2, 100);
    raw = (data[0] << 8) | data[1];
    imu_data.temp = 35.0f + ((raw + 13200) / 280.0f);
    
    // Чтение данных акселерометра
    HAL_I2C_Mem_Read(&hi2c1, ADXL345_ADDR << 1, ADXL345_DATAX0, 1, data, 6, 100);
    raw = (data[1] << 8) | data[0];
    imu_data.accel_x = (raw * 0.0039f) - calibration.accel_offset[0];
    raw = (data[3] << 8) | data[2];
    imu_data.accel_y = (raw * 0.0039f) - calibration.accel_offset[1];
    raw = (data[5] << 8) | data[4];
    imu_data.accel_z = (raw * 0.0039f) - calibration.accel_offset[2];
    
    // Чтение данных магнитометра
    HAL_I2C_Mem_Read(&hi2c1, HMC5883L_ADDR << 1, HMC5883L_DATA, 1, data, 6, 100);
    raw = (data[0] << 8) | data[1];
    imu_data.mag_x = (raw * 0.92f) - calibration.mag_offset[0];
    raw = (data[2] << 8) | data[3];
    imu_data.mag_y = (raw * 0.92f) - calibration.mag_offset[1];
    raw = (data[4] << 8) | data[5];
    imu_data.mag_z = (raw * 0.92f) - calibration.mag_offset[2];
    
    // Применение масштабирования магнитометра
    imu_data.mag_x *= calibration.mag_scale[0];
    imu_data.mag_y *= calibration.mag_scale[1];
    imu_data.mag_z *= calibration.mag_scale[2];
    
    // Вычисление ориентации
    // Roll (крен)
    imu_data.roll = atan2f(imu_data.accel_y, imu_data.accel_z) * 180.0f / 3.14159f;
    
    // Pitch (тангаж)
    imu_data.pitch = atan2f(-imu_data.accel_x, 
        sqrtf(imu_data.accel_y * imu_data.accel_y + 
              imu_data.accel_z * imu_data.accel_z)) * 180.0f / 3.14159f;
    
    // Yaw (рыскание)
    float mag_x = imu_data.mag_x * cosf(imu_data.pitch * 3.14159f / 180.0f) +
                 imu_data.mag_z * sinf(imu_data.pitch * 3.14159f / 180.0f);
    float mag_y = imu_data.mag_x * sinf(imu_data.roll * 3.14159f / 180.0f) * 
                 sinf(imu_data.pitch * 3.14159f / 180.0f) +
                 imu_data.mag_y * cosf(imu_data.roll * 3.14159f / 180.0f) -
                 imu_data.mag_z * sinf(imu_data.roll * 3.14159f / 180.0f) * 
                 cosf(imu_data.pitch * 3.14159f / 180.0f);
    
    imu_data.yaw = atan2f(mag_y, mag_x) * 180.0f / 3.14159f;
    if(imu_data.yaw < 0) imu_data.yaw += 360.0f;
}

const IMU_Data* IMU_GetData(void) {
    return &imu_data;
}

void IMU_Calibrate(void) {
    // TODO: Реализовать калибровку IMU
    // 1. Сбор данных в неподвижном состоянии
    // 2. Вычисление смещений гироскопа
    // 3. Вычисление смещений акселерометра
    // 4. Калибровка магнитометра (сбор данных при вращении)
} 