#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "main.h"

// Определение состояний двигателей
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD = 1,
    MOTOR_BACKWARD = 2
} MotorState;

// Определение моторов
typedef enum {
    // Двигатели движения
    MOTOR_LF,    // Передний левый
    MOTOR_LC,    // Центральный левый
    MOTOR_LR,    // Задний левый
    MOTOR_RF,    // Передний правый
    MOTOR_RC,    // Центральный правый
    MOTOR_RR,    // Задний правый
    
    // Двигатели поворота
    MOTOR_ALF,   // Поворотный передний левый
    MOTOR_ALR,   // Поворотный задний левый
    MOTOR_ARF,   // Поворотный передний правый
    MOTOR_ARR,   // Поворотный задний правый
    
    MOTOR_COUNT  // Общее количество моторов
} MotorID;

// Инициализация системы управления двигателями
void MotorControl_Init(void);

// Установка состояния двигателя
void MotorControl_SetMotorState(MotorID motor, MotorState state);

// Обновление состояния всех двигателей
void MotorControl_Update(void);

// Калибровка поворотных двигателей
void MotorControl_CalibrateSteering(void);

#endif // MOTOR_CONTROL_H 