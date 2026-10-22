#include "stm32f10x.h"
#include "Delay.h"

void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
    uint8_t KeyNum = 0;
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
    {
        Delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0);
        Delay_ms(20);
        KeyNum = 1;
    }
    return KeyNum;
}

int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    Key_Init();

        while (1)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0) 
        {
            GPIO_Write(GPIOA, ~0x0001);
            Delay_ms(200); 
            GPIO_Write(GPIOA, ~0x0002); 
            Delay_ms(200);
            GPIO_Write(GPIOA, ~0x0004); 
            Delay_ms(200);
        }
        else
        {
            GPIO_Write(GPIOA, ~0x0007); 
            Delay_ms(500);
            GPIO_Write(GPIOA, 0x0007);  
            Delay_ms(500);
        }
    }
}