/**
 * @file pid.c
 * @brief Robust PID Controller Implementation
 * @date 2026-02-08
 */

#include "pid.h"

#include <math.h>

/* --- Private Helper Functions (MISRA-C Compliance) --- */

/**
 * @brief Clamp value between min and max
 */
static inline pid_real_t pid_clamp(pid_real_t val, pid_real_t min, pid_real_t max) {
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

/* --- Public API Implementation --- */

bool pid_init(pid_t *pid, const pid_cfg_t *cfg) {
    if ((pid == NULL) || (cfg == NULL)) {
        return false;
    }

    // Basic configuration validation
    if (cfg->out_max < cfg->out_min) {
        return false;  // Invalid output range
    }

    // Check for negative time constants or gains if applicable (optional but recommended)
    if (cfg->d_lpf_alpha < 0.0f || cfg->d_lpf_alpha > 1.0f) {
        return false;  // Invalid filter alpha
    }

    pid->cfg = cfg;
    pid_reset(pid);

    return true;
}

void pid_reset(pid_t *pid) {
    if (pid == NULL) {
        return;
    }

    pid->integral = 0.0f;
    pid->prev_measure = 0.0f;
    pid->prev_error = 0.0f;
    pid->d_lpf = 0.0f;
    pid->out = 0.0f;
    pid->first_run = true;
}

pid_real_t pid_update(pid_t *pid, pid_real_t setpoint, pid_real_t measurement, pid_real_t dt) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return 0.0f;
    }

    // Protection against invalid dt
    if (dt <= 1e-6f) {
        return pid->out;
    }

    /* --- 1. Setpoint Ramp Control --- */
    pid_real_t target_setpoint = setpoint;

    // Initialization on first run
    if (pid->first_run) {
        pid->internal_setpoint = target_setpoint;
        pid->prev_measure = measurement;
        pid->prev_error = target_setpoint - measurement;
        pid->first_run = false;
    }

    // Rate limiting
    if (pid->cfg->max_setpoint_ramp > 0.0f) {
        pid_real_t max_change = pid->cfg->max_setpoint_ramp * dt;
        pid_real_t diff = target_setpoint - pid->internal_setpoint;

        if (diff > max_change) {
            pid->internal_setpoint += max_change;
        } else if (diff < -max_change) {
            pid->internal_setpoint -= max_change;
        } else {
            pid->internal_setpoint = target_setpoint;
        }

        setpoint = pid->internal_setpoint;
    }

    /* --- 2. Error Calculation --- */
    pid_real_t error = setpoint - measurement;

    /* --- 3. Linear Deadband (Upgrade from simple zeroing) --- */

    /* --- 3. Deadband --- */
    if (pid->cfg->deadband > 0.0f) {
        if (error > pid->cfg->deadband) {
            error -= pid->cfg->deadband;
        } else if (error < -pid->cfg->deadband) {
            error += pid->cfg->deadband;
        } else {
            error = 0.0f;
        }
    }

    /* --- 4. Proportional Term --- */
    pid_real_t p_term = pid->cfg->kp * error;

    /* --- 5. Integral Term (with Configurable Anti-windup) --- */
    bool output_saturated = (pid->out >= pid->cfg->out_max) || (pid->out <= pid->cfg->out_min);
    bool error_sign_same_as_out = (error * pid->out) > 0.0f;

    switch (pid->cfg->anti_windup_mode) {
        case PID_ANTI_WINDUP_CONDITIONAL:
            if (!output_saturated || !error_sign_same_as_out) {
                pid->integral += pid->cfg->ki * error * dt;
            }
            break;

        case PID_ANTI_WINDUP_BACK_CALC:
            /* Standard I-term accumulation; correction applied after output clamping */
            pid->integral += pid->cfg->ki * error * dt;
            break;

        case PID_ANTI_WINDUP_CLAMP:
            pid->integral += pid->cfg->ki * error * dt;
            pid->integral = pid_clamp(pid->integral, pid->cfg->out_min, pid->cfg->out_max);
            break;

        case PID_ANTI_WINDUP_NONE:
        default:
            pid->integral += pid->cfg->ki * error * dt;
            break;
    }

    /* --- 5. Derivative Term (Robust) --- */
    pid_real_t d_term = 0.0f;
    pid_real_t derivative = 0.0f;

    if (pid->cfg->derivative_on_measurement) {
        derivative = -(measurement - pid->prev_measure) / dt;
    } else {
        derivative = (error - pid->prev_error) / dt;
    }

    // Always update history
    pid->prev_measure = measurement;
    pid->prev_error = error;

    // Low Pass Filter for Derivative
    if (pid->cfg->d_lpf_alpha > 0.0f) {
        pid->d_lpf = pid->d_lpf + pid->cfg->d_lpf_alpha * (derivative - pid->d_lpf);
        d_term = pid->cfg->kd * pid->d_lpf;
    } else {
        pid->d_lpf = derivative;
        d_term = pid->cfg->kd * derivative;
    }

    /* --- 6. Feed-Forward Term --- */
    pid_real_t f_term = pid->cfg->kf * setpoint;

    /* --- 7. Calculate Unclamped Output --- */
    pid_real_t out_unclamped = p_term + pid->integral + d_term + f_term;

    /* --- 8. Final Clamping --- */
    pid_real_t out = pid_clamp(out_unclamped, pid->cfg->out_min, pid->cfg->out_max);

    /* --- 9. Back-calculation Correction --- */
    if (pid->cfg->anti_windup_mode == PID_ANTI_WINDUP_BACK_CALC) {
        if (pid->cfg->kw > 0.0f) {
            /* I += Kw * (u_sat - u_unclamped) * dt */
            pid_real_t saturation_diff = out - out_unclamped;
            pid->integral += pid->cfg->kw * saturation_diff * dt;
        }
    }

    pid->out = out;
    return out;
}

/* ... (Incremental PID remains unchanged) ... */

/* --- Cascade PID Implementation --- */

bool pid_cascade_init(pid_cascade_t *cascade, pid_t *outer, pid_t *inner) {
    if (cascade == NULL || outer == NULL || inner == NULL) {
        return false;
    }
    cascade->outer = outer;
    cascade->inner = inner;
    return true;
}

pid_real_t pid_cascade_update(pid_cascade_t *cascade, pid_real_t setpoint, pid_real_t outer_measure,
                              pid_real_t inner_measure, pid_real_t dt) {
    if (cascade == NULL) {
        return 0.0f;
    }

    // 1. Update Outer Loop
    // The output of outer loop is the setpoint for inner loop.
    pid_real_t inner_setpoint = pid_update(cascade->outer, setpoint, outer_measure, dt);

    // 2. Update Inner Loop
    pid_real_t output = pid_update(cascade->inner, inner_setpoint, inner_measure, dt);

    /* 3. Sync Anti-windup (Placeholders for future expansion) */
    /* Note: Advanced cascade strategies may require feedback from inner to outer loop */

    return output;
}

pid_real_t pid_update_incremental(pid_t *pid, pid_real_t setpoint, pid_real_t measurement,
                                  pid_real_t dt) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return 0.0f;
    }

    if (dt <= 1e-6f) {
        return 0.0f;
    }

    pid_real_t error = setpoint - measurement;

    if (pid->first_run) {
        pid->prev_measure = measurement;
        pid->prev_error = error;
        pid->d_lpf = 0.0f;
        pid->first_run = false;
        return 0.0f;  // No output change on first run
    }

    /* --- 1. Proportional Change --- */
    // dP = Kp * (error - prev_error)
    pid_real_t p_diff = pid->cfg->kp * (error - pid->prev_error);

    /* --- 2. Integral Change --- */
    // dI = Ki * error * dt
    // Note: No anti-windup needed here usually, but output rate limiting handles it.
    pid_real_t i_diff = pid->cfg->ki * error * dt;

    /* --- 3. Derivative Change --- */
    // Calculate new derivative
    pid_real_t derivative;
    if (pid->cfg->derivative_on_measurement) {
        derivative = -(measurement - pid->prev_measure) / dt;
    } else {
        derivative = (error - pid->prev_error) / dt;
    }

    // Remember old filtered value
    pid_real_t old_d_lpf = pid->d_lpf;

    // Update LPF
    if (pid->cfg->d_lpf_alpha > 0.0f) {
        pid->d_lpf = pid->d_lpf + pid->cfg->d_lpf_alpha * (derivative - pid->d_lpf);
    } else {
        pid->d_lpf = derivative;
    }

    // dD = Kd * (new_D - old_D)
    pid_real_t d_diff = pid->cfg->kd * (pid->d_lpf - old_d_lpf);

    /* --- 4. Total Change --- */
    pid_real_t delta_out = p_diff + i_diff + d_diff;

    /* --- 5. Rate Limiting (Slew Rate) --- */
    if (pid->cfg->max_rate > 0.0f) {
        pid_real_t max_delta = pid->cfg->max_rate * dt;
        delta_out = pid_clamp(delta_out, -max_delta, max_delta);
    }

    /* --- 6. Update History --- */
    pid->prev_measure = measurement;
    pid->prev_error = error;

    return delta_out;
}

void pid_set_integral(pid_t *pid, pid_real_t value) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return;
    }

    // Direct assignment with clamping
    pid->integral = pid_clamp(value, pid->cfg->out_min, pid->cfg->out_max);
}

void pid_track_manual(pid_t *pid, pid_real_t manual_output, pid_real_t measurement,
                      pid_real_t setpoint) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return;
    }

    /*
     * Bumpless Transfer Logic:
     * We want: pid_out = manual_output
     * From: pid_out = P + I + D + FF
     * So: I = manual_output - (P + D + FF)
     */

    pid_real_t error = setpoint - measurement;

    // 1. Calculate P-term
    pid_real_t p_term = pid->cfg->kp * error;

    // 2. Calculate D-term
    // Since we don't have dt, we assume D = 0 for tracking to be safe.
    pid_real_t d_term = 0.0f;

    // 3. Calculate FF-term
    pid_real_t f_term = pid->cfg->kf * setpoint;

    // 4. Back-calculate Integral
    pid_real_t new_integral = manual_output - p_term - d_term - f_term;

    // 5. Update State
    pid->integral = pid_clamp(new_integral, pid->cfg->out_min, pid->cfg->out_max);

    // Update history to prevent derivative kick when switching back to Auto
    pid->prev_measure = measurement;
    pid->prev_error = error;
    pid->internal_setpoint = setpoint;
}