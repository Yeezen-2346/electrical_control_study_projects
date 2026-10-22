#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

/* 硬件连接（三个灯都是低电平点亮）：
 *   LED1 -> PA0（TIM2_CH1，硬件 PWM 调亮度）
 *   LED2 -> PA4（普通 GPIO，软件 PWM 调亮度）
 *   LED3 -> PA7（普通 GPIO，软件 PWM 调亮度）
 */

/* 亮度范围：0 ~ 100，0 = 熄灭，100 = 常亮（最亮） */
#define LED_DUTY_MIN    0
#define LED_DUTY_MAX    100u

void LED_Init(void);                      /* PA0/PA4/PA7 初始化，输出高电平（全灭） */
void PWM_Init(void);                      /* PA0 -> TIM2_CH1，1kHz 硬件 PWM，占空比 0 */
void LED_AllOff(void);                    /* 三灯全灭 */
void LED_AllOn(void);                     /* 三灯常亮 */
void LED_SetBrightness(uint8_t duty);     /* 三灯同步亮度：duty = 0 ~ 100 */

#endif
