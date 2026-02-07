/**
 * @file simulation_main.c
 * @brief PID Simulation Runner
 * Simulate a FOPDT Plant (First Order Plus Dead Time) process.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../pid.h"


// --- Plant Simulation (Tustin Discretization) ---
// Plant: G(s) = K / (tau*s + 1)
// Discretized: y[k] = (dt/(tau+dt))*K*u[k] + (tau/(tau+dt))*y[k-1]
// Adding some noise and load disturbance

typedef struct {
    float y_prev;
    float tau;  // Time constant
    float K;    // Gain
    float dt;   // Sample time
} plant_t;

void plant_init(plant_t *p, float tau, float K, float dt) {
    p->y_prev = 0.0f;
    p->tau = tau;
    p->K = K;
    p->dt = dt;
}

float plant_update(plant_t *p, float u) {
    float alpha = p->dt / (p->tau + p->dt);
    float y = alpha * p->K * u + (1.0f - alpha) * p->y_prev;
    p->y_prev = y;
    return y;
}

// --- Main Simulation Loop ---

int main() {
    // 1. Setup PID
    pid_cfg_t cfg = {.kp = 2.0f,
                     .ki = 1.5f,
                     .kd = 0.5f,
                     .kf = 0.0f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .deadband = 0.5f,
                     .d_lpf_alpha = 0.1f,  // ~RC filter
                     .derivative_on_measurement = true};

    pid_t pid;
    if (!pid_init(&pid, &cfg)) {
        printf("PID Init failed!\n");
        return -1;
    }

    // 2. Setup Plant (Heater Model: Gain=5, Tau=2.0s)
    plant_t plant;
    plant_init(&plant, 2.0f, 5.0f, 0.01f);  // 10ms sampling

    // 3. Simulation Parameters
    float total_time = 10.0f;  // seconds
    float dt = 0.01f;          // 10ms
    int steps = (int) (total_time / dt);

    float setpoint = 0.0f;
    float measurement = 0.0f;
    float control_signal = 0.0f;
    float load_disturbance = 0.0f;

    // Header for CSV
    printf("Time,Setpoint,Measurement,Output,P_term,I_term,D_term\n");

    for (int i = 0; i < steps; i++) {
        float t = i * dt;

        // Scenario: Step Change at t=1.0s
        if (t >= 1.0f) {
            setpoint = 50.0f;  // Target temperature 50C
        }

        // Scenario: Load Disturbance (e.g. open window) at t=6.0s
        if (t >= 6.0f && t < 8.0f) {
            load_disturbance = -20.0f;  // Cooling effect
        } else {
            load_disturbance = 0.0f;
        }

        // Run PID
        control_signal = pid_update(&pid, setpoint, measurement, dt);

        // Apply to Plant (Output + Disturbance)
        // Note: Real plants have limits too, but PID handles its own output limits.
        // We simulate the physical effect of the output on the plant.
        float effective_input = control_signal + load_disturbance;

        // Update Plant
        measurement = plant_update(&plant, effective_input);

        // Add some random sensor noise (+/- 0.2)
        // measurement += ((rand() % 100) / 100.0f * 0.4f) - 0.2f;

        // Output CSV
        // For debugging internal terms, we'd need to access pid struct,
        // but let's stick to black box testing mostly unless we expose internals.
        // pid.integral is exposed in our struct, so we can print it.
        // P/D terms are local vars in update, so we can't easily print them without modifying pid.c
        // or re-calculating them. Let's just print I-term since it's in state.
        printf("%.3f,%.2f,%.2f,%.2f,%.2f\n", t, setpoint, measurement, control_signal,
               pid.integral);
    }

    return 0;
}
