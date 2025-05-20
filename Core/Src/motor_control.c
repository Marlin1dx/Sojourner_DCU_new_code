#include "motor_control.h"

// Структура для хранения состояния моторов
static struct {
    MotorState states[MOTOR_COUNT];
    uint8_t shift_registers[3]; // Три сдвиговых регистра
} motor_control;

// Маппинг моторов на биты сдвиговых регистров
static const struct {
    uint8_t register_index;  // Индекс регистра (0-2)
    uint8_t bit1;           // Первый бит управления
    uint8_t bit2;           // Второй бит управления
} motor_mapping[MOTOR_COUNT] = {
    // LF (M0, M1)
    {0, 1, 2},
    // LC (M2, M3)
    {0, 3, 4},
    // LR (M4, M5)
    {0, 5, 6},
    // RF (M6, M7)
    {1, 1, 2},
    // RC (M8, M9)
    {1, 3, 4},
    // RR (M10, M11)
    {1, 5, 6},
    // ALF (M12, M13)
    {0, 7, 8},
    // ALR (M14, M15)
    {1, 7, 8},
    // ARF (M16, M17)
    {2, 1, 2},
    // ARR (M18, M19)
    {2, 3, 4}
};

void MotorControl_Init(void) {
    // Инициализация GPIO для сдвиговых регистров
    // SER, RCLK, SRCLK для каждого регистра
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Включаем тактирование портов
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // Настройка пинов для U2
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // Настройка пинов для U5
    GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Настройка пинов для U8
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // Инициализация состояний
    for(int i = 0; i < MOTOR_COUNT; i++) {
        motor_control.states[i] = MOTOR_STOP;
    }
    
    // Очистка сдвиговых регистров
    for(int i = 0; i < 3; i++) {
        motor_control.shift_registers[i] = 0;
    }
    
    MotorControl_Update();
}

void MotorControl_SetMotorState(MotorID motor, MotorState state) {
    if(motor >= MOTOR_COUNT) return;
    
    motor_control.states[motor] = state;
    MotorControl_Update();
}

void MotorControl_Update(void) {
    // Обновляем биты в сдвиговых регистрах
    for(int motor = 0; motor < MOTOR_COUNT; motor++) {
        uint8_t reg = motor_mapping[motor].register_index;
        uint8_t bit1 = motor_mapping[motor].bit1;
        uint8_t bit2 = motor_mapping[motor].bit2;
        
        // Очищаем биты для текущего мотора
        motor_control.shift_registers[reg] &= ~((1 << bit1) | (1 << bit2));
        
        // Устанавливаем новые биты в зависимости от состояния
        switch(motor_control.states[motor]) {
            case MOTOR_FORWARD:
                motor_control.shift_registers[reg] |= (1 << bit1);
                break;
            case MOTOR_BACKWARD:
                motor_control.shift_registers[reg] |= (1 << bit2);
                break;
            default:
                break;
        }
    }
    
    // Отправляем данные в сдвиговые регистры
    // U2
    for(int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (motor_control.shift_registers[0] >> i) & 0x01);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    
    // U5
    for(int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (motor_control.shift_registers[1] >> i) & 0x01);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    
    // U8
    for(int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, (motor_control.shift_registers[2] >> i) & 0x01);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
}

void MotorControl_CalibrateSteering(void) {
    // TODO: Реализовать калибровку поворотных двигателей
    // 1. Поворот в -45 градусов
    // 2. Запись минимальных значений датчиков Холла
    // 3. Поворот в +45 градусов
    // 4. Запись максимальных значений датчиков Холла
} 