#include "stm32f10x.h"
#include "board_config.h"
#include "board_a.h"
#include "board_b.h"

int main(void)
{
#if (BOARD_ROLE == BOARD_ROLE_A)
    BoardA_Init();
#else
    BoardB_Init();
#endif
    while (1)
    {
        __WFI();
    }
}
