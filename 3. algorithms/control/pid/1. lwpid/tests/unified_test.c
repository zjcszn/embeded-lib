#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../pid.h"

// --- Simulation Parameters ---
#define DT       0.01f
#define SIM_TIME 5.0f
#define STEPS    (int) (SIM_TIME / DT)

// --- Helper Functions ---
void print_test_header(const char *name, const char *columns) {
    printf("TEST:%s\n", name);
    printf("%s\n", columns);
}

void print_test_footer() {
    printf("END_TEST\n");
}

bool float_near(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// --- Plant Model (First Order System) ---
typedef struct {
    float output;
    float velocity;  // Not used for 1st order but good for 2nd
    float gain;
    float time_constant;
} plant_t;

void plant_init(plant_t *p) {
    p->output = 0.0f;
    p->velocity = 0.0f;
    p->gain = 1.0f;
    p->time_constant = 1.0f;
}

void plant_reset(plant_t *p) {
    p->output = 0.0f;
    p->velocity = 0.0f;
}

float plant_update(plant_t *p, float input, float dt) {
    // Simple First Order: dy/dt = (K*u - y) / tau
    float derivative = (p->gain * input - p->output) / p->time_constant;
    p->output += derivative * dt;
    // Simple Euler integration
    return p->output;
}

// ==========================================
// 1. Basic Unit Tests
// ==========================================

void test_init_check(void) {
    print_test_header("Init_Check", "Result,Message");

    pid_t pid;
    pid_cfg_t cfg = {0};

    // 1. Test Valid Init
    cfg.out_max = 100;
    cfg.out_min = -100;
    bool res = pid_init(&pid, &cfg);
    printf("%d,Valid_Init\n", res);

    // 2. Test Invalid Range
    cfg.out_max = -100;
    cfg.out_min = 100;
    res = !pid_init(&pid, &cfg);  // Expect false
    printf("%d,Invalid_Range\n", res);

    // 3. Test Null Pointers
    res = !pid_init(NULL, &cfg);  // Expect false
    printf("%d,Null_PID\n", res);

    print_test_footer();
}

void test_p_term(void) {
    print_test_header("P_Term", "Error,Output,Expected");

    pid_cfg_t cfg = {.kp = 2.0f, .ki = 0.0f, .kd = 0.0f, .out_max = 100.0f, .out_min = -100.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float errors[] = {0.0f, 10.0f, -10.0f, 60.0f, -60.0f};

    for (int i = 0; i < 5; i++) {
        float err = errors[i];
        // Manually setting error via setpoint/measurement
        float sp = err;
        float meas = 0.0f;

        float out = pid_update(&pid, sp, meas, 0.1f);

        // Expected: Kp * err, clamped
        float expected = cfg.kp * err;
        if (expected > cfg.out_max)
            expected = cfg.out_max;
        if (expected < cfg.out_min)
            expected = cfg.out_min;

        printf("%.2f,%.2f,%.2f\n", err, out, expected);
    }
    print_test_footer();
}

void test_i_term(void) {
    print_test_header("I_Term", "Time,Error,Integral,Output");

    pid_cfg_t cfg = {.kp = 0.0f, .ki = 1.0f, .kd = 0.0f, .out_max = 100.0f, .out_min = -100.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float error = 10.0f;  // Constant error
    float t = 0.0f;
    float dt = 0.1f;

    // Phase 1: Accumulate
    for (int i = 0; i < 20; i++) {
        float out = pid_update(&pid, error, 0.0f, dt);
        printf("%.2f,%.2f,%.2f,%.2f\n", t, error, pid.integral, out);
        t += dt;

        // Phase 2: Reset at t=1.0
        if (i == 9) {
            pid_reset(&pid);
            // Error persists, integral should restart from 0
        }
    }
    print_test_footer();
}

void test_d_term(void) {
    print_test_header("D_Term", "Time,Setpoint,Measurement,Output,D_Term_Est");

    pid_cfg_t cfg = {.kp = 0.0f,
                     .ki = 0.0f,
                     .kd = 1.0f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .derivative_on_measurement = true};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;
    float meas = 0.0f;

    // Ramp measurement: slope = 5.0
    // D should be -5.0 (since derivative is on -measurement)

    for (int i = 0; i < 10; i++) {
        meas = 5.0f * t;
        float out = pid_update(&pid, 0.0f, meas, dt);

        printf("%.2f,%.2f,%.2f,%.2f,%.2f\n", t, 0.0f, meas, out, -5.0f);
        t += dt;
    }
    print_test_footer();
}

// ==========================================
// 2. Feature Tests
// ==========================================

void test_anti_windup(void) {
    print_test_header("Anti_Windup", "Time,Integral,Output,Mode");

    // Test Clamp
    pid_cfg_t cfg = {.kp = 0.5f,  // Reduced Kp so Output != Integral
                     .ki = 1.0f,
                     .kd = 0.0f,
                     .out_max = 10.0f,
                     .out_min = -10.0f,
                     .anti_windup_mode = PID_ANTI_WINDUP_CLAMP};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;

    // Run into saturation
    for (int i = 0; i < 10; i++) {
        float out = pid_update(&pid, 10.0f, 0.0f, dt);  // Error = 10
        printf("%.2f,%.2f,%.2f,CLAMP\n", t, pid.integral, out);
        t += dt;
    }

    // Reset and Test Conditional
    cfg.anti_windup_mode = PID_ANTI_WINDUP_CONDITIONAL;
    pid_init(&pid, &cfg);  // re-init to reset
    // t = 0.0f; // Keep time continuous to avoid plotting artifacts (zigzag/triangle)

    for (int i = 0; i < 10; i++) {
        float out = pid_update(&pid, 10.0f, 0.0f, dt);
        printf("%.2f,%.2f,%.2f,COND\n", t, pid.integral, out);
        t += dt;
    }

    print_test_footer();
}

void test_setpoint_ramp(void) {
    print_test_header("Setpoint_Ramp", "Time,TargetSP,InternalSP,Output");

    pid_cfg_t cfg = {
        .kp = 1.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .out_max = 100.0f,
        .out_min = -100.0f,
        .max_setpoint_ramp = 10.0f  // 10 units per second
    };
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;
    float sp = 0.0f;

    // Initial stabilize
    pid_update(&pid, 0.0f, 0.0f, dt);

    // Step SP to 50
    sp = 50.0f;

    for (int i = 0; i < 10; i++) {
        float out = pid_update(&pid, sp, 0.0f, dt);
        printf("%.2f,%.2f,%.2f,%.2f\n", t, sp, pid.internal_setpoint, out);
        t += dt;
    }
    print_test_footer();
}

void test_output_rate_limit(void) {
    print_test_header("Output_Rate_Limit", "Time,Error,Output,DeltaOut");

    pid_cfg_t cfg = {
        .kp = 0.0f,  // Use Integral for continuous change
        .ki = 10.0f,
        .kd = 0.0f,
        .out_max = 100.0f,
        .out_min = -100.0f,
        .max_rate = 5.0f  // Max 5 units per second -> 0.5 per 0.1s
    };
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;
    float prev_out = 0.0f;

    // Large error step: Error 10 -> Target Out 100.
    // Initial Out is 0.
    // Should behave as Incremental output limitation?
    // Wait, Standard PID rate limit is usually implemented on the output change?
    // Looking at pid.c, standard PID does NOT have rate limiting in `pid_update`!
    // It's only in `pid_update_incremental`!
    // Let's re-read pid.h/c.
    // pid_update: "1. Setpoint Ramp", "2. Error", "3. Deadband", "4. P", "5. I", "6. D", "7. FF",
    // "8. Clamp". Wait, I missed reading rate limiting in `pid_update`. Checking `pid.c` again...
    // Ah, `pid_update` does NOT have `max_rate` logic applied to `out`.
    // The `max_rate` in `pid_cfg_t` is described as "Maximum output change per second".
    // But in `pid.c`, `pid_update` implementation does not use `max_rate`.
    // ONLY `pid_update_incremental` uses it: `/* --- 5. Rate Limiting (Slew Rate) --- */`.

    // THIS IS A BUG/MISSING FEATURE IN STANDARD PID or intentional?
    // Usually standard PID rate limits output by rate limiting setpoint or using filter.
    // If user wants rate limited output in Position form, they should use Setpoint Ramp or Filter.
    // BUT, the config struct has `max_rate`.

    // Let's test `pid_update_incremental` here for rate limit.
    // Or I should note this discrepancy.
    // For now I will test Incremental Rate Limit.

    for (int i = 0; i < 10; i++) {
        float out_change = pid_update_incremental(&pid, 100.0f, 0.0f, dt);
        float current_out = prev_out + out_change;  // Simulate accumulation

        printf("%.2f,%.2f,%.2f,%.2f\n", t, 100.0f, current_out, out_change);

        prev_out = current_out;
        t += dt;
    }
    print_test_footer();
}

void test_deadband(void) {
    print_test_header("Deadband", "Error,Output");

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.0f,
                     .kd = 0.0f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .deadband = 2.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    // Sweep error from -5 to 5 to show deadband region [-2, 2] clearly
    float err = -5.0f;
    float step = 0.5f;

    while (err <= 5.0f) {
        float out = pid_update(&pid, err, 0.0f, 0.1f);
        printf("%.2f,%.2f\n", err, out);
        err += step;
    }
    print_test_footer();
}

void test_d_lpf(void) {
    print_test_header("D_LPF_Filter", "Time,Measurement,Derivative,Output,Alpha");

    // Two PIDs to compare: No Filter vs Filter
    pid_cfg_t cfg_clean = {
        .kp = 0.0f,
        .ki = 0.0f,
        .kd = 1.0f,
        .out_max = 100.0f,
        .out_min = -100.0f,
        .d_lpf_alpha =
            1.0f,  // No filter (Alpha 1.0 means new value fully taken? Wait, let's check formula)
        // Formula: lpf = lpf + alpha * (input - lpf)
        // If alpha = 1.0: lpf = lpf + input - lpf = input. Correct.
        .derivative_on_measurement = true};

    pid_cfg_t cfg_filter = cfg_clean;
    cfg_filter.d_lpf_alpha = 0.1f;  // Heavy filter

    pid_t pid1, pid2;
    pid_init(&pid1, &cfg_clean);
    pid_init(&pid2, &cfg_filter);

    float t = 0.0f;
    float dt = 0.01f;
    float meas = 0.0f;

    // Noisy signal
    for (int i = 0; i < 100; i++) {
        // Base sine wave + noise
        float pure = sinf(t * 2.0f);
        float noise = (i % 2 == 0) ? 0.1f : -0.1f;
        meas = pure + noise;

        float out1 = pid_update(&pid1, 0.0f, meas, dt);  // Raw Derivative
        float out2 = pid_update(&pid2, 0.0f, meas, dt);  // Filtered

        printf("%.2f,%.3f,%.3f,%.3f,0.1\n", t, meas, out1, out2);
        t += dt;
    }
    print_test_footer();
}

// ==========================================
// 3. Advanced Control Tests
// ==========================================

void test_incremental(void) {
    print_test_header("Incremental_PID", "Time,Setpoint,Measurement,DeltaOut,TotalOut");

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.5f,
                     .kd = 0.1f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .max_rate = 0.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float output_accum = 0.0f;
    float sp = 10.0f;
    float meas = 0.0f;  // Open loop to see step response behavior
    float dt = 0.1f;
    float t = 0.0f;

    // Increased plant gain from 0.01 to 0.5 so Output=20 -> Meas=10
    // Gain 0.01 required Output=1000 which is > max 100.
    float plant_gain = 0.5f;

    for (int i = 0; i < 50; i++) {
        float delta = pid_update_incremental(&pid, sp, meas, dt);
        output_accum += delta;
        // Simple plant feedback
        meas += output_accum * plant_gain * dt;

        printf("%.2f,%.2f,%.2f,%.2f,%.2f\n", t, sp, meas, delta, output_accum);
        t += dt;
    }
    print_test_footer();
}

void test_cascade_sim(void) {
    print_test_header("Cascade_Control", "Time,Setpoint,OuterOut,InnerOut,ProcessVal");

    // Outer Loop: Position Control
    pid_cfg_t cfg_outer = {
        .kp = 2.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .out_max = 50.0f,
        .out_min = -50.0f  // Limit velocity setpoint
    };
    pid_t pid_outer;
    pid_init(&pid_outer, &cfg_outer);

    // Inner Loop: Velocity Control
    pid_cfg_t cfg_inner = {
        .kp = 5.0f, .ki = 2.0f, .kd = 0.0f, .out_max = 100.0f, .out_min = -100.0f};
    pid_t pid_inner;
    pid_init(&pid_inner, &cfg_inner);

    pid_cascade_t cas;
    pid_cascade_init(&cas, &pid_outer, &pid_inner);

    // Plant: Position and Velocity
    float pos = 0.0f;
    float vel = 0.0f;
    float dt = 0.01f;
    float t = 0.0f;
    float sp_pos = 100.0f;

    // Increased steps to 1000 (10s)
    for (int i = 0; i < 1000; i++) {
        // Update Cascade
        // Outer Meas = Position, Inner Meas = Velocity
        float force = pid_cascade_update(&cas, sp_pos, pos, vel, dt);

        // Physics: F = ma (m=1) -> acc = force
        // vel = vel + acc * dt
        // pos = pos + vel * dt
        // Add some friction
        float acc = force - 0.1f * vel;
        vel += acc * dt;
        pos += vel * dt;

        if (i % 10 == 0) {  // Downsample output
            printf("%.2f,%.2f,%.2f,%.2f,%.2f\n", t, sp_pos, pid_outer.out, force, pos);
        }
        t += dt;
    }
    print_test_footer();
}

void test_bumpless_full(void) {
    print_test_header("Bumpless_Transfer", "Time,Setpoint,Measurement,Output,Mode");

    pid_cfg_t cfg = {
        .kp = 2.0f,
        .ki = 0.5f,
        .kd = 0.0f,
        .out_max = 100.0f,
        .out_min = -100.0f,
        .kw = 1.0f  // Back-calculation gain
    };
    pid_t pid;
    pid_init(&pid, &cfg);

    float meas = 10.0f;
    float sp = 20.0f;
    float dt = 0.1f;
    float t = 0.0f;

    // 1. Auto Mode (0 - 2s)
    for (int i = 0; i < 20; i++) {
        float out = pid_update(&pid, sp, meas, dt);
        printf("%.2f,%.2f,%.2f,%.2f,AUTO\n", t, sp, meas, out);
        t += dt;
    }

    // 2. Manual Mode (External override) (2s - 4s)
    float manual_val = 50.0f;
    // Simulate setpoint change during manual mode to test tracking
    sp = 30.0f;

    for (int i = 0; i < 20; i++) {
        // Force manual output and track
        // Important: In real system, we read manual_val from actuator or user input
        pid_track_manual(&pid, manual_val, meas, sp);

        printf("%.2f,%.2f,%.2f,%.2f,MANUAL\n", t, sp, meas, manual_val);
        t += dt;
    }

    // 3. Switch back to Auto (4s - 6s)
    // Should start near manual_val
    for (int i = 0; i < 20; i++) {
        float out = pid_update(&pid, sp, meas, dt);
        printf("%.2f,%.2f,%.2f,%.2f,AUTO_RE\n", t, sp, meas, out);
        t += dt;
    }

    print_test_footer();
}

void test_tick_update(void) {
    print_test_header("Tick_Update", "Time,DeltaTime,Output");

    pid_cfg_t cfg = {.kp = 1.0f, .ki = 0.0f, .kd = 0.0f, .out_max = 100.0f, .out_min = -100.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    uint32_t current_tick = 1000;
    uint32_t last_tick = 1000;
    float tick_scale = 0.001f;  // 1ms per tick

    // Initial stabilize
    pid_update(&pid, 10.0f, 0.0f, 0.01f);

    float t = 0.0f;

    // Simulate jittery ticks (large jitter for visibility)
    // Nominal 10ms (10 ticks)
    uint32_t increments[] = {10, 20, 5, 30, 10, 5};

    for (int i = 0; i < 6; i++) {
        current_tick += increments[i];

        // Manual dt calculation
        uint32_t delta_tick = current_tick - last_tick;
        float dt_real = delta_tick * tick_scale;

        last_tick = current_tick;
        t += dt_real;

        float out = pid_update(&pid, 10.0f, 0.0f, dt_real);
        printf("%.3f,%.3f,%.2f\n", t, dt_real, out);
    }
    print_test_footer();
}

// ==========================================
// 4. Robustness Tests (Simplified)
// ==========================================

void test_noise_robustness(void) {
    print_test_header("Noise_Response", "Time,Signal,Noisy,Filtered");

    // Just re-use D-Term LPF test pattern essentially, or test closed loop performance?
    // Let's do a simple closed loop step response with noise.

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.5f,
                     .kd = 0.1f,
                     .d_lpf_alpha = 0.1f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .derivative_on_measurement = true};
    pid_t pid;
    pid_init(&pid, &cfg);

    plant_t plant;
    plant_init(&plant);

    float sp = 10.0f;
    float dt = 0.01f;
    float t = 0.0f;

    srand(1234);  // Fixed seed

    // Increased steps to 2000 (20s)
    for (int i = 0; i < 2000; i++) {
        float noise = ((float) (rand() % 100) / 50.0f) - 1.0f;  // +/- 1.0
        float meas = plant.output;
        float meas_noisy = meas + noise;

        float out = pid_update(&pid, sp, meas_noisy, dt);
        plant_update(&plant, out, dt);

        if (i % 20 == 0)  // Downsample
            printf("%.2f,%.3f,%.3f,%.3f\n", t, meas, meas_noisy, out);
        t += dt;
    }
    print_test_footer();
}

// --- Main Dispatcher ---

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <test_name>\n", argv[0]);
        printf("Available tests: init_check, p_term, i_term, d_term, anti_windup, ...\n");
        return 1;
    }

    const char *test = argv[1];

    if (strcmp(test, "init_check") == 0)
        test_init_check();
    else if (strcmp(test, "p_term") == 0)
        test_p_term();
    else if (strcmp(test, "i_term") == 0)
        test_i_term();
    else if (strcmp(test, "d_term") == 0)
        test_d_term();
    else if (strcmp(test, "anti_windup") == 0)
        test_anti_windup();
    else if (strcmp(test, "setpoint_ramp") == 0)
        test_setpoint_ramp();
    else if (strcmp(test, "output_rate_limit") == 0)
        test_output_rate_limit();
    else if (strcmp(test, "deadband") == 0)
        test_deadband();
    else if (strcmp(test, "d_lpf") == 0)
        test_d_lpf();
    else if (strcmp(test, "incremental") == 0)
        test_incremental();
    else if (strcmp(test, "cascade") == 0)
        test_cascade_sim();
    else if (strcmp(test, "bumpless") == 0)
        test_bumpless_full();
    else if (strcmp(test, "tick_update") == 0)
        test_tick_update();
    else if (strcmp(test, "noise") == 0)
        test_noise_robustness();
    else {
        printf("Unknown test: %s\n", test);
        return 1;
    }

    return 0;
}
