#ifndef SERIAL_H
#define SERIAL_H

void Serial_Init(void);
int Serial_Read(void);
void Serial_Poll(void);
char *Serial_BeginLine(void);
void Serial_QueueLine(unsigned length);

#define SERIAL_LINE_CAPACITY  112u

#endif
