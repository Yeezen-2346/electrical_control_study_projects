#include "PID.h"

static float Clamp(float value, float limit)
{
    if (value > limit)
        return limit;
    if (value < -limit)
        return -limit;
    return value;
}

void PID_Reset(PID *pid, PID_Params params, float position)
{
    pid->params = params;
    pid->integral = 0.0f;
    pid->previous_position = position;
    pid->filtered_velocity = 0.0f;
}

float PID_Update(PID *pid, float target, float position, float dt)
{
    const PID_Params *p = &pid->params;
    float error = target - position;
    float velocity = (position - pid->previous_position) / dt;
    float increment = p->ki * error * dt;
    float candidate = Clamp(pid->integral + increment, p->limit);
    float pd, raw;

    pid->previous_position = position;
    pid->filtered_velocity += dt / (0.04f + dt) *
                             (velocity - pid->filtered_velocity);
    pd = p->kp * error - p->kd * pid->filtered_velocity;
    raw = pd + candidate;

    if (!((raw > p->limit && increment > 0.0f) ||
          (raw < -p->limit && increment < 0.0f)))
        pid->integral = candidate;
    return Clamp(pd + pid->integral, p->limit);
}
