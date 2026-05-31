#include "Emm_V5.h"
#include "usart.h"

/**
  * @brief    将当前位置清零
  * @param    USARTx ：串口号（USART1 或 USART2）
  * @param    addr   ：电机地址
  * @retval   无
  */
void Emm_V5_Reset_CurPos_To_Zero(USART_TypeDef* USARTx, uint8_t addr)
{
    uint8_t cmd[4] = {addr, 0x0A, 0x6D, 0x6B};
    usart_SendCmd(USARTx, cmd, 4);
}

/**
  * @brief    解除堵转保护
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @retval   无
  */
void Emm_V5_Reset_Clog_Pro(USART_TypeDef* USARTx, uint8_t addr)
{
    uint8_t cmd[4] = {addr, 0x0E, 0x52, 0x6B};
    usart_SendCmd(USARTx, cmd, 4);
}

/**
  * @brief    读取系统参数
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @param    s      ：系统参数类型
  * @retval   无
  */
void Emm_V5_Read_Sys_Params(USART_TypeDef* USARTx, uint8_t addr, SysParams_t s)
{
    uint8_t i = 0;
    uint8_t cmd[16] = {0};

    cmd[i] = addr; ++i;

    switch(s)
    {
        case S_VER  : cmd[i] = 0x1F; ++i; break;
        case S_RL   : cmd[i] = 0x20; ++i; break;
        case S_PID  : cmd[i] = 0x21; ++i; break;
        case S_VBUS : cmd[i] = 0x24; ++i; break;
        case S_CPHA : cmd[i] = 0x27; ++i; break;
        case S_ENCL : cmd[i] = 0x31; ++i; break;
        case S_TPOS : cmd[i] = 0x33; ++i; break;
        case S_VEL  : cmd[i] = 0x35; ++i; break;
        case S_CPOS : cmd[i] = 0x36; ++i; break;
        case S_PERR : cmd[i] = 0x37; ++i; break;
        case S_FLAG : cmd[i] = 0x3A; ++i; break;
        case S_ORG  : cmd[i] = 0x3B; ++i; break;
        case S_Conf : cmd[i] = 0x42; ++i; cmd[i] = 0x6C; ++i; break;
        case S_State: cmd[i] = 0x43; ++i; cmd[i] = 0x7A; ++i; break;
        default: break;
    }

    cmd[i] = 0x6B; ++i;
    usart_SendCmd(USARTx, cmd, i);
}

/**
  * @brief    修改开环/闭环控制模式
  * @param    USARTx    ：串口号
  * @param    addr      ：电机地址
  * @param    svF       ：是否存储标志，false为不存储，true为存储
  * @param    ctrl_mode ：控制模式（0=关闭脉冲输入，1=开环，2=闭环，3=En复用多圈限位/Dir复用到位输出）
  * @retval   无
  */
void Emm_V5_Modify_Ctrl_Mode(USART_TypeDef* USARTx, uint8_t addr, bool svF, uint8_t ctrl_mode)
{
    uint8_t cmd[6] = {addr, 0x46, 0x69, svF, ctrl_mode, 0x6B};
    usart_SendCmd(USARTx, cmd, 6);
}

/**
  * @brief    使能信号控制
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @param    state  ：使能状态，true为使能，false为关闭
  * @param    snF    ：多机同步标志，false为不启用，true为启用
  * @retval   无
  */
void Emm_V5_En_Control(USART_TypeDef* USARTx, uint8_t addr, bool state, bool snF)
{
    uint8_t cmd[6] = {addr, 0xF3, 0xAB, (uint8_t)state, snF, 0x6B};
    usart_SendCmd(USARTx, cmd, 6);
}

/**
  * @brief    速度模式控制
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @param    dir    ：方向，0为CW，其余值为CCW
  * @param    vel    ：速度，范围0 - 5000 RPM
  * @param    acc    ：加速度档位，范围0 - 255，0为直接启动
  * @param    snF    ：多机同步标志，false为不启用，true为启用
  * @retval   无
  */
void Emm_V5_Vel_Control(USART_TypeDef* USARTx, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF)
{
    uint8_t cmd[8] = {
        addr, 0xF6, dir,
        (uint8_t)(vel >> 8), (uint8_t)(vel >> 0),
        acc, snF, 0x6B
    };
    usart_SendCmd(USARTx, cmd, 8);
}

/**
  * @brief    位置模式控制
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @param    dir    ：方向，0为CW，其余值为CCW
  * @param    vel    ：速度(RPM)，范围0 - 5000
  * @param    acc    ：加速度档位，范围0 - 255
  * @param    clk    ：脉冲数，范围0 - (2^32 - 1)
  * @param    raF    ：相位/绝对标志，false为相对运动，true为绝对值运动
  * @param    snF    ：多机同步标志，false为不启用，true为启用
  * @retval   无
  */
void Emm_V5_Pos_Control(USART_TypeDef* USARTx, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF)
{
    uint8_t cmd[13] = {
        addr, 0xFD, dir,
        (uint8_t)(vel >> 8), (uint8_t)(vel >> 0), acc,
        (uint8_t)(clk >> 24), (uint8_t)(clk >> 16), (uint8_t)(clk >> 8), (uint8_t)(clk >> 0),
        raF, snF, 0x6B
    };
    usart_SendCmd(USARTx, cmd, 13);
}

/**
  * @brief    立即停止（所有控制模式通用）
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @param    snF    ：多机同步标志，false为不启用，true为启用
  * @retval   无
  */
void Emm_V5_Stop_Now(USART_TypeDef* USARTx, uint8_t addr, bool snF)
{
    uint8_t cmd[5] = {addr, 0xFE, 0x98, snF, 0x6B};
    usart_SendCmd(USARTx, cmd, 5);
}

/**
  * @brief    多机同步运动
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @retval   无
  */
void Emm_V5_Synchronous_motion(USART_TypeDef* USARTx, uint8_t addr)
{
    uint8_t cmd[4] = {addr, 0xFF, 0x66, 0x6B};
    usart_SendCmd(USARTx, cmd, 4);
}

/**
  * @brief    设置单圈回零的零点位置
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @param    svF    ：是否存储标志，false为不存储，true为存储
  * @retval   无
  */
void Emm_V5_Origin_Set_O(USART_TypeDef* USARTx, uint8_t addr, bool svF)
{
    uint8_t cmd[5] = {addr, 0x93, 0x88, svF, 0x6B};
    usart_SendCmd(USARTx, cmd, 5);
}

/**
  * @brief    修改回零参数
  * @param    USARTx  ：串口号
  * @param    addr    ：电机地址
  * @param    svF     ：是否存储标志
  * @param    o_mode  ：回零模式（0=单圈就近，1=单圈方向，2=多圈无限位碰撞，3=多圈有限位开关）
  * @param    o_dir   ：回零方向（0=CW，其余=CCW）
  * @param    o_vel   ：回零速度(RPM)
  * @param    o_tm    ：回零超时时间(ms)
  * @param    sl_vel  ：无限位碰撞检测转速(RPM)
  * @param    sl_ma   ：无限位碰撞检测电流(mA)
  * @param    sl_ms   ：无限位碰撞检测时间(ms)
  * @param    potF    ：上电自动触发回零（false=不使能，true=使能）
  * @retval   无
  */
void Emm_V5_Origin_Modify_Params(USART_TypeDef* USARTx, uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir,
                                 uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF)
{
    uint8_t cmd[20] = {
        addr, 0x4C, 0xAE, svF, o_mode, o_dir,
        (uint8_t)(o_vel >> 8), (uint8_t)(o_vel >> 0),
        (uint8_t)(o_tm >> 24), (uint8_t)(o_tm >> 16), (uint8_t)(o_tm >> 8), (uint8_t)(o_tm >> 0),
        (uint8_t)(sl_vel >> 8), (uint8_t)(sl_vel >> 0),
        (uint8_t)(sl_ma >> 8), (uint8_t)(sl_ma >> 0),
        (uint8_t)(sl_ms >> 8), (uint8_t)(sl_ms >> 0),
        potF, 0x6B
    };
    usart_SendCmd(USARTx, cmd, 20);
}

/**
  * @brief    触发回零
  * @param    USARTx  ：串口号
  * @param    addr    ：电机地址
  * @param    o_mode  ：回零模式（0=单圈就近，1=单圈方向，2=多圈无限位碰撞，3=多圈有限位开关）
  * @param    snF     ：多机同步标志
  * @retval   无
  */
void Emm_V5_Origin_Trigger_Return(USART_TypeDef* USARTx, uint8_t addr, uint8_t o_mode, bool snF)
{
    uint8_t cmd[5] = {addr, 0x9A, o_mode, snF, 0x6B};
    usart_SendCmd(USARTx, cmd, 5);
}

/**
  * @brief    强制中断并退出回零
  * @param    USARTx ：串口号
  * @param    addr   ：电机地址
  * @retval   无
  */
void Emm_V5_Origin_Interrupt(USART_TypeDef* USARTx, uint8_t addr)
{
    uint8_t cmd[4] = {addr, 0x9C, 0x48, 0x6B};
    usart_SendCmd(USARTx, cmd, 4);
}

