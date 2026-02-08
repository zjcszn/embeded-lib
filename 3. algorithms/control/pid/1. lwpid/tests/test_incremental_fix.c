#include <math.h>
#include <stdio.h>

#include "pid.h"


// Simple assertion macro
#define TEST_ASSERT(cond, msg)     \
    if (!(cond)) {                 \
        printf("FAIL: %s\n", msg); \
        return 1;                  \
    } else {                       \
        printf("PASS: %s\n", msg); \
    }

int main() {
    printf("Testing Incremental PID Filter Consistency Fix...\n");

    pid_t pid;
    pid_cfg_t cfg = {0};

    // Configure PID: Kp=1, Input Filter Tau=1.0s (Strong)
    cfg.kp = 1.0f;
    cfg.ki = 0.0f;
    cfg.kd = 0.0f;
    cfg.kf = 0.0f;
    cfg.beta = 1.0f;
    cfg.gamma = 0.0f;  // Derivative on Measurement (unused here since Kd=0)

    cfg.out_max = 1000.0f;
    cfg.out_min = -1000.0f;

    // Input Filter Setup
    cfg.input_filter_type = PID_FILTER_PT1;
    cfg.input_filter_tau = 1.0f;  // 1 second time constant

    // Initialize
    if (!pid_init(&pid, &cfg)) {
        printf("FAIL: PID Init failed\n");
        return 1;
    }

    float dt = 0.1f;  // 100ms
    float setpoint = 10.0f;
    float current_output = 0.0f;

    // Step 1: Steady State (SP=10, Meas=10)
    // Filter initializes to 10.
    float meas = 10.0f;
    float out_change = pid_update_incremental(&pid, setpoint, meas, dt, current_output);
    current_output += out_change;

    printf("T=0.0: Meas=%.1f, dOut=%.4f\n", meas, out_change);

    // Step 2: Huge Noise Spike (Meas=100)
    // Filter Update: 10 + (0.1/1.1)*(100-10) = 10 + 0.0909*90 = 18.18
    // P_curr (+beta) = 1 * (10 - 18.18) = -8.18
    // P_prev = 1 * (10 - 10) = 0
    // dP = -8.18
    meas = 100.0f;
    out_change = pid_update_incremental(&pid, setpoint, meas, dt, current_output);
    current_output += out_change;

    printf("T=0.1: Meas=%.1f (Noise), dOut=%.4f\n", meas, out_change);

    // Step 3: Noise Gone (Meas=10)
    // Filter Update: 18.18 + 0.0909*(10 - 18.18) = 18.18 - 0.74 = 17.44
    // P_curr = 1 * (10 - 17.44) = -7.44
    // P_prev (FIXED) = 1 * (10 - 18.18) = -8.18  => dP = -7.44 - (-8.18) = +0.74 (Small recoil)
    // P_prev (BUGGY) = 1 * (10 - 100) = -90      => dP = -7.44 - (-90) = +82.56 (HUGE KICK)
    meas = 10.0f;
    out_change = pid_update_incremental(&pid, setpoint, meas, dt, current_output);
    current_output += out_change;

    printf("T=0.2: Meas=%.1f (Clean), dOut=%.4f\n", meas, out_change);

    // Check if dOut is reasonable (Small recoil < 2.0)
    if (fabsf(out_change) < 2.0f) {
        printf("PASS: Incremental PID handled noise smoothly (dOut = %.4f)\n", out_change);
        return 0;
    } else {
        printf("FAIL: Large output kick detected (dOut = %.4f). Bug likely present.\n", out_change);
        return 1;
    }
}
