#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

/* UART frame: AA 55 LEN DATA[LEN] CRC8, CRC8 covers LEN + DATA. */
#define PROTO_UART_HEAD0              0xAAU
#define PROTO_UART_HEAD1              0x55U
#define PROTO_UART_MIN_DATA           1U
#define PROTO_UART_MAX_DATA           8U
#define PROTO_UART_MAX_FRAME          (2U + 1U + PROTO_UART_MAX_DATA + 1U)
#define PROTO_UART_PARSER_BUF_SIZE    32U

/* UART command bytes. */
#define PROTO_CMD_BRIGHTNESS          0x01U
#define PROTO_CMD_VOLTAGE             0x02U

/* Brightness range: 0 = off, 10 = full brightness. */
#define PROTO_BRIGHTNESS_MIN          0U
#define PROTO_BRIGHTNESS_MAX          10U

/* CAN identifiers and payload lengths. */
#define PROTO_CAN_ID_BRIGHTNESS       0x100U
#define PROTO_CAN_ID_VOLTAGE          0x200U
#define PROTO_CAN_DLC_BRIGHTNESS      2U
#define PROTO_CAN_DLC_VOLTAGE         5U

#define PROTO_VDDA_MV                 3300U

typedef void (*Proto_FrameHandler)(const uint8_t *data, uint8_t len);

typedef struct
{
    uint8_t buf[PROTO_UART_PARSER_BUF_SIZE];
    uint8_t count;
    Proto_FrameHandler handler;
} Proto_UartParser;

/* CRC-8: polynomial 0x07, initial value 0x00, no reflection, no final XOR. */
uint8_t Proto_Crc8(const uint8_t *data, uint8_t len);

/* Build a complete UART frame. Returns total length, or 0 on invalid input. */
uint8_t Proto_BuildUartFrame(const uint8_t *data, uint8_t len, uint8_t *out);

/* Streaming parser: accepts arbitrary byte chunks and can resynchronise. */
void Proto_UartParserInit(Proto_UartParser *parser, Proto_FrameHandler handler);
void Proto_UartParserReset(Proto_UartParser *parser);
void Proto_UartParserFeed(Proto_UartParser *parser, uint8_t byte);

/* Protocol-level payload checks. Return 1 when valid, 0 otherwise. */
uint8_t Proto_DecodeBrightness(const uint8_t *data, uint8_t len, uint8_t *level);
uint8_t Proto_DecodeVoltage(const uint8_t *data, uint8_t len,
                            uint16_t *adc, uint16_t *mv);

/* Build the 5-byte CAN/UART voltage payload: 02 ADC_L ADC_H MV_L MV_H. */
uint8_t Proto_EncodeVoltage(uint16_t adc, uint16_t mv, uint8_t *out);

/* MV = round(ADC * 3300 / 4095), with a 12-bit input clamp. */
uint16_t Proto_AdcToMillivolts(uint16_t adc);

#endif /* PROTOCOL_H */
