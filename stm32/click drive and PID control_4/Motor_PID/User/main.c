#include "stm32f10x.h"
#include "Control.h"
#include "Encoder.h"
#include "Motor.h"
#include "Serial.h"

static volatile uint32_t milliseconds;

void SysTick_Handler(void)
{
    ++milliseconds;
}

int main(void)
{
    uint32_t now;

    SystemCoreClockUpdate();
    Motor_Init();
    Encoder_Init();
    Serial_Init();
    Control_Init(0);
    if (SysTick_Config(SystemCoreClock / 1000u) != 0)
        for (;;) { }

    for (;;) {
        now = milliseconds;
        Serial_Poll();
        /* Finish sampling before accepting a command that can reset the origin. */
        Control_Poll(now);
        Control_Command(Serial_Read(), now);
    }
}
