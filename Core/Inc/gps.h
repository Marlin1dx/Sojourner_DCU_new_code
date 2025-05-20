#ifndef GPS_H
#define GPS_H

#include "main.h"

// Структура для хранения данных GPS
typedef struct {
    float latitude;      // Широта (градусы)
    float longitude;     // Долгота (градусы)
    float altitude;      // Высота над уровнем моря (метры)
    float speed;         // Скорость (м/с)
    float course;        // Курс (градусы)
    uint8_t satellites;  // Количество спутников
    uint8_t fix;         // Тип фиксации (0=нет, 1=2D, 2=3D)
    uint8_t hour;        // Час (UTC)
    uint8_t minute;      // Минуты (UTC)
    uint8_t second;      // Секунды (UTC)
    uint8_t day;         // День
    uint8_t month;       // Месяц
    uint16_t year;       // Год
} GPS_Data;

// Инициализация GPS
void GPS_Init(void);

// Обновление данных GPS
void GPS_Update(void);

// Получение текущих данных GPS
const GPS_Data* GPS_GetData(void);

// Проверка наличия валидных данных
uint8_t GPS_HasValidData(void);

// Обработка UART для GPS
void GPS_ProcessUART(void);

#endif // GPS_H 