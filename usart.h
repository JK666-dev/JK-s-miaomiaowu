#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"
#include "fifo.h"
#include <stdbool.h>

// USART1 帧标志及缓存
extern __IO bool rxFrameFlag1;
extern __IO uint8_t rxCmd1[FIFO_SIZE];
extern __IO uint8_t rxCount1;

// USART2 帧标志及缓存
extern __IO bool rxFrameFlag2;
extern __IO uint8_t rxCmd2[FIFO_SIZE];
extern __IO uint8_t rxCount2;

void usart_init(void);
void usart_SendCmd(USART_TypeDef* USARTx, __IO uint8_t *cmd, uint8_t len);
void usart_SendByte(USART_TypeDef* USARTx, uint16_t data);

#endif


