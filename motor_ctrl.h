#ifndef __MOTOR_CTRL_H
#define __MOTOR_CTRL_H

#include "stm32f10x.h"

void Motor_Init(void);
void Motor_SendRelative(USART_TypeDef* USARTx, uint8_t addr, uint8_t dir, uint16_t speed, uint8_t acc, int32_t pulses);
void Motor_AllStop(void);

#endif

