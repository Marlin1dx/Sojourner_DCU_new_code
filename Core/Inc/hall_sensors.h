#ifndef HALL_SENSORS_H
#define HALL_SENSORS_H

#include "main.h"

// Определение датчиков Холла
typedef enum {
    HALL_ALF,   // Передний левый поворотный
    HALL_ALR,   // Задний левый поворотный
    HALL_ARF,   // Передний правый поворотный
    HALL_ARR,   // Задний правый поворотный
    HALL_COUNT
} HallSensorID;

// Структура для хранения калибровочных данных
typedef struct {
    uint16_t min_value;
    uint16_t max_value;
    uint16_t center_value;
} HallCalibrationData;

// Инициализация датчиков Холла
void HallSensors_Init(void);

// Получение текущего значения датчика
uint16_t HallSensors_GetValue(HallSensorID sensor);

// Получение угла поворота в градусах (-45 до +45)
float HallSensors_GetAngle(HallSensorID sensor);

// Калибровка датчиков
void HallSensors_Calibrate(void);

// Обработка данных АЦП в прерывании
void HallSensors_ProcessADC(void);

// Получение калибровочных данных
const HallCalibrationData* HallSensors_GetCalibrationData(HallSensorID sensor);

#endif // HALL_SENSORS_H