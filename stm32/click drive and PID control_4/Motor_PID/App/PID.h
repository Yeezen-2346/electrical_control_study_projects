#ifndef PID_H
#define PID_H

typedef struct {
    float kp, ki, kd;
    float limit;
} PID_Params;

typedef struct {
    PID_Params params;
    float integral;
    float previous_position;
    float filtered_velocity;
} PID;

void PID_Reset(PID *pid, PID_Params params, float position);
float PID_Update(PID *pid, float target, float position, float dt);

#endif
