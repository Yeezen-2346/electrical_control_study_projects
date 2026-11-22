#include "Control.h"
#include "AppConfig.h"
#include "Encoder.h"
#include "Motor.h"
#include "PID.h"
#include "Serial.h"
#include <math.h>
#include <stdio.h>

typedef enum { IDLE, OPEN, STEP, SINE } Mode;

#define OPEN_DURATION_MS  2000u
#define STEP_HALF_MS      5000u
#define SINE_DURATION_MS  20000u

static const PID_Params tuning[] = {
    {0.007f, 0.0f,   0.0f,   DRIVE_LIMIT},
    {0.009f, 0.006f, 0.001f, DRIVE_LIMIT}
};

static Mode mode;
static PID pid;
static unsigned tuning_index;
static uint32_t started, last_control, last_log;
static float open_output;

static void Log(uint32_t elapsed, float target, float actual, float output)
{
    char *line = Serial_BeginLine();
    int length;
    if (!line)
        return;
    length = snprintf(line, SERIAL_LINE_CAPACITY,
                      "%lu,%.2f,%.2f,%lld,%d,%.3f,%u,%u\r\n",
                      (unsigned long)elapsed, target, actual,
                      (long long)Encoder_GetCounts(), Encoder_GetDirection(),
                      output, (unsigned)mode, tuning_index + 1u);
    if (length > 0)
        Serial_QueueLine((unsigned)length);
}

void Control_Init(uint32_t now)
{
    started = last_control = last_log = now;
    Motor_Stop();
}

void Control_Command(int command, uint32_t now)
{
    Mode next_mode = IDLE;
    float duty = 0.0f;

    switch (command) {
    case 'w': duty =  0.25f; break;
    case 'e': duty =  0.45f; break;
    case 's': duty = -0.25f; break;
    case 'd': duty = -0.45f; break;
    case 'p': next_mode = STEP; break;
    case 't': next_mode = SINE; break;
    case '1': tuning_index = 0; break;
    case '2': tuning_index = 1; break;
    case 'x': break;
    case 'z':
        if (mode == IDLE)
            Encoder_Zero();
        return;
    default: return;
    }

    Motor_Stop();
    mode = IDLE;
    if (duty != 0.0f)
        next_mode = OPEN;
    if ((next_mode == STEP || next_mode == SINE) &&
        ENCODER_COUNTS_PER_TURN <= 0.0f)
        return;

    if (next_mode == STEP || next_mode == SINE)
        Encoder_Zero();

    mode = next_mode;
    open_output = duty;
    started = last_control = last_log = now;
    PID_Reset(&pid, tuning[tuning_index], Encoder_GetPosition());
    if (mode == STEP)
        Log(0, 90.0f, Encoder_GetPosition(), 0.0f);
    else if (mode != IDLE)
        Log(0, 0.0f, Encoder_GetPosition(), 0.0f);
}

void Control_Poll(uint32_t now)
{
    uint32_t dt_ms = now - last_control;
    uint32_t elapsed = now - started;
    float actual, target = 0.0f, output = 0.0f;
    int finished = 0;

    if (dt_ms < CONTROL_MS)
        return;
    last_control = now;
    Encoder_Sample();
    actual = Encoder_GetPosition();
    if (dt_ms > 3u * CONTROL_MS) {
        Motor_Stop();
        mode = IDLE;
        return;
    }

    switch (mode) {
    case OPEN:
        finished = elapsed >= OPEN_DURATION_MS;
        if (!finished)
            output = open_output;
        break;
    case STEP:
        finished = elapsed >= 2u * STEP_HALF_MS;
        target = elapsed < STEP_HALF_MS ? 90.0f : -90.0f;
        break;
    case SINE:
        finished = elapsed >= SINE_DURATION_MS;
        if (!finished)
            target = 90.0f * sinf(1.25663706144f * (elapsed * 0.001f));
        break;
    case IDLE:
        break;
    }

    if (!finished && (mode == STEP || mode == SINE))
        output = PID_Update(&pid, target, actual, dt_ms * 0.001f);
    Motor_SetOutput(output);

    if (finished || now - last_log >= LOG_MS) {
        Log(elapsed, target, actual, output);
        last_log = now;
    }
    if (finished)
        mode = IDLE;
}
