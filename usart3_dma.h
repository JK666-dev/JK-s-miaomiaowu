#ifndef __USART3_DMA_H
#define __USART3_DMA_H

#include "stm32f10x.h"
#include <stdbool.h>

#define USART3_DMA_BUF_SIZE  256

typedef struct {
    int16_t x;
    int16_t y;
    bool fresh;
} VisionData_t;

extern VisionData_t vision;

void USART3_DMA_Init(uint32_t baudrate);
void USART3_SendByte(uint8_t data);
void USART3_SendString(char *str);
bool vision_data_available(void);
void vision_data_clear(void);

#endif
