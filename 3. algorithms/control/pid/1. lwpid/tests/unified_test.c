#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../pid.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

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
    res = !pid_init(&pid, &cfg);
    printf("%d,Invalid_Range\n", res);

    // 3. Test Null Pointers
    res = !pid_init(NULL, &cfg);
    printf("%d,Null_PID\n", res);

    // 4. Test NaN (Input validation)
    cfg.out_max = 100;
    cfg.out_min = -100;
    cfg.kp = NAN;
    res = pid_init(&pid, &cfg);
    printf("%d,NaN_Check_Init\n", !res);

    print_test_footer();
}

void test_p_term(void) {
    print_test_header("P_Term", "Error,Output,Expected");

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.0f,
                     .kd = 0.1f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .d_tau = 0.0f,
                     .beta = 1.0f,
                     .gamma = 0.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float errors[] = {0.0f, 10.0f, -10.0f, 60.0f, -60.0f};

    for (int i = 0; i < 5; i++) {
        float err = errors[i];
        float sp = err;
        float meas = 0.0f;

        float out = pid_update(&pid, sp, meas, 0.1f);

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

    float error = 10.0f;
    float t = 0.0f;
    float dt = 0.1f;

    for (int i = 0; i < 20; i++) {
        float out = pid_update(&pid, error, 0.0f, dt);
        printf("%.2f,%.2f,%.2f,%.2f\n", t, error, pid.integral, out);
        t += dt;

        if (i == 9)
            pid_reset(&pid);
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
                     .beta = 1.0f,
                     .gamma = 0.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;
    float meas = 0.0f;

    // Ramp measurement: slope = 5.0
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

    pid_cfg_t cfg = {.kp = 0.0f,
                     .ki = 1.0f,
                     .kd = 0.0f,
                     .out_max = 10.0f,
                     .out_min = -10.0f,
                     .beta = 1.0f,
                     .gamma = 1.0f,
                     .anti_windup_mode = PID_ANTI_WINDUP_CLAMP};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;
    float error = 100.0f;

    // Static Clamp
    for (int i = 0; i < 10; i++) {
        float out = pid_update(&pid, error, 0.0f, dt);
        printf("%.2f,%.2f,%.2f,CLAMP\n", t, pid.integral, out);
        t += dt;
    }

    // Dynamic Clamp
    cfg.anti_windup_mode = PID_ANTI_WINDUP_DYNAMIC_CLAMP;
    cfg.kp = 5.0f;

    pid_reset(&pid);
    pid_init(&pid, &cfg);

    for (int i = 0; i < 10; i++) {
        float out = pid_update(&pid, error, 0.0f, dt);
        printf("%.2f,%.2f,%.2f,DYN_CLAMP\n", t, pid.integral, out);
        t += dt;
    }
    print_test_footer();
}

void test_setpoint_ramp(void) {
    print_test_header("Setpoint_Ramp", "Time,TargetSP,InternalSP,Output");

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.0f,
                     .kd = 0.0f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .beta = 1.0f,
                     .max_setpoint_ramp = 10.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;
    float sp = 0.0f;

    pid_update(&pid, 0.0f, 0.0f, dt);

    sp = 50.0f;
    for (int i = 0; i < 10; i++) {
        float out = pid_update(&pid, sp, 0.0f, dt);
        printf("%.2f,%.2f,%.2f,%.2f\n", t, sp, pid.internal_setpoint, out);
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
                     .beta = 1.0f,
                     .deadband = 2.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float err = -5.0f;
    while (err <= 5.0f) {
        float out = pid_update(&pid, err, 0.0f, 0.1f);
        printf("%.2f,%.2f\n", err, out);
        err += 0.5f;
    }
    print_test_footer();
}

void test_d_lpf(void) {
    print_test_header("D_LPF_Robustness", "Time,RawDeriv,FilteredTau,FilteredAlpha,Dt");

    pid_cfg_t cfg_tau = {.kp = 0.0f,
                         .ki = 0.0f,
                         .kd = 1.0f,
                         .out_max = 100.0f,
                         .out_min = -100.0f,
                         .d_tau = 0.1f,
                         .beta = 1.0f,
                         .gamma = 0.0f};

    pid_cfg_t cfg_alpha = cfg_tau;
    cfg_alpha.d_tau = 0.0f;
    cfg_alpha.d_lpf_alpha = 0.1f;

    pid_t pid_tau, pid_alpha;
    pid_init(&pid_tau, &cfg_tau);
    pid_init(&pid_alpha, &cfg_alpha);

    float t = 0.0f;

    // Use a higher frequency signal (5Hz) to be in the stopband of the filter (fc=1.6Hz)
    // This makes the attenuation sensitive to the actual cutoff frequency.
    float signal_freq = 5.0f;
    float omega = 2.0f * M_PI * signal_freq;

    // Run for longer to see effects
    for (int i = 0; i < 200; i++) {
        float dt = (i / 20 % 2 == 0) ? 0.01f : 0.05f;  // Switch dt every 20 steps
        float input = sinf(omega * t);

        float out1 = pid_update(&pid_tau, input, 0.0f, dt);    // Low Pass (Robust)
        float out2 = pid_update(&pid_alpha, input, 0.0f, dt);  // Fixed Alpha (Sensitive to dt)

        printf("%.3f,%.2f,%.3f,%.3f,%.3f\n", t, input, out1, out2, dt);
        t += dt;
    }
    print_test_footer();
}

void test_biquad(void) {
    print_test_header("Biquad_Filter", "Time,Freq,Output_Bypass,Output_Notch");

    pid_cfg_t cfg_bypass = {.kp = 0.0f,
                            .kd = 1.0f,
                            .out_max = 10000.0f,
                            .out_min = -10000.0f,
                            .beta = 1.0f,
                            .gamma = 0.0f};
    // No filter or very high cutoff LPF acting as bypass
    cfg_bypass.d_filter_type = PID_FILTER_NONE;

    pid_cfg_t cfg_notch = {.kp = 0.0f,
                           .kd = 1.0f,
                           .out_max = 10000.0f,
                           .out_min = -10000.0f,
                           .d_filter_type = PID_FILTER_BIQUAD,
                           .beta = 1.0f,
                           .gamma = 0.0f};  // Kick-free
    pid_filter_calc_notch(&cfg_notch.d_biquad_coeffs, 0.001f, 50.0f, 10.0f);

    pid_t pid_bypass, pid_notch;
    pid_init(&pid_bypass, &cfg_bypass);
    pid_init(&pid_notch, &cfg_notch);

    float dt = 0.001f;
    float t = 0.0f;
    float total_time = 2.0f;
    float start_freq = 10.0f;
    float end_freq = 100.0f;

    // Chirp sweep
    int steps = (int) (total_time / dt);
    for (int i = 0; i < steps; i++) {
        // Instantaneous frequency: f(t) = f0 + (f1-f0)*t/T
        // Phase phi(t) = integral(2*pi*f(t)) = 2*pi*(f0*t + (f1-f0)/(2*T)*t^2)
        float current_freq = start_freq + (end_freq - start_freq) * t / total_time;
        float phi =
            2.0f * M_PI * (start_freq * t + (end_freq - start_freq) / (2.0f * total_time) * t * t);

        float signal = sinf(phi);

        float out1 = pid_update(&pid_bypass, signal, 0.0f, dt);
        float out2 = pid_update(&pid_notch, signal, 0.0f, dt);

        // Subsample for plotting
        if (i % 10 == 0) {
            printf("%.3f,%.1f,%.3f,%.3f\n", t, current_freq, out1, out2);
        }
        t += dt;
    }
    print_test_footer();
}

// 3. New Tests for Missing Coverage

void test_rate_limit(void) {
    print_test_header("Output_Rate_Limit", "Time,Error,Output,DeltaOut");

    pid_cfg_t cfg = {.kp = 0.0f,
                     .ki = 1.0f,
                     .kd = 0.0f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .beta = 1.0f,
                     .max_rate = 50.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    // Use Incremental for rate limit test as per implementation
    pid_reset(&pid);

    float t = 0.0f;
    float dt = 0.1f;
    float sp = 100.0f;
    float meas = 0.0f;
    float total_out = 0.0f;

    // Run longer to see ramping
    for (int i = 0; i < 20; i++) {
        float inc = pid_update_incremental(&pid, sp, meas, dt, total_out);
        total_out += inc;
        printf("%.2f,%.2f,%.2f,%.2f\n", t, sp - meas, total_out, inc);
        t += dt;
    }
    print_test_footer();
}

void test_incremental(void) {
    print_test_header("Incremental_Convergence", "Time,Setpoint,Measurement,TotalOut,DeltaOut");

    pid_cfg_t cfg = {.kp = 0.5f, .ki = 0.5f, .out_max = 100.0f, .out_min = -100.0f, .beta = 1.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    plant_t plant;
    plant_init(&plant);

    float t = 0.0f;
    float dt = 0.1f;
    float sp = 10.0f;
    float meas = 0.0f;
    float u = 0.0f;

    // Increased to 300 steps (30s) for convergence
    for (int i = 0; i < 300; i++) {
        float inc = pid_update_incremental(&pid, sp, meas, dt, u);
        u += inc;
        if (u > 100)
            u = 100;
        if (u < -100)
            u = -100;
        meas = plant_update(&plant, u, dt);

        if (i % 10 == 0)
            printf("%.2f,%.2f,%.2f,%.2f,%.2f\n", t, sp, meas, u, inc);
        t += dt;
    }
    print_test_footer();
}

void test_cascade(void) {
    print_test_header("Cascade_Control", "Time,Setpoint,ProcessVal,OuterOut,InnerOut");

    pid_cfg_t cfg_out = {.kp = 2.0f, .out_max = 20.0f, .out_min = -20.0f, .beta = 1.0f};
    pid_t pid_out;
    pid_init(&pid_out, &cfg_out);

    pid_cfg_t cfg_in = {
        .kp = 1.0f, .ki = 0.5f, .out_max = 100.0f, .out_min = -100.0f, .beta = 1.0f};
    pid_t pid_in;
    pid_init(&pid_in, &cfg_in);

    pid_cascade_t cascade;
    pid_cascade_init(&cascade, &pid_out, &pid_in);

    float pos = 0.0f;
    float vel = 0.0f;
    float t = 0.0f;
    float dt = 0.05f;
    float sp = 10.0f;

    // Increased to 300 steps (15s) for convergence
    for (int i = 0; i < 300; i++) {
        float force = pid_cascade_update(&cascade, sp, pos, vel, dt);
        float accel = force;
        vel += accel * dt;
        pos += vel * dt;

        if (i % 10 == 0)
            printf("%.2f,%.2f,%.2f,%.2f,%.2f\n", t, sp, pos, pid_out.out, force);
        t += dt;
    }
    print_test_footer();
}

void test_bumpless(void) {
    print_test_header("Bumpless_Manual", "Time,Setpoint,Measurement,Output,Mode");

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.5f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .beta = 1.0f,
                     .anti_windup_mode = PID_ANTI_WINDUP_CLAMP};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.1f;
    float sp = 10.0f;
    float meas = 0.0f;
    float man_val = 50.0f;
    char *mode = "AUTO";

    for (int i = 0; i < 60; i++) {
        float out;
        if (t >= 2.0f && t < 4.0f) {
            mode = "MANUAL";
            pid_track_manual(&pid, man_val, meas, sp);
            out = man_val;
        } else {
            if (t >= 4.0f)
                mode = "AUTO_RE";
            else
                mode = "AUTO";
            out = pid_update(&pid, sp, meas, dt);
            if (out > meas)
                meas += 0.5f;
        }
        printf("%.2f,%.2f,%.2f,%.2f,%s\n", t, sp, meas, out, mode);
        t += dt;
    }
    print_test_footer();
}

void test_noise(void) {
    print_test_header("Noise_Robustness", "Time,Signal,Noisy,Filtered");

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.0f,
                     .kd = 0.1f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .d_tau = 0.0f,
                     .gamma = 0.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;
    float dt = 0.05f;
    float sp = 10.0f;
    float meas = 0.0f;

    for (int i = 0; i < 100; i++) {
        meas += (sp - meas) * 0.1f;
        float noise = ((float) (rand() % 200) / 100.0f) - 1.0f;
        float meas_noisy = meas + noise;
        float out = pid_update(&pid, sp, meas_noisy, dt);
        printf("%.2f,%.2f,%.2f,%.2f\n", t, meas, meas_noisy, out);
        t += dt;
    }
    print_test_footer();
}

void test_tick(void) {
    print_test_header("Tick_Update", "Time,DeltaTime,Output");

    pid_cfg_t cfg = {.kp = 1.0f, .beta = 1.0f};
    pid_t pid;
    pid_init(&pid, &cfg);

    float t = 0.0f;

    for (int i = 0; i < 20; i++) {
        float dt = 0.01f + ((float) (rand() % 40) / 1000.0f);
        float out = pid_update(&pid, 10.0f, 0.0f, dt);
        printf("%.3f,%.3f,%.2f\n", t, dt, out);
        t += dt;
    }
    print_test_footer();
}

void test_perf_metrics(void) {
    print_test_header("Perf_Metrics", "Metric,Value,Unit");

    pid_cfg_t cfg = {.kp = 1.0f,
                     .ki = 0.1f,
                     .kd = 0.05f,
                     .out_max = 100.0f,
                     .out_min = -100.0f,
                     .d_tau = 0.01f,
                     .beta = 1.0f,
                     .anti_windup_mode = PID_ANTI_WINDUP_DYNAMIC_CLAMP};
    pid_t pid;
    pid_init(&pid, &cfg);

    clock_t start = clock();
    int iterations = 100000;
    volatile float out;

    for (int i = 0; i < iterations; i++) {
        out = pid_update(&pid, 1.0f, 0.0f, 0.001f);
    }

    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    double avg_time_ns = (cpu_time_used * 1e9) / iterations;  // in ns

    printf("Avg_Time_Per_Update,%.2f,ns\n", avg_time_ns);

    pid_reset(&pid);
    plant_t plant;
    plant_init(&plant);
    float t = 0.0f;
    float dt = 0.01f;
    float sp = 10.0f;
    float rise_time = 0.0f;
    bool risen = false;

    for (int i = 0; i < 200; i++) {
        float meas = plant.output;
        float u = pid_update(&pid, sp, meas, dt);
        plant_update(&plant, u, dt);

        if (!risen && meas >= 0.9f * sp) {
            rise_time = t;
            risen = true;
        }
        t += dt;
    }
    printf("Rise_Time_(0-90),%.3f,s\n", rise_time);
    print_test_footer();
    (void) out;
}

void test_input_filter(void) {
    print_test_header("Input_Filter", "Time,Raw,Filtered_PT1,Filtered_Notch");

    // 1. PT1 Filter Test
    pid_cfg_t cfg_pt1 = {.kp = 1.0f,
                         .input_filter_type = PID_FILTER_PT1,
                         .input_filter_tau = 0.1f,
                         .out_max = 1000.0f,
                         .out_min = -1000.0f,
                         .beta = 1.0f};
    pid_t pid_pt1;
    pid_init(&pid_pt1, &cfg_pt1);

    // 2. Notch Filter Test (50Hz Notch at 1kHz sampling)
    pid_cfg_t cfg_notch = {.kp = 1.0f,
                           .input_filter_type = PID_FILTER_BIQUAD,
                           .out_max = 1000.0f,
                           .out_min = -1000.0f,
                           .beta = 1.0f};

    // Calculate Notch Coeffs: center=50Hz, bw=10Hz, dt=0.001s
    pid_filter_calc_notch(&cfg_notch.input_biquad_coeffs, 0.001f, 50.0f, 10.0f);

    pid_t pid_notch;
    pid_init(&pid_notch, &cfg_notch);

    float t = 0.0f;
    float dt = 0.001f;  // 1kHz

    // Generate signal: 5Hz sine + 50Hz noise
    float freq_sig = 5.0f;
    float freq_noise = 50.0f;

    for (int i = 0; i < 200; i++) {
        float signal = sinf(2.0f * M_PI * freq_sig * t);
        float noise = 0.5f * sinf(2.0f * M_PI * freq_noise * t);
        float raw = signal + noise;

        // We only care about how the measurement is filtered internally
        // To verify this without exposing internal state, we can look at P-term if Kp=1, Setpoint=0
        // Error = 0 - FilteredVal = -FilteredVal
        // Output = Kp * Error = -FilteredVal
        // So FilteredVal = -Output

        float out_pt1 = pid_update(&pid_pt1, 0.0f, raw, dt);
        float out_notch = pid_update(&pid_notch, 0.0f, raw, dt);

        printf("%.3f,%.3f,%.3f,%.3f\n", t, raw, -out_pt1, -out_notch);
        t += dt;
    }
    print_test_footer();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <test_name>\n", argv[0]);
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
    else if (strcmp(test, "deadband") == 0)
        test_deadband();
    else if (strcmp(test, "d_lpf") == 0)
        test_d_lpf();
    else if (strcmp(test, "biquad") == 0)
        test_biquad();
    else if (strcmp(test, "perf") == 0)
        test_perf_metrics();
    else if (strcmp(test, "output_rate_limit") == 0)
        test_rate_limit();
    else if (strcmp(test, "incremental") == 0)
        test_incremental();
    else if (strcmp(test, "cascade") == 0)
        test_cascade();
    else if (strcmp(test, "bumpless") == 0)
        test_bumpless();
    else if (strcmp(test, "tick_update") == 0)
        test_tick();
    else if (strcmp(test, "noise") == 0)
        test_noise();
    else if (strcmp(test, "input_filter") == 0)
        test_input_filter();
    else {
        printf("Unknown test: %s\n", test);
        return 1;
    }
    return 0;
}
