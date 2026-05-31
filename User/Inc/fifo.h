#ifndef __FIFO_H
#define __FIFO_H
#include "stm32f10x.h"
#include <stdbool.h>

#define FIFO_SIZE   128

typedef struct
{
    uint16_t buffer[FIFO_SIZE];
    uint16_t ptrRead;
    uint16_t ptrWrite;
} FIFO_t;

// 所有函数都接受 FIFO_t* 参数，无全局变量

void fifo_init(FIFO_t* fifo);
void fifo_enQueue(FIFO_t* fifo, uint16_t data);
uint16_t fifo_deQueue(FIFO_t* fifo);
bool fifo_isEmpty(FIFO_t* fifo);
uint16_t fifo_queueLength(FIFO_t* fifo);

#endif

