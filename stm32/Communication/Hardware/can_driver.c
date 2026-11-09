#include "can_driver.h"

static CAN_RxHandler s_rx_handler = 0;

void CAN1_SetRxHandler(CAN_RxHandler handler)
{
    s_rx_handler = handler;
}

void CAN1_AppInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /* PA11 = CAN_RX, PA12 = CAN_TX. */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    /*
     * APB1 = 36 MHz. 36 MHz / 9 = 4 MHz; one bit = 1 + 5 + 2 = 8 tq,
     * therefore 4 MHz / 8 = 500 kbit/s.
     */
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
    CAN_InitStructure.CAN_Prescaler = 9;

    (void)CAN_Init(CAN1, &CAN_InitStructure);

    /*
     * All-pass filter: ID and mask are all zero. Software still checks IDE,
     * RTR, StdId, DLC and payload fields in every receive callback.
     */
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

uint8_t CAN1_SendStd(uint16_t id, const uint8_t *data, uint8_t dlc)
{
    CanTxMsg message;
    uint8_t i;

    if (dlc > 8U)
    {
        return 0U;
    }

    if ((dlc > 0U) && (data == 0))
    {
        return 0U;
    }

    message.StdId = id;
    message.ExtId = 0U;
    message.IDE = CAN_ID_STD;
    message.RTR = CAN_RTR_DATA;
    message.DLC = dlc;

    for (i = 0U; i < dlc; i++)
    {
        message.Data[i] = data[i];
    }

    for (i = dlc; i < 8U; i++)
    {
        message.Data[i] = 0U;
    }

    if (CAN_Transmit(CAN1, &message) == CAN_TxStatus_NoMailBox)
    {
        return 0U;
    }

    return 1U;
}

/* Shared CAN1 RX0 vector for both board roles. */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CanRxMsg message;

    while (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
    {
        CAN_Receive(CAN1, CAN_FIFO0, &message);

        if (s_rx_handler != 0)
        {
            s_rx_handler(&message);
        }
    }
}
