#include "stm32f10x.h"
#include "LED.h"
#include "Key.h"

/* ============================================================================
 * 实验：LED 状态机（按键外部中断触发状态转移）
 *
 *   状态循环：熄灭 -> 常亮 -> 呼吸灯 -> 熄灭 -> 常亮 -> ......
 *   上电初始状态：熄灭
 *   状态转移触发：按键外部中断（PB10 -> EXTI10，下降沿触发）
 *
 * 硬件连接：
 *   LED1 = PA0（TIM2_CH1，硬件 PWM 调亮度）
 *   LED2 = PA4（普通 GPIO，软件 PWM 调亮度）
 *   LED3 = PA7（普通 GPIO，软件 PWM 调亮度）
 *   三个灯均为低电平点亮
 *   KEY  = PB10（上拉输入，按下为低电平）
 * ==========================================================================*/

/* ------------------------------- 状态定义 ------------------------------- */
typedef enum
{
    STATE_OFF = 0,          /* 熄灭（初始状态） */
    STATE_ON,               /* 常亮 */
    STATE_BREATH,           /* 呼吸灯 */
    STATE_NUM               /* 状态总数：用来做循环取模 */
} LedState_t;

/* 当前状态。中断里只置事件标志，状态变量只在主循环里改 */
static LedState_t g_LedState = STATE_OFF;

/* ------------------------------- 状态转移 ------------------------------- */
/**
  * @brief  状态转移：熄灭 -> 常亮 -> 呼吸灯 -> 熄灭 -> ...
  */
static void LedState_Next(void)
{
    g_LedState = (LedState_t)((g_LedState + 1) % STATE_NUM);
}

/* ------------------------------- 呼吸灯动作 ----------------------------- */
/**
  * @brief  呼吸灯：亮度 0 -> 100 渐亮，再 100 -> 0 渐暗，一次完整呼吸约 2s
  * @note   每变一次亮度（约 10ms）检查一次按键事件，
  *         一有按键马上返回主循环做状态转移，保证按键响应及时
  */
static void LedState_BreathRun(void)
{
    uint8_t duty;

    /* 渐亮：0 -> 100 */
    for (duty = 0; duty <= LED_DUTY_MAX; duty++)
    {
        LED_SetBrightness(duty);
        if (Key_Pending())
        {
            return;
        }
    }

    /* 渐暗：100 -> 0 */
    for (duty = LED_DUTY_MAX; duty > 0; duty--)
    {
        LED_SetBrightness((uint8_t)(duty - 1));
        if (Key_Pending())
        {
            return;
        }
    }
}

/* ------------------------------- 主函数 --------------------------------- */
int main(void)
{
    LED_Init();             /* LED：PA0 / PA4 / PA7 */
    PWM_Init();             /* PA0：TIM2_CH1 硬件 PWM */
    Key_EXTI_Init();        /* 按键：PB10 外部中断 */

    /* 上电进入初始状态：熄灭 */
    g_LedState = STATE_OFF;
    LED_AllOff();

    while (1)
    {
        /* 1. 外部中断捕获到按键 -> 转移到下一个状态 */
        if (Key_GetEvent())
        {
            LedState_Next();
        }

        /* 2. 执行当前状态对应的动作 */
        switch (g_LedState)
        {
            case STATE_OFF:                 /* 熄灭 */
                LED_AllOff();
                break;

            case STATE_ON:                  /* 常亮 */
                LED_AllOn();
                break;

            case STATE_BREATH:              /* 呼吸灯 */
                LedState_BreathRun();
                break;

            default:                        /* 兜底：异常状态回到熄灭 */
                g_LedState = STATE_OFF;
                LED_AllOff();
                break;
        }
    }
}
