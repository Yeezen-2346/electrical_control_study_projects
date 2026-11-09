#include "board_config.h"

#if (BOARD_ROLE == BOARD_ROLE_B)

#include "stm32f10x.h"
#include "board_b.h"
#include "can_driver.h"
#include "Delay.h"
#include "protocol.h"

#define BOARD_B_PWM_PERIOD            999U
#define BOARD_B_PWM_PRESCALER         71U

static void BoardB_PwmInit(void);
static void BoardB_AdcInit(void);
static void BoardB_SetBrightness(uint8_t level);
static uint16_t BoardB_ReadAdc(void);
static void BoardB_OnCanRx(const CanRxMsg *message);

static void BoardB_PwmInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* PA6 = TIM3_CH1, alternate function push-pull. */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*
     * 72 MHz / (71 + 1) = 1 MHz counter clock.
     * 1 MHz / (999 + 1) = 1 kHz PWM.
     */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = BOARD_B_PWM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = BOARD_B_PWM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0U;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

static void BoardB_AdcInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1,
                           ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); /* 72 MHz / 6 = 12 MHz ADCCLK. */

    /* PA0 = ADC1_IN0, analog input. */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_DeInit(ADC1);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1U;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1U,
                             ADC_SampleTime_55Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    /* Let the ADC stabilise, then run the required calibration. */
    Delay_us(10U);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) != RESET)
    {
    }

    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) != RESET)
    {
    }
}

static void BoardB_SetBrightness(uint8_t level)
{
    uint16_t compare;

    if (level > PROTO_BRIGHTNESS_MAX)
    {
        level = PROTO_BRIGHTNESS_MAX;
    }

    /*
     * 1 kHz period is 1000 counts (ARR = 999). level 0 -> 0 %,
     * level 5 -> 50 %, level 10 -> compare 1000 -> permanently high.
     */
    compare = (uint16_t)level * 100U;
    if (compare > 1000U)
    {
        compare = 1000U;
    }

    TIM_SetCompare1(TIM3, compare);
}

static uint16_t BoardB_ReadAdc(void)
{
    uint32_t guard;

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    guard = 0U;
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
    {
        guard++;
        if (guard > 100000U)
        {
            /* Extremely unlikely; return a safe value instead of hanging. */
            return 0U;
        }
    }

    /* Reading DR clears EOC. */
    return ADC_GetConversionValue(ADC1);
}

static void BoardB_OnCanRx(const CanRxMsg *message)
{
    uint8_t level;
    uint8_t payload[PROTO_CAN_DLC_VOLTAGE];
    uint16_t adc;
    uint16_t mv;

    if ((message->IDE != CAN_ID_STD) || (message->RTR != CAN_RTR_DATA))
    {
        return;
    }

    if ((message->StdId != PROTO_CAN_ID_BRIGHTNESS) ||
        (message->DLC != PROTO_CAN_DLC_BRIGHTNESS))
    {
        return;
    }

    if (Proto_DecodeBrightness(message->Data, message->DLC, &level) == 0U)
    {
        return;
    }

    BoardB_SetBrightness(level);

    /* One ADC sample and one 0x200 response for every valid command. */
    adc = BoardB_ReadAdc();
    mv = Proto_AdcToMillivolts(adc);
    (void)Proto_EncodeVoltage(adc, mv, payload);
    (void)CAN1_SendStd(PROTO_CAN_ID_VOLTAGE, payload,
                       PROTO_CAN_DLC_VOLTAGE);
}

void BoardB_Init(void)
{
    BoardB_PwmInit();
    BoardB_AdcInit();

    CAN1_SetRxHandler(BoardB_OnCanRx);
    CAN1_AppInit();

    /* Known safe initial state until the first valid command. */
    BoardB_SetBrightness(0U);
}

#else
/* Keep this translation unit non-empty when the board A target is built. */
typedef int BoardB_NotBuilt;
#endif /* BOARD_ROLE == BOARD_ROLE_B */
