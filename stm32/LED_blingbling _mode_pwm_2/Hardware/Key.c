#include "Key.h"
#include "Delay.h"

/* 硬件连接：KEY -> PB10（上拉输入，按下为低电平 -> 下降沿） */
#define KEY_GPIO_PORT       GPIOB
#define KEY_GPIO_PIN        GPIO_Pin_10

#define KEY_EXTI_LINE       EXTI_Line10
#define KEY_EXTI_PORTSOURCE GPIO_PortSourceGPIOB
#define KEY_EXTI_PINSOURCE  GPIO_PinSource10     /* 注意：是引脚编号 10，不是 GPIO_Pin_10 */
#define KEY_IRQn            EXTI15_10_IRQn       /* EXTI10 ~ EXTI15 共用这个中断通道 */

#define KEY_DEBOUNCE_MS     20u                  /* 消抖时间 */
#define KEY_RELEASE_MAX_MS  2000u                /* 等待松手的最长时间，防止一直按住时卡死 */

/* 按键事件标志：1 = 中断捕获到一次按下，等主循环确认后清除 */
static volatile uint8_t Key_Event = 0;

/**
  * @brief  按键外部中断初始化：PB10 -> EXTI10，下降沿触发
  */
void Key_EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 用 EXTI 必须打开 AFIO 时钟，漏了中断永远不来 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* 1. PB10：上拉输入 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = KEY_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStructure);

    /* 2. 把 PB10 映射到 EXTI10 */
    GPIO_EXTILineConfig(KEY_EXTI_PORTSOURCE, KEY_EXTI_PINSOURCE);

    /* 3. EXTI10：下降沿触发（按下瞬间为低电平） */
    EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 4. NVIC：使能 EXTI15_10 中断 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = KEY_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  按键中断服务函数：只记事件标志，不做延时
  * @note   中断里绝对不能调用 Delay_ms/Delay_us：
  *         延时靠 SysTick，而主循环的软件 PWM 也在用 SysTick，
  *         两者互相打断会让 Delay_us 永远等不到计数结束而死机。
  *         所以消抖、等待松手都放在主循环的 Key_GetEvent() 里做。
  */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(KEY_EXTI_LINE) != RESET)
    {
        EXTI_ClearITPendingBit(KEY_EXTI_LINE);      /* 先清标志，否则会反复进中断 */
        Key_Event = 1;                              /* 记录一次按键，交给主循环 */
    }
}

/**
  * @brief  查询按键事件（不清除标志）
  */
uint8_t Key_Pending(void)
{
    return Key_Event;
}

/**
  * @brief  取一次有效按键：消抖 + 等待松手（限时）
  * @retval 1 = 确实按了一下；0 = 只是抖动干扰，忽略
  */
uint8_t Key_GetEvent(void)
{
    uint16_t wait;

    if (Key_Event == 0)
    {
        return 0;
    }

    Key_Event = 0;                                  /* 先清掉，避免重复处理 */

    Delay_ms(KEY_DEBOUNCE_MS);                      /* 消抖 */
    if (GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_GPIO_PIN) != 0)
    {
        return 0;                                   /* 已经是高电平，说明是干扰，不算按键 */
    }

    /* 等待松手（限时 2s，防止一直按住时主循环卡住） */
    wait = 0;
    while ((GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_GPIO_PIN) == 0) && (wait < KEY_RELEASE_MAX_MS))
    {
        Delay_ms(1);
        wait++;
    }

    Key_Event = 0;                                  /* 清掉抖动过程中又置起来的标志 */

    return 1;
}
