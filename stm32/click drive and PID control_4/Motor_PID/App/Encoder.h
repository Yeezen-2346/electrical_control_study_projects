#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
void Encoder_Init(void);
void Encoder_Zero(void);
void Encoder_Sample(void);
int64_t Encoder_GetCounts(void);
int Encoder_GetDirection(void);
float Encoder_GetPosition(void);

#endif
