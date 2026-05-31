#include "fifo.h"

// 注意：不再定义任何全局 FIFO 变量！

// 初始化 FIFO
void fifo_init(FIFO_t* fifo)
{
    fifo->ptrRead  = 0;
    fifo->ptrWrite = 0;
}

// 入队
void fifo_enQueue(FIFO_t* fifo, uint16_t data)
{
    fifo->buffer[fifo->ptrWrite] = data;
    ++fifo->ptrWrite;
    if(fifo->ptrWrite >= FIFO_SIZE) fifo->ptrWrite = 0;
}

// 出队
uint16_t fifo_deQueue(FIFO_t* fifo)
{
    uint16_t element = fifo->buffer[fifo->ptrRead];
    ++fifo->ptrRead;
    if(fifo->ptrRead >= FIFO_SIZE) fifo->ptrRead = 0;
    return element;
}

// 判空
bool fifo_isEmpty(FIFO_t* fifo)
{
    return (fifo->ptrRead == fifo->ptrWrite);
}

// 队列长度
uint16_t fifo_queueLength(FIFO_t* fifo)
{
    if(fifo->ptrRead <= fifo->ptrWrite)
        return (fifo->ptrWrite - fifo->ptrRead);
    else
        return (FIFO_SIZE - fifo->ptrRead + fifo->ptrWrite);
}


