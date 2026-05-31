/**
  * @file    usart3_dma.c
  * @brief   USART3 环形缓冲区 + 回显 + 坐标解析（带调试输出）
  */

#include "usart3_dma.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

VisionData_t vision = {0, 0, false};

#define RX_BUF_SIZE    256
static uint8_t rx_ring[RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

static uint8_t frame_buf[64];
static uint16_t frame_len = 0;
static volatile uint8_t frame_ready = 0;

// 调试开关：1=输出原始帧和解析结果，0=关闭
#define DEBUG_ENABLE   1

void USART3_DMA_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_ERR, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART3, ENABLE);
}

void USART3_IRQHandler(void)
{
    if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET ||
        USART_GetFlagStatus(USART3, USART_FLAG_FE)  != RESET ||
        USART_GetFlagStatus(USART3, USART_FLAG_NE)  != RESET)
    {
        volatile uint32_t tmp;
        tmp = USART3->SR;
        tmp = USART3->DR;
        (void)tmp;
    }

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART3);

        // 回显（方便调试）
        while (!(USART3->SR & USART_FLAG_TXE));
        USART3->DR = data;

        // LED翻转指示收到字节
        GPIO_WriteBit(GPIOC, GPIO_Pin_13,
                      (BitAction)!GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13));

        // 存入环形缓冲区
        uint16_t next = (rx_head + 1) % RX_BUF_SIZE;
        if (next != rx_tail) {
            rx_ring[rx_head] = data;
            rx_head = next;
        }

        // 检测换行符
        if (data == '\n') {
            frame_ready = 1;
        }
    }
}

static bool extract_frame(void)
{
    uint16_t start = rx_tail;
    uint16_t pos = start;
    uint16_t len = 0;

    while (pos != rx_head) {
        if (rx_ring[pos] == '\n') {
            if (start <= pos)
                len = pos - start + 1;
            else
                len = RX_BUF_SIZE - start + pos + 1;

            if (len <= sizeof(frame_buf)) {
                for (uint16_t i = 0; i < len; i++)
                    frame_buf[i] = rx_ring[(start + i) % RX_BUF_SIZE];
                frame_len = len;
                rx_tail = (pos + 1) % RX_BUF_SIZE;
                return true;
            } else {
                rx_tail = (pos + 1) % RX_BUF_SIZE;
                return false;
            }
        }
        pos = (pos + 1) % RX_BUF_SIZE;
    }
    return false;
}

static bool parse_coordinate(void)
{
    char str[32];
    int x = 0, y = 0;
    uint16_t i, j = 0;

    // 跳过非数字字符（如 '\r' 或空格），找到第一个数字
    for (i = 0; i < frame_len; i++) {
        char c = frame_buf[i];
        if (c >= '0' && c <= '9') break;
    }
    if (i >= frame_len) return false;

    // 将剩余部分拷贝到临时字符串，但只保留数字、逗号和负号（如果需要）
    for (; i < frame_len && j < 31; i++) {
        char c = frame_buf[i];
        if ((c >= '0' && c <= '9') || c == ',' || c == '-') {
            str[j++] = c;
        } else if (c == '\n' || c == '\r') {
            break; // 遇到换行或回车停止
        }
    }
    str[j] = '\0';

    // 尝试解析 "x,y"
    if (sscanf(str, "%d,%d", &x, &y) == 2) {
        vision.x = (int16_t)x;
        vision.y = (int16_t)y;
        vision.fresh = true;
        return true;
    }
    return false;
}

void USART3_SendString(char *str)
{
    while (*str) {
        while (!(USART3->SR & USART_FLAG_TXE));
        USART3->DR = (uint8_t)*str++;
    }
}

void USART3_SendByte(uint8_t data)
{
    while (!(USART3->SR & USART_FLAG_TXE));
    USART3->DR = data;
}

bool vision_data_available(void)
{
    if (frame_ready) {
        frame_ready = 0;
        if (extract_frame()) {
#if DEBUG_ENABLE
            // 发送原始帧内容（方便查看格式）
            USART3_SendString("[RAW] ");
            for (uint16_t i = 0; i < frame_len; i++) {
                USART3_SendByte(frame_buf[i]);
            }
            USART3_SendString("\r\n");
#endif
            if (parse_coordinate()) {
#if DEBUG_ENABLE
                char dbg[32];
                sprintf(dbg, "[OK] x=%d y=%d\r\n", vision.x, vision.y);
                USART3_SendString(dbg);
#endif
                return true;
            } else {
#if DEBUG_ENABLE
                USART3_SendString("[ERR] parse failed\r\n");
#endif
            }
        }
    }
    return false;
}

void vision_data_clear(void)
{
    vision.fresh = false;
}

