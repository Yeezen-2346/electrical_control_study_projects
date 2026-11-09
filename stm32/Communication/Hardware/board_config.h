#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 * Keil targets:
 *   Board_A -> defines BOARD_A
 *   Board_B -> defines BOARD_B
 *
 * If neither symbol is defined, board A is built by default. Defining both
 * is treated as a configuration error.
 */
#define BOARD_ROLE_A 1
#define BOARD_ROLE_B 2

#if defined(BOARD_A) && defined(BOARD_B)
#error "Define only one of BOARD_A and BOARD_B."
#elif defined(BOARD_B)
#define BOARD_ROLE BOARD_ROLE_B
#else
#define BOARD_ROLE BOARD_ROLE_A
#endif

/* Both MCUs run from the 72 MHz system clock configured by SystemInit. */
#define BOARD_CPU_CLOCK_HZ            72000000U

#endif /* BOARD_CONFIG_H */
