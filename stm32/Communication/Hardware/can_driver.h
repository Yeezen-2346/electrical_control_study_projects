#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include "stm32f10x.h"

typedef void (*CAN_RxHandler)(const CanRxMsg *message);

/* Configure PA11/PA12, CAN1 at 500 kbit/s and an all-pass RX0 filter. */
void CAN1_AppInit(void);

/* The callback runs in the CAN1 RX0 interrupt context. */
void CAN1_SetRxHandler(CAN_RxHandler handler);

/*
 * Send one 11-bit standard data frame.
 * Returns 1 when a mailbox accepted the frame, 0 when no mailbox was free.
 */
uint8_t CAN1_SendStd(uint16_t id, const uint8_t *data, uint8_t dlc);

#endif /* CAN_DRIVER_H */
