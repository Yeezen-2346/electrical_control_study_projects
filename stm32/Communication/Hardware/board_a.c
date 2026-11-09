#include "board_config.h"

#if (BOARD_ROLE == BOARD_ROLE_A)

#include "stm32f10x.h"
#include "board_a.h"
#include "can_driver.h"
#include "protocol.h"

static Proto_UartParser s_uart_parser;

static void BoardA_UartInit(void);
static void BoardA_UartSend(const uint8_t *data, uint8_t len);
static void BoardA_OnUartFrame(const uint8_t *data, uint8_t len);
static void BoardA_OnCanRx(const CanRxMsg *message);

static void BoardA_UartInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,
                           ENABLE);

    /* PA9 = USART1_TX, PA10 = USART1_RX. */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200U;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void BoardA_UartSend(const uint8_t *data, uint8_t len)
{
    uint8_t i;

    for (i = 0U; i < len; i++)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        {
        }
        USART_SendData(USART1, data[i]);
    }

    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {
    }
}

static void BoardA_OnUartFrame(const uint8_t *data, uint8_t len)
{
    uint8_t level;
    uint8_t payload[PROTO_CAN_DLC_BRIGHTNESS];

    if (Proto_DecodeBrightness(data, len, &level) == 0U)
    {
        return;
    }

    payload[0] = PROTO_CMD_BRIGHTNESS;
    payload[1] = level;

    /* Only a fully validated PC command reaches this CAN transmit call. */
    (void)CAN1_SendStd(PROTO_CAN_ID_BRIGHTNESS, payload,
                       PROTO_CAN_DLC_BRIGHTNESS);
}

static void BoardA_OnCanRx(const CanRxMsg *message)
{
    uint8_t frame[PROTO_UART_MAX_FRAME];
    uint8_t total;
    uint16_t adc;
    uint16_t mv;

    if ((message->IDE != CAN_ID_STD) || (message->RTR != CAN_RTR_DATA))
    {
        return;
    }

    if ((message->StdId != PROTO_CAN_ID_VOLTAGE) ||
        (message->DLC != PROTO_CAN_DLC_VOLTAGE))
    {
        return;
    }

    if (Proto_DecodeVoltage(message->Data, message->DLC, &adc, &mv) == 0U)
    {
        return;
    }

    /*
     * Re-wrap the five CAN payload bytes as:
     * AA 55 05 02 ADC_L ADC_H MV_L MV_H CRC8
     */
    total = Proto_BuildUartFrame(message->Data, PROTO_CAN_DLC_VOLTAGE, frame);
    if (total > 0U)
    {
        BoardA_UartSend(frame, total);
    }
}

void USART1_IRQHandler(void)
{
    uint8_t byte;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        byte = (uint8_t)USART_ReceiveData(USART1);
        Proto_UartParserFeed(&s_uart_parser, byte);
    }

    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
    {
        /* Drop a corrupted partial frame after an overrun. */
        (void)USART_ReceiveData(USART1);
        Proto_UartParserReset(&s_uart_parser);
    }
}

void BoardA_Init(void)
{
    Proto_UartParserInit(&s_uart_parser, BoardA_OnUartFrame);

    CAN1_SetRxHandler(BoardA_OnCanRx);
    CAN1_AppInit();

    BoardA_UartInit();
}

#else
/* Keep this translation unit non-empty when the board B target is built. */
typedef int BoardA_NotBuilt;
#endif /* BOARD_ROLE == BOARD_ROLE_A */
