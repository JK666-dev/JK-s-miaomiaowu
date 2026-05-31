#include "main.h"


// 校验和生成
static uint8_t CalcChecksum(uint8_t* buf, uint8_t len)
{
    uint8_t sum = 0;
    for(uint8_t i=0; i<len; i++) sum += buf[i];
    return sum;
}

// 发送相对位置模式命令（13字节帧）
void Motor_SendRelative(USART_TypeDef* USARTx, uint8_t addr, uint8_t dir, uint16_t speed, uint8_t acc, int32_t pulses)
{
    uint8_t buf[13];
    buf[0] = addr;                  // 地址
    buf[1] = 0xFD;                  // 功能码
    buf[2] = dir;                   // 方向：0x00=CW, 0x01=CCW
    buf[3] = (speed >> 8) & 0xFF;   // 速度高字节
    buf[4] = speed & 0xFF;          // 速度低字节
    buf[5] = acc;                   // 加速度档位
    buf[6] = (pulses >> 24) & 0xFF; // 脉冲数最高字节
    buf[7] = (pulses >> 16) & 0xFF;
    buf[8] = (pulses >> 8) & 0xFF;
    buf[9] = pulses & 0xFF;
    buf[10] = 0x02;                 // 运动模式：相对当前实时位置
    buf[11] = 0x00;                 // 同步标志：立即执行
    buf[12] = CalcChecksum(buf, 12);

    // 发送前拉高 DE/RE 控制引脚（如果使用 RS485，此处省略，假设直接 TTL 连接）
    UART_SendBytes(USARTx, buf, 13);
}

void Motor_AllStop(void)
{
    // 发送立即停止命令（可选用位置模式脉冲数=0实现）
    Motor_SendRelative(MOTOR_X_UART, 0x01, 0x00, 500, 0, 0);
    Motor_SendRelative(MOTOR_Y_UART, 0x01, 0x00, 500, 0, 0);
}

