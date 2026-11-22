#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define CONTROL_MS  10u
#define LOG_MS      20u
#define PWM_HZ      20000u
#define UART_BAUD   115200u

/* MG310P20 Hall encoder: 13 cycles * 20.409 gear ratio * 4 edges. */
#define ENCODER_COUNTS_PER_TURN  1061.268f
#define ENCODER_SIGN             (-1)
#define MOTOR_SIGN               1
#define DRIVE_LIMIT              0.55f

#endif
