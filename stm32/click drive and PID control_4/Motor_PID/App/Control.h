#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
void Control_Init(uint32_t now);
void Control_Command(int command, uint32_t now);
void Control_Poll(uint32_t now);

#endif
