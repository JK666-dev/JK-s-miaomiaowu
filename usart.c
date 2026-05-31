#include "usart.h"

// ---------- USART1 全局变量 ----------
__IO bool rxFrameFlag1 = false;
__IO uint8_t rxCmd1[FIFO_SIZE] = {0};
__IO uint8_t rxCount1 = 0;
__IO FIFO_t rxFIFO1 = {0};

// ---------- USART2 全局变量 ----------
__IO bool rxFrameFlag2 = false;
__IO uint8_t rxCmd2[FIFO_SIZE] = {0};
__IO uint8_t rxCount2 = 0;
__IO FIFO_t rxFIFO2 = {0};

void usart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // ---------- USART1 初始化 (PA9-TX, PA10-RX) ----------
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    USART1->SR; USART1->DR;
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    // ---------- USART2 初始化 (PA2-TX, PA3-RX) ----------
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_Init(USART2, &USART_InitStructure);

    USART2->SR; USART2->DR;
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

// ---------- USART1 中断服务函数 ----------
void USART1_IRQHandler(void)
{
    __IO uint16_t i = 0;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        fifo_enQueue(&rxFIFO1, (uint8_t)USART1->DR);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
    else if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        USART1->SR; USART1->DR;
        rxCount1 = fifo_queueLength(&rxFIFO1);
        for(i=0; i < rxCount1; i++) { rxCmd1[i] = fifo_deQueue(&rxFIFO1); }
        rxFrameFlag1 = true;
    }
}

// ---------- USART2 中断服务函数 ----------
void USART2_IRQHandler(void)
{
    __IO uint16_t i = 0;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        fifo_enQueue(&rxFIFO2, (uint8_t)USART2->DR);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    else if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        USART2->SR; USART2->DR;
        rxCount2 = fifo_queueLength(&rxFIFO2);
        for(i=0; i < rxCount2; i++) { rxCmd2[i] = fifo_deQueue(&rxFIFO2); }
        rxFrameFlag2 = true;
    }
}

// ---------- 发送函数（增加 USARTx 参数） ----------
void usart_SendByte(USART_TypeDef* USARTx, uint16_t data)
{
    __IO uint16_t t0 = 0;
    USARTx->DR = (data & (uint16_t)0x01FF);
    while(!(USARTx->SR & USART_FLAG_TXE))
    {
        ++t0;
        if(t0 > 8000) return;
    }
}

void usart_SendCmd(USART_TypeDef* USARTx, __IO uint8_t *cmd, uint8_t len)
{
    uint8_t i;
    for(i=0; i < len; i++) { usart_SendByte(USARTx, cmd[i]); }
}


