#ifndef __EMM_V5_H
#define __EMM_V5_H
#include "stm32f10x.h"
#include <stdbool.h>

// 系统参数类型枚举（必须列出所有成员）
typedef enum
{
    S_VER  = 0,    // 版本
    S_RL,          // 负载率
    S_PID,         // PID参数
    S_VBUS,        // 总线电压
    S_CPHA,        // 编码器相位
    S_ENCL,        // 编码器线数
    S_TPOS,        // 目标位置
    S_VEL,         // 实时速度
    S_CPOS,        // 当前位置
    S_PERR,        // 位置误差
    S_FLAG,        // 状态标志
    S_ORG,         // 回零状态
    S_Conf,        // 配置参数
    S_State        // 电机状态
} SysParams_t;

// 所有电机控制函数声明（增加 USARTx 参数）
void Emm_V5_Reset_CurPos_To_Zero(USART_TypeDef* USARTx, uint8_t addr);
void Emm_V5_Reset_Clog_Pro(USART_TypeDef* USARTx, uint8_t addr);
void Emm_V5_Read_Sys_Params(USART_TypeDef* USARTx, uint8_t addr, SysParams_t s);
void Emm_V5_Modify_Ctrl_Mode(USART_TypeDef* USARTx, uint8_t addr, bool svF, uint8_t ctrl_mode);
void Emm_V5_En_Control(USART_TypeDef* USARTx, uint8_t addr, bool state, bool snF);
void Emm_V5_Vel_Control(USART_TypeDef* USARTx, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);
void Emm_V5_Pos_Control(USART_TypeDef* USARTx, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF);
void Emm_V5_Stop_Now(USART_TypeDef* USARTx, uint8_t addr, bool snF);
void Emm_V5_Synchronous_motion(USART_TypeDef* USARTx, uint8_t addr);
void Emm_V5_Origin_Set_O(USART_TypeDef* USARTx, uint8_t addr, bool svF);
void Emm_V5_Origin_Modify_Params(USART_TypeDef* USARTx, uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir,
                                 uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF);
void Emm_V5_Origin_Trigger_Return(USART_TypeDef* USARTx, uint8_t addr, uint8_t o_mode, bool snF);
void Emm_V5_Origin_Interrupt(USART_TypeDef* USARTx, uint8_t addr);

#endif


