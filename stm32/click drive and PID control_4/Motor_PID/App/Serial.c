#include "Serial.h"
#include "AppConfig.h"
#include "stm32f10x.h"

static char lines[2][SERIAL_LINE_CAPACITY];
static unsigned lengths[2];
static unsigned read_slot, write_slot, next_byte;

void Serial_Init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpio);
    USART_StructInit(&uart);
    uart.USART_BaudRate = UART_BAUD;
    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
}

int Serial_Read(void)
{
    uint16_t status = USART1->SR;
    if (status & (USART_SR_FE | USART_SR_NE | USART_SR_ORE)) {
        (void)USART1->DR;
        return -1;
    }
    if (status & USART_SR_RXNE)
        return USART_ReceiveData(USART1);
    return -1;
}

void Serial_Poll(void)
{
    unsigned length = lengths[read_slot];
    if (length != 0 && USART_GetFlagStatus(USART1, USART_FLAG_TXE)) {
        USART_SendData(USART1, (uint8_t)lines[read_slot][next_byte++]);
        if (next_byte == length) {
            lengths[read_slot] = 0;
            read_slot ^= 1u;
            next_byte = 0;
        }
    }
}

char *Serial_BeginLine(void)
{
    return lengths[write_slot] == 0 ? lines[write_slot] : 0;
}

void Serial_QueueLine(unsigned length)
{
    if (length != 0 && length < SERIAL_LINE_CAPACITY) {
        lengths[write_slot] = length;
        write_slot ^= 1u;
    }
}
