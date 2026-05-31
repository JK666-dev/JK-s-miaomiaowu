#include "board.h"
#include "delay.h"
#include "usart.h"
#include "Emm_V5.h"
#include "usart3_dma.h"   
#include <stdlib.h>

/* ---------- 硬件接线 ---------- */
#define YAW_UART     USART1
#define PITCH_UART   USART2
#define YAW_FLAG     rxFrameFlag1
#define PITCH_FLAG   rxFrameFlag2
#define MOTOR_ADDR   1

#define IMG_CENTER_X 160
#define IMG_CENTER_Y 120

// 控制参数
#define DEAD_ZONE_PX     3        // 死区：偏差小于此值认为已对准，不移动
#define MIN_STEP         5        // 最小移动步数（避免粘滞）
#define MAX_STEP         20      // 最大单次移动步数（限制超调）
#define PROPORTIONAL_K   2        // 比例系数：步数 = K * |偏差| (可调)

// 移动速度（RPM）和加速度，固定即可
#define MOVE_VEL         5      // 转速 150 RPM
#define MOVE_ACC         0       // 加速度档位

// 可选：积分参数（用于消除静差，但一般比例足够）
#define INTEGRAL_LIMIT   500      // 积分累计上限
static int32_t integral_x = 0, integral_y = 0;

volatile uint32_t sys_tick_ms = 0;
void SysTick_Handler(void) { sys_tick_ms++; }

void LED_Init(void) {
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    g.GPIO_Pin = GPIO_Pin_13; g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &g);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

/* 改进的电机移动函数：直接使用速度、加速度、步数（脉冲数） */
static void motor_move(USART_TypeDef* uart, volatile bool *flag,
                       uint8_t addr, uint8_t dir, uint32_t pulses, 
                       uint16_t vel, uint8_t acc)
{
    if (pulses == 0) return;
    // 调用位置模式控制（带速度、加速度）
    Emm_V5_Pos_Control(uart, addr, dir, vel, acc, pulses, 0, 0);
    
    uint32_t start = sys_tick_ms;
    while (*flag == false) {
        if ((sys_tick_ms - start) > 1000) { *flag = false; break; } // 超时1秒
    }
    *flag = false;
}

/* 计算单轴需要的步数（带比例、死区、积分） */
static uint32_t compute_step(int32_t error, int32_t *integral, int32_t kp)
{
    if (abs(error) <= DEAD_ZONE_PX) {
        *integral = 0;        // 死区内清除积分
        return 0;
    }
    
    // 积分项（可选，用于消除微小静差）
    *integral += error;
    if (*integral >  INTEGRAL_LIMIT) *integral =  INTEGRAL_LIMIT;
    if (*integral < -INTEGRAL_LIMIT) *integral = -INTEGRAL_LIMIT;
    
    // 比例 + 积分 步数
    int32_t step = kp * error + (*integral) / 10;  // 积分系数较小
    if (step < 0) step = -step;   // 步数取绝对值
    
    // 限制范围
    if (step < MIN_STEP) step = MIN_STEP;
    if (step > MAX_STEP) step = MAX_STEP;
    
    return (uint32_t)step;
}

int main(void) {
    board_init();
    LED_Init();
   
    
    if (SysTick_Config(SystemCoreClock / 1000)) while(1);
    delay_ms(1000);
    
    // 使能电机
    Emm_V5_En_Control(YAW_UART, MOTOR_ADDR, true, false);
    Emm_V5_En_Control(PITCH_UART, MOTOR_ADDR, true, false);
    
    while (1) {
        if (!vision_data_available()) { 
            delay_ms(5); 
            continue; 
        }
        
        int16_t cx = vision.x;
        int16_t cy = vision.y;
				
				
				
        vision_data_clear();
        
        int dx = cx - IMG_CENTER_X;          // 水平偏差（右正）
        int dy = IMG_CENTER_Y - cy;          // 垂直偏差（上正）
        
        // 计算步数
        uint32_t step_x = compute_step(dx, &integral_x, PROPORTIONAL_K);
        uint32_t step_y = compute_step(dy, &integral_y, PROPORTIONAL_K);
        
        // 如果两个方向都有运动，可考虑分别移动（当前电机不支持同时运动，串行执行）
        if (step_x > 0) {
             uint8_t dir = (dx > 0) ? 1 : 0;   // 右转
            motor_move(YAW_UART, &YAW_FLAG, MOTOR_ADDR, dir, step_x, MOVE_VEL, MOVE_ACC);
        }
        if (step_y > 0) {
            uint8_t dir = (dy > 0) ? 0 : 1;   // 仰头（根据你的实际方向调整）
            motor_move(PITCH_UART, &PITCH_FLAG, MOTOR_ADDR, dir, step_y, MOVE_VEL, MOVE_ACC);
        }
        
        // 只有移动后才回复 OK（也可以每次都回复，但避免频繁回复）
        if (step_x > 0 || step_y > 0) {
            USART3_SendString("OK\n");
        } else {
            // 已对准，无需移动，也可回复 OK 保持通信
            USART3_SendString("OK\n");
        }
        
        delay_ms(20);  // 小延时，避免连续占用总线
    }
}

