#include "Motor.h"
#include "AppConfig.h"
#include "stm32f10x.h"

#define IN1  GPIO_Pin_12
#define IN2  GPIO_Pin_13
#define STBY GPIO_Pin_14

static uint16_t period_counts;
static int direction;

void Motor_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_ClocksTypeDef clocks;
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef pwm;
    uint32_t timer_hz;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    GPIO_ResetBits(GPIOB, IN1 | IN2 | STBY);
    gpio.GPIO_Pin = IN1 | IN2 | STBY;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    RCC_GetClocksFreq(&clocks);
    timer_hz = clocks.PCLK1_Frequency;
    if (clocks.PCLK1_Frequency != clocks.HCLK_Frequency)
        timer_hz *= 2u;
    period_counts = (uint16_t)(timer_hz / PWM_HZ);
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Period = period_counts - 1u;
    TIM_TimeBaseInit(TIM3, &timer);
    TIM_OCStructInit(&pwm);
    pwm.TIM_OCMode = TIM_OCMode_PWM1;
    pwm.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC1Init(TIM3, &pwm);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_Cmd(TIM3, ENABLE);
    Motor_Stop();
}

void Motor_Stop(void)
{
    GPIO_ResetBits(GPIOB, STBY | IN1 | IN2);
    TIM_SetCompare1(TIM3, 0);
    direction = 0;
}

void Motor_SetOutput(float output)
{
    uint16_t compare;
    int next_direction;
    int switching;

    if (output > DRIVE_LIMIT)
        output = DRIVE_LIMIT;
    if (output < -DRIVE_LIMIT)
        output = -DRIVE_LIMIT;
    if (output == 0.0f) {
        Motor_Stop();
        return;
    }

    next_direction = output > 0.0f ? MOTOR_SIGN : -MOTOR_SIGN;
    switching = next_direction != direction;
    if (switching) {
        GPIO_ResetBits(GPIOB, STBY | IN1 | IN2);
        GPIO_SetBits(GPIOB, next_direction > 0 ? IN1 : IN2);
        direction = next_direction;
    }
    if (output < 0.0f)
        output = -output;
    compare = (uint16_t)(output * period_counts + 0.5f);
    TIM_SetCompare1(TIM3, compare);
    if (switching)
        TIM_GenerateEvent(TIM3, TIM_EventSource_Update);
    GPIO_SetBits(GPIOB, STBY);
}
