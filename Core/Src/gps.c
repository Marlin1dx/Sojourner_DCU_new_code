#include "main.h"
#include "gps.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Буфер для приема данных
#define GPS_BUFFER_SIZE 256
static uint8_t gps_buffer[GPS_BUFFER_SIZE];
static uint16_t buffer_index = 0;

// Данные GPS
static GPS_Data gps_data;

// Дескриптор UART
// UART_HandleTypeDef huart2;

// Инициализация UART
static void UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

// Парсинг NMEA предложения
static void ParseNMEA(const char* sentence) {
    char* token;
    char* rest = (char*)sentence;
    
    // Пропускаем $GPRMC или $GPGGA
    token = strtok_r(rest, ",", &rest);
    if(strcmp(token, "$GPRMC") == 0) {
        // Время
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.hour = (token[0] - '0') * 10 + (token[1] - '0');
            gps_data.minute = (token[2] - '0') * 10 + (token[3] - '0');
            gps_data.second = (token[4] - '0') * 10 + (token[5] - '0');
        }
        
        // Статус
        token = strtok_r(NULL, ",", &rest);
        gps_data.fix = (token[0] == 'A') ? 1 : 0;
        
        // Широта
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            float lat = atof(token);
            int degrees = (int)(lat / 100);
            float minutes = lat - (degrees * 100);
            gps_data.latitude = degrees + (minutes / 60.0f);
        }
        
        // N/S
        token = strtok_r(NULL, ",", &rest);
        if(token[0] == 'S') gps_data.latitude = -gps_data.latitude;
        
        // Долгота
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            float lon = atof(token);
            int degrees = (int)(lon / 100);
            float minutes = lon - (degrees * 100);
            gps_data.longitude = degrees + (minutes / 60.0f);
        }
        
        // E/W
        token = strtok_r(NULL, ",", &rest);
        if(token[0] == 'W') gps_data.longitude = -gps_data.longitude;
        
        // Скорость
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.speed = atof(token) * 0.514f; // Узлы в м/с
        }
        
        // Курс
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.course = atof(token);
        }
        
        // Дата
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.day = (token[0] - '0') * 10 + (token[1] - '0');
            gps_data.month = (token[2] - '0') * 10 + (token[3] - '0');
            gps_data.year = 2000 + (token[4] - '0') * 10 + (token[5] - '0');
        }
    }
    else if(strcmp(token, "$GPGGA") == 0) {
        // Время
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.hour = (token[0] - '0') * 10 + (token[1] - '0');
            gps_data.minute = (token[2] - '0') * 10 + (token[3] - '0');
            gps_data.second = (token[4] - '0') * 10 + (token[5] - '0');
        }
        
        // Широта
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            float lat = atof(token);
            int degrees = (int)(lat / 100);
            float minutes = lat - (degrees * 100);
            gps_data.latitude = degrees + (minutes / 60.0f);
        }
        
        // N/S
        token = strtok_r(NULL, ",", &rest);
        if(token[0] == 'S') gps_data.latitude = -gps_data.latitude;
        
        // Долгота
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            float lon = atof(token);
            int degrees = (int)(lon / 100);
            float minutes = lon - (degrees * 100);
            gps_data.longitude = degrees + (minutes / 60.0f);
        }
        
        // E/W
        token = strtok_r(NULL, ",", &rest);
        if(token[0] == 'W') gps_data.longitude = -gps_data.longitude;
        
        // Качество фиксации
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.fix = atoi(token);
        }
        
        // Количество спутников
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.satellites = atoi(token);
        }
        
        // Высота
        token = strtok_r(NULL, ",", &rest);
        if(token[0] != '\0') {
            gps_data.altitude = atof(token);
        }
    }
}

// Обработчик прерывания UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART2) {
        // Добавляем символ в буфер
        if(buffer_index < GPS_BUFFER_SIZE - 1) {
            gps_buffer[buffer_index++] = gps_buffer[0];
            
            // Проверяем на конец строки
            if(gps_buffer[0] == '\n') {
                gps_buffer[buffer_index] = '\0';
                ParseNMEA((char*)gps_buffer);
                buffer_index = 0;
            }
        }
        else {
            buffer_index = 0;
        }
        
        // Запускаем следующее чтение
        HAL_UART_Receive_IT(&huart2, gps_buffer, 1);
    }
}

void GPS_Init(void) {
    // Инициализация UART
    UART_Init();
    
    // Инициализация данных
    memset(&gps_data, 0, sizeof(GPS_Data));
    
    // Запуск приема данных
    HAL_UART_Receive_IT(&huart2, gps_buffer, 1);
}

void GPS_Update(void) {
    // Обновление происходит в прерывании
}

const GPS_Data* GPS_GetData(void) {
    return &gps_data;
}

uint8_t GPS_HasValidData(void) {
    return (gps_data.fix > 0);
}

// Обработка данных GPS через UART
void GPS_ProcessUART(void) {
    // Здесь будет логика обработки входящих UART данных
    // Например, парсинг NMEA-сообщений
    extern UART_HandleTypeDef huart2;
    uint8_t rx_data[1];
    
    if (HAL_UART_Receive(&huart2, rx_data, 1, 10) == HAL_OK) {
        // Логика обработки принятого байта
        // Можно добавить буферизацию и парсинг NMEA-протокола
    }
} 