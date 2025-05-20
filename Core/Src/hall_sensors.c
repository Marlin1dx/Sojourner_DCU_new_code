#include "hall_sensors.h"
#include "motor_control.h"

// Структура для хранения калибровочных данных
static HallCalibrationData calibration_data[HALL_COUNT];

// Маппинг датчиков на пины ADC
static const uint32_t hall_pins[HALL_COUNT] = {
    GPIO_PIN_4,  // PA4 - ALF
    GPIO_PIN_5,  // PA5 - ALR
    GPIO_PIN_6,  // PA6 - ARF
    GPIO_PIN_7   // PA7 - ARR
};

// Маппинг датчиков на каналы ADC
static const uint32_t hall_channels[HALL_COUNT] = {
    ADC_CHANNEL_4,  // ALF
    ADC_CHANNEL_5,  // ALR
    ADC_CHANNEL_6,  // ARF
    ADC_CHANNEL_7   // ARR
};

// Маппинг датчиков на моторы поворота
static const MotorID hall_to_motor[HALL_COUNT] = {
    MOTOR_ALF,  // ALF
    MOTOR_ALR,  // ALR
    MOTOR_ARF,  // ARF
    MOTOR_ARR   // ARR
};

void HallSensors_Init(void) {
    // Включаем тактирование ADC и GPIO
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // Настройка GPIO для датчиков Холла
    for (int i = 0; i < HALL_COUNT; i++) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = hall_pins[i];
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    
    // Настройка ADC
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = HALL_COUNT;
    HAL_ADC_Init(&hadc1);
    
    // Настройка каналов ADC
    ADC_ChannelConfTypeDef sConfig = {0};
    for(int i = 0; i < HALL_COUNT; i++) {
        sConfig.Channel = hall_channels[i];
        sConfig.Rank = i + 1;
        sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    }
    
    // Запуск ADC
    HAL_ADC_Start(&hadc1);
}

uint16_t HallSensors_GetValue(HallSensorID sensor) {
    if(sensor >= HALL_COUNT) return 0;
    
    // Ждем завершения преобразования
    HAL_ADC_PollForConversion(&hadc1, 100);
    
    // Получаем значение
    return HAL_ADC_GetValue(&hadc1);
}

float HallSensors_GetAngle(HallSensorID sensor) {
    if(sensor >= HALL_COUNT) return 0.0f;
    
    uint16_t value = HallSensors_GetValue(sensor);
    const HallCalibrationData* cal = &calibration_data[sensor];
    
    // Линейная интерполяция между min и max значениями
    float range = cal->max_value - cal->min_value;
    if(range == 0) return 0.0f;
    
    float normalized = (float)(value - cal->min_value) / range;
    return (normalized * 90.0f) - 45.0f; // Преобразуем в диапазон -45 до +45
}

void HallSensors_Calibrate(void) {
    // Для каждого датчика
    for(int i = 0; i < HALL_COUNT; i++) {
        // Поворачиваем в -45 градусов
        MotorControl_SetMotorState(hall_to_motor[i], MOTOR_BACKWARD);
        HAL_Delay(3000); // Ждем 3 секунды для поворота
        
        // Записываем минимальное значение
        calibration_data[i].min_value = HallSensors_GetValue(i);
        
        // Поворачиваем в +45 градусов
        MotorControl_SetMotorState(hall_to_motor[i], MOTOR_FORWARD);
        HAL_Delay(3000); // Ждем 3 секунды для поворота
        
        // Записываем максимальное значение
        calibration_data[i].max_value = HallSensors_GetValue(i);
        
        // Вычисляем центральное значение
        calibration_data[i].center_value = 
            (calibration_data[i].min_value + calibration_data[i].max_value) / 2;
        
        // Останавливаем мотор
        MotorControl_SetMotorState(hall_to_motor[i], MOTOR_STOP);
    }
}

const HallCalibrationData* HallSensors_GetCalibrationData(HallSensorID sensor) {
    if(sensor >= HALL_COUNT) return NULL;
    return &calibration_data[sensor];
}

void HallSensors_ProcessADC(void) {} 