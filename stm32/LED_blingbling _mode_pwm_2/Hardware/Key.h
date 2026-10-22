#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

/* 硬件连接：KEY -> PB10（上拉输入，按下为低电平，下降沿触发中断） */

void Key_EXTI_Init(void);       /* PB10 初始化 + EXTI10 下降沿中断 + NVIC 使能 */

/* 查询是否有按键事件（只查询，不清除），1 = 有
 * 供呼吸灯等长动作中途退出用，保证按键响应及时
 */
uint8_t Key_Pending(void);

/* 取一次有效按键事件（会清除标志）：消抖 + 等待松手，1 = 确认按下了一次
 * 注意：里面有延时，只能在主循环里调用，不要在中断里调用
 */
uint8_t Key_GetEvent(void);

#endif
