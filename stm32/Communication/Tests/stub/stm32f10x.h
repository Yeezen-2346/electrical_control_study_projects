/*
 * Host-only declaration stub used to syntax-check the firmware sources with
 * GCC. It is NOT part of the Keil project and is not a hardware simulator.
 */
#ifndef STM32F10X_STUB_H
#define STM32F10X_STUB_H

#include <stdint.h>

typedef enum { DISABLE = 0, ENABLE = 1 } FunctionalState;
typedef enum { RESET = 0, SET = 1 } FlagStatus;
typedef FlagStatus ITStatus;

typedef struct { int dummy; } GPIO_TypeDef;
typedef struct { int dummy; } USART_TypeDef;
typedef struct { int dummy; } CAN_TypeDef;
typedef struct { int dummy; } TIM_TypeDef;
typedef struct { int dummy; } ADC_TypeDef;

#define GPIOA ((GPIO_TypeDef *)0)
#define USART1 ((USART_TypeDef *)0)
#define CAN1 ((CAN_TypeDef *)0)
#define TIM3 ((TIM_TypeDef *)0)
#define ADC1 ((ADC_TypeDef *)0)

#define GPIO_Pin_0  ((uint16_t)0x0001)
#define GPIO_Pin_6  ((uint16_t)0x0040)
#define GPIO_Pin_9  ((uint16_t)0x0200)
#define GPIO_Pin_10 ((uint16_t)0x0400)
#define GPIO_Pin_11 ((uint16_t)0x0800)
#define GPIO_Pin_12 ((uint16_t)0x1000)

typedef enum
{
    GPIO_Mode_AIN = 0,
    GPIO_Mode_IPU = 1,
    GPIO_Mode_AF_PP = 2
} GPIOMode_TypeDef;

typedef enum
{
    GPIO_Speed_50MHz = 3
} GPIOSpeed_TypeDef;

typedef struct
{
    uint16_t GPIO_Pin;
    GPIOSpeed_TypeDef GPIO_Speed;
    GPIOMode_TypeDef GPIO_Mode;
} GPIO_InitTypeDef;

#define RCC_APB2Periph_GPIOA  ((uint32_t)0x00000004)
#define RCC_APB2Periph_ADC1   ((uint32_t)0x00000200)
#define RCC_APB2Periph_USART1 ((uint32_t)0x00004000)
#define RCC_APB1Periph_CAN1   ((uint32_t)0x02000000)
#define RCC_APB1Periph_TIM3   ((uint32_t)0x00000002)
#define RCC_PCLK2_Div6        ((uint32_t)0x00000002)

void RCC_APB2PeriphClockCmd(uint32_t periph, FunctionalState state);
void RCC_APB1PeriphClockCmd(uint32_t periph, FunctionalState state);
void RCC_ADCCLKConfig(uint32_t div);
void GPIO_Init(GPIO_TypeDef *gpio, GPIO_InitTypeDef *init);

typedef struct
{
    uint32_t USART_BaudRate;
    uint16_t USART_WordLength;
    uint16_t USART_StopBits;
    uint16_t USART_Parity;
    uint16_t USART_HardwareFlowControl;
    uint16_t USART_Mode;
} USART_InitTypeDef;

#define USART_WordLength_8b             ((uint16_t)0x0000)
#define USART_StopBits_1                ((uint16_t)0x0000)
#define USART_Parity_No                 ((uint16_t)0x0000)
#define USART_HardwareFlowControl_None  ((uint16_t)0x0000)
#define USART_Mode_Rx                   ((uint16_t)0x0004)
#define USART_Mode_Tx                   ((uint16_t)0x0008)
#define USART_IT_RXNE                   ((uint16_t)0x0525)
#define USART_FLAG_TXE                  ((uint16_t)0x0080)
#define USART_FLAG_TC                   ((uint16_t)0x0040)
#define USART_FLAG_ORE                  ((uint16_t)0x0008)

void USART_Init(USART_TypeDef *usart, USART_InitTypeDef *init);
void USART_Cmd(USART_TypeDef *usart, FunctionalState state);
void USART_ITConfig(USART_TypeDef *usart, uint16_t it, FunctionalState state);
ITStatus USART_GetITStatus(USART_TypeDef *usart, uint16_t it);
FlagStatus USART_GetFlagStatus(USART_TypeDef *usart, uint16_t flag);
uint16_t USART_ReceiveData(USART_TypeDef *usart);
void USART_SendData(USART_TypeDef *usart, uint16_t data);

typedef struct
{
    uint8_t NVIC_IRQChannel;
    uint8_t NVIC_IRQChannelPreemptionPriority;
    uint8_t NVIC_IRQChannelSubPriority;
    FunctionalState NVIC_IRQChannelCmd;
} NVIC_InitTypeDef;

#define USART1_IRQn       37
#define USB_LP_CAN1_RX0_IRQn 20
#define NVIC_PriorityGroup_2 0x500

void NVIC_PriorityGroupConfig(uint32_t group);
void NVIC_Init(NVIC_InitTypeDef *init);

typedef struct
{
    FunctionalState CAN_TTCM;
    FunctionalState CAN_ABOM;
    FunctionalState CAN_AWUM;
    FunctionalState CAN_NART;
    FunctionalState CAN_RFLM;
    FunctionalState CAN_TXFP;
    uint8_t CAN_Mode;
    uint8_t CAN_SJW;
    uint8_t CAN_BS1;
    uint8_t CAN_BS2;
    uint16_t CAN_Prescaler;
} CAN_InitTypeDef;

typedef struct
{
    uint8_t CAN_FilterNumber;
    uint8_t CAN_FilterMode;
    uint8_t CAN_FilterScale;
    uint16_t CAN_FilterIdHigh;
    uint16_t CAN_FilterIdLow;
    uint16_t CAN_FilterMaskIdHigh;
    uint16_t CAN_FilterMaskIdLow;
    uint16_t CAN_FilterFIFOAssignment;
    FunctionalState CAN_FilterActivation;
} CAN_FilterInitTypeDef;

typedef struct
{
    uint32_t StdId;
    uint32_t ExtId;
    uint8_t IDE;
    uint8_t RTR;
    uint8_t DLC;
    uint8_t Data[8];
} CanTxMsg;

typedef struct
{
    uint32_t StdId;
    uint32_t ExtId;
    uint8_t IDE;
    uint8_t RTR;
    uint8_t DLC;
    uint8_t Data[8];
    uint8_t FMI;
} CanRxMsg;

#define CAN_Mode_Normal          ((uint8_t)0x00)
#define CAN_SJW_1tq              ((uint8_t)0x00)
#define CAN_BS1_5tq              ((uint8_t)0x04)
#define CAN_BS2_2tq              ((uint8_t)0x01)
#define CAN_FilterMode_IdMask    ((uint8_t)0x00)
#define CAN_FilterScale_32bit    ((uint8_t)0x01)
#define CAN_FIFO0                ((uint8_t)0x00)
#define CAN_IT_FMP0              ((uint32_t)0x00000002)
#define CAN_ID_STD               ((uint8_t)0x00)
#define CAN_RTR_DATA             ((uint8_t)0x00)
#define CAN_TxStatus_NoMailBox   ((uint8_t)0x04)

void CAN_DeInit(CAN_TypeDef *can);
void CAN_StructInit(CAN_InitTypeDef *init);
uint8_t CAN_Init(CAN_TypeDef *can, CAN_InitTypeDef *init);
void CAN_FilterInit(CAN_FilterInitTypeDef *init);
void CAN_ITConfig(CAN_TypeDef *can, uint32_t it, FunctionalState state);
ITStatus CAN_GetITStatus(CAN_TypeDef *can, uint32_t it);
uint8_t CAN_Transmit(CAN_TypeDef *can, CanTxMsg *msg);
void CAN_Receive(CAN_TypeDef *can, uint8_t fifo, CanRxMsg *msg);

typedef struct
{
    uint16_t TIM_Prescaler;
    uint16_t TIM_CounterMode;
    uint16_t TIM_Period;
    uint16_t TIM_ClockDivision;
    uint8_t TIM_RepetitionCounter;
} TIM_TimeBaseInitTypeDef;

typedef struct
{
    uint16_t TIM_OCMode;
    uint16_t TIM_OutputState;
    uint16_t TIM_Pulse;
    uint16_t TIM_OCPolarity;
} TIM_OCInitTypeDef;

#define TIM_CKD_DIV1            ((uint16_t)0x0000)
#define TIM_CounterMode_Up      ((uint16_t)0x0000)
#define TIM_OCMode_PWM1         ((uint16_t)0x0060)
#define TIM_OutputState_Enable  ((uint16_t)0x0001)
#define TIM_OCPolarity_High     ((uint16_t)0x0000)
#define TIM_OCPreload_Enable    ((uint16_t)0x0008)

void TIM_TimeBaseStructInit(TIM_TimeBaseInitTypeDef *init);
void TIM_TimeBaseInit(TIM_TypeDef *tim, TIM_TimeBaseInitTypeDef *init);
void TIM_OCStructInit(TIM_OCInitTypeDef *init);
void TIM_OC1Init(TIM_TypeDef *tim, TIM_OCInitTypeDef *init);
void TIM_OC1PreloadConfig(TIM_TypeDef *tim, uint16_t preload);
void TIM_ARRPreloadConfig(TIM_TypeDef *tim, FunctionalState state);
void TIM_Cmd(TIM_TypeDef *tim, FunctionalState state);
void TIM_SetCompare1(TIM_TypeDef *tim, uint16_t compare);

typedef struct
{
    uint32_t ADC_Mode;
    FunctionalState ADC_ScanConvMode;
    FunctionalState ADC_ContinuousConvMode;
    uint32_t ADC_ExternalTrigConv;
    uint32_t ADC_DataAlign;
    uint8_t ADC_NbrOfChannel;
} ADC_InitTypeDef;

#define ADC_Mode_Independent        ((uint32_t)0x00000000)
#define ADC_ExternalTrigConv_None   ((uint32_t)0x000E0000)
#define ADC_DataAlign_Right         ((uint32_t)0x00000000)
#define ADC_Channel_0               ((uint8_t)0x00)
#define ADC_SampleTime_55Cycles5    ((uint8_t)0x05)
#define ADC_FLAG_EOC                ((uint8_t)0x02)

void ADC_DeInit(ADC_TypeDef *adc);
void ADC_Init(ADC_TypeDef *adc, ADC_InitTypeDef *init);
void ADC_RegularChannelConfig(ADC_TypeDef *adc, uint8_t channel,
                              uint8_t rank, uint8_t sample_time);
void ADC_Cmd(ADC_TypeDef *adc, FunctionalState state);
void ADC_ResetCalibration(ADC_TypeDef *adc);
FlagStatus ADC_GetResetCalibrationStatus(ADC_TypeDef *adc);
void ADC_StartCalibration(ADC_TypeDef *adc);
FlagStatus ADC_GetCalibrationStatus(ADC_TypeDef *adc);
void ADC_SoftwareStartConvCmd(ADC_TypeDef *adc, FunctionalState state);
FlagStatus ADC_GetFlagStatus(ADC_TypeDef *adc, uint8_t flag);
uint16_t ADC_GetConversionValue(ADC_TypeDef *adc);

#define __WFI() ((void)0)

#endif /* STM32F10X_STUB_H */
