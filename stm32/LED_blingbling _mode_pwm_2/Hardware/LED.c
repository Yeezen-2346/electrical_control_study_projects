#include "LED.h"
#include "Delay.h"

/* 硬件连接（低电平点亮）：
 *   LED1 -> PA0（TIM2_CH1，硬件 PWM）
 *   LED2 -> PA4（软件 PWM）
 *   LED3 -> PA7（软件 PWM）
 *
 * 说明：三个灯里只有 PA0 有定时器通道，PA4/PA7 没有，
 *       为了让三灯亮度完全同步，PA4/PA7 用软件 PWM（GPIO 快速开关）
 *       打出和 PA0 一样的 1kHz 占空比。
 */
#define LED1_PIN            GPIO_Pin_0
#define LED2_PIN            GPIO_Pin_4
#define LED3_PIN            GPIO_Pin_7
#define LED_SW_PINS         (LED2_PIN | LED3_PIN)   /* 软件 PWM 的两个灯 */

#define LED_PWM_STEPS       LED_DUTY_MAX            /* 一个软件 PWM 周期的级数 = 100 */
#define LED_PWM_STEP_US     10u                     /* 每级 10us -> 周期 1ms（1kHz） */
#define LED_PWM_REPEAT      10u                     /* 保持 10 个周期 = 10ms */

/**
  * @brief  LED 初始化：PA0/PA4/PA7 推挽输出，默认输出高电平（全灭）
  */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN | LED3_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 全部输出高电平 -> 三个灯都灭，这就是上电初始状态：熄灭 */
    GPIO_SetBits(GPIOA, LED1_PIN | LED2_PIN | LED3_PIN);
}

/**
  * @brief  PA0 硬件 PWM 初始化：TIM2_CH1，1kHz
  *         极性选 Low：占空比越大，引脚输出低电平的时间越长，灯越亮
  */
void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* PA0 复用推挽输出，交给 TIM2_CH1 控制 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = LED1_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 72MHz / 720 = 100kHz 计数；计满 100 个数 = 1kHz 的 PWM 周期 */
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;    /* 低电平点亮 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                          /* 占空比 0 -> 熄灭 */
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_Cmd(TIM2, ENABLE);
}

/**
  * @brief  三灯全灭（熄灭状态）
  */
void LED_AllOff(void)
{
    TIM_SetCompare1(TIM2, 0);                   /* PA0：占空比 0，输出高电平 */
    GPIO_SetBits(GPIOA, LED_SW_PINS);           /* PA4/PA7：输出高电平 */
}

/**
  * @brief  三灯常亮（常亮状态）
  */
void LED_AllOn(void)
{
    TIM_SetCompare1(TIM2, LED_DUTY_MAX);        /* PA0：占空比 100，一直输出低电平 */
    GPIO_ResetBits(GPIOA, LED_SW_PINS);         /* PA4/PA7：输出低电平 */
}

/**
  * @brief  设置三灯亮度（呼吸灯用）
  * @param  duty 0 ~ 100，0 = 灭，100 = 最亮
  * @note   PA0 用硬件 PWM；PA4/PA7 用软件 PWM 打出 1ms 的周期，重复 10 次
  *         （也就是这个亮度保持 10ms），三灯的平均亮度一致
  */
void LED_SetBrightness(uint8_t duty)
{
    uint8_t level;
    uint8_t repeat;

    if (duty > LED_DUTY_MAX)
    {
        duty = (uint8_t)LED_DUTY_MAX;
    }

    TIM_SetCompare1(TIM2, duty);                /* PA0：改硬件 PWM 占空比 */

    for (repeat = 0; repeat < LED_PWM_REPEAT; repeat++)
    {
        for (level = 0; level < LED_PWM_STEPS; level++)
        {
            if (level < duty)
            {
                GPIO_ResetBits(GPIOA, LED_SW_PINS);     /* 低电平点亮 */
            }
            else
            {
                GPIO_SetBits(GPIOA, LED_SW_PINS);
            }

            Delay_us(LED_PWM_STEP_US);                  /* 一级 10us */
        }
    }
}
