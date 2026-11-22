#include "Encoder.h"
#include "AppConfig.h"
#include "stm32f10x.h"

static uint16_t previous;
static int64_t counts;
static int direction;

void Encoder_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef timer;
    TIM_ICInitTypeDef input;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpio);

    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Period = 0xffffu;
    TIM_TimeBaseInit(TIM2, &timer);
    TIM_ICStructInit(&input);
    input.TIM_ICFilter = 6;
    TIM_ICInit(TIM2, &input);
    input.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM2, &input);
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_Cmd(TIM2, ENABLE);
    Encoder_Zero();
}

void Encoder_Zero(void)
{
    previous = TIM_GetCounter(TIM2);
    counts = 0;
    direction = 0;
}

void Encoder_Sample(void)
{
    uint16_t now = TIM_GetCounter(TIM2);
    int32_t delta = (uint16_t)(now - previous);
    previous = now;
    if (delta >= 32768)
        delta -= 65536;
    delta *= ENCODER_SIGN;
    counts += delta;
    direction = (delta > 0) - (delta < 0);
}

int64_t Encoder_GetCounts(void) { return counts; }
int Encoder_GetDirection(void) { return direction; }

float Encoder_GetPosition(void)
{
    const float counts_per_turn = ENCODER_COUNTS_PER_TURN;
    const float divisor = counts_per_turn > 0.0f ? counts_per_turn : 1.0f;
    const float scale = counts_per_turn > 0.0f ? 360.0f / divisor : 0.0f;
    return (float)counts * scale;
}
