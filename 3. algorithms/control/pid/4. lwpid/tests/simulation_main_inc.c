/**
 * @file simulation_main_inc.c
 * @brief Incremental PID Simulation Runner
 * Verifies that the Incremental PID output, when accumulated, matches Positional PID.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../pid.h"

int main() {
    // 1. Setup Config
    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.5f,
                     .kd = 0.1f,
                     .kf = 0.0f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .deadband = 0.0f,
                     .d_lpf_alpha = 0.1f,
                     .derivative_on_measurement = true};

    // 2. Setup Two PIDs
    pid_t pid_pos;
    pid_t pid_inc;

    pid_init(&pid_pos, &cfg);
    pid_init(&pid_inc, &cfg);

    // 3. Simulation
    float setpoint = 10.0f;
    float measurement = 0.0f;  // Fixed PV to see open-loop response diff
    float dt = 0.1f;
    int steps = 50;

    // We act as the integrator for the Incremental PID
    float inc_accumulator = 0.0f;

    printf("Time,Positional_Out,Incremental_Accumulated,Diff\n");

    for (int i = 0; i < steps; i++) {
        float t = i * dt;

        // Vary PV to test dynamics
        if (i > 10)
            measurement = 5.0f;
        if (i > 30)
            measurement = 12.0f;

        // Run PIDs
        float out_pos = pid_update(&pid_pos, setpoint, measurement, dt);

        float out_inc_delta = pid_update_incremental(&pid_inc, setpoint, measurement, dt);
        inc_accumulator += out_inc_delta;

        // Sanity Check: Clamp accumulator to match positional limits
        if (inc_accumulator > cfg.out_max)
            inc_accumulator = cfg.out_max;
        if (inc_accumulator < cfg.out_min)
            inc_accumulator = cfg.out_min;

        printf("%.2f,%.4f,%.4f,%.4f\n", t, out_pos, inc_accumulator,
               fabsf(out_pos - inc_accumulator));
    }

    return 0;
}
