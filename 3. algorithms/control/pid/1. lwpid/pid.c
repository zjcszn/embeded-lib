/**
 * @file pid.c
 * @brief Robust PID Controller Implementation
 * @date 2026-02-08
 */

#include "pid.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Private Helper Functions ---

static inline pid_real_t pid_clamp(pid_real_t val, pid_real_t min, pid_real_t max) {
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

static inline bool pid_inputs_valid(pid_real_t sp, pid_real_t meas, pid_real_t dt) {
    return PID_CHECK_FINITE(sp) && PID_CHECK_FINITE(meas) && PID_CHECK_FINITE(dt) && (dt > 1e-6f);
}

static pid_real_t pid_filter_apply_biquad(pid_real_t input, const pid_biquad_coeffs_t *c,
                                          pid_filter_biquad_state_t *s) {
    // Direct Form I: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    pid_real_t output =
        c->b0 * input + c->b1 * s->x1 + c->b2 * s->x2 - c->a1 * s->y1 - c->a2 * s->y2;

    if (!PID_CHECK_FINITE(output)) {
        s->x1 = s->x2 = 0.0f;
        s->y1 = s->y2 = 0.0f;
        return input;
    }

    s->x2 = s->x1;
    s->x1 = input;
    s->y2 = s->y1;
    s->y1 = output;

    return output;
}

// --- Public API ---

bool pid_init(pid_t *pid, const pid_cfg_t *cfg) {
    if ((pid == NULL) || (cfg == NULL)) {
        return false;
    }
    if (cfg->out_max < cfg->out_min) {
        return false;
    }

#ifdef PID_CHECK_FINITE
    if (!PID_CHECK_FINITE(cfg->kp) || !PID_CHECK_FINITE(cfg->ki) || !PID_CHECK_FINITE(cfg->kd)) {
        return false;
    }
#endif

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
    pid->d_biquad.x1 = 0.0f;
    pid->d_biquad.x2 = 0.0f;
    pid->d_biquad.y1 = 0.0f;
    pid->d_biquad.y2 = 0.0f;

    pid->out = 0.0f;
    pid->first_run = true;
}

pid_real_t pid_update(pid_t *pid, pid_real_t setpoint, pid_real_t measurement, pid_real_t dt) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return 0.0f;
    }

    if (!pid_inputs_valid(setpoint, measurement, dt)) {
        PID_LOG("PID Error: Invalid inputs\n");
        return pid->out;
    }

    // 1. Setpoint Ramp
    pid_real_t target_setpoint = setpoint;

    if (pid->first_run) {
        pid->internal_setpoint = target_setpoint;
        pid->prev_measure = measurement;
        pid->prev_error = target_setpoint - measurement;
        pid->first_run = false;
    }

    if (pid->cfg->max_setpoint_ramp > 0.0f) {
        pid_real_t max_change = pid->cfg->max_setpoint_ramp * dt;
        pid_real_t diff = target_setpoint - pid->internal_setpoint;

        // Optimize clamping
        if (diff > max_change) {
            diff = max_change;
        } else if (diff < -max_change) {
            diff = -max_change;
        }

        pid->internal_setpoint += diff;
        setpoint = pid->internal_setpoint;
    }

    // 2. Error
    pid_real_t error = setpoint - measurement;

    // 3. Deadband
    if (pid->cfg->deadband > 0.0f) {
        if (error > pid->cfg->deadband) {
            error -= pid->cfg->deadband;
        } else if (error < -pid->cfg->deadband) {
            error += pid->cfg->deadband;
        } else {
            error = 0.0f;
        }
    }

    // 4. Proportional
    pid_real_t p_term = pid->cfg->kp * error;

    // 5. Derivative (Robust)
    pid_real_t derivative;
    if (pid->cfg->derivative_on_measurement) {
        derivative = -(measurement - pid->prev_measure) / dt;
    } else {
        derivative = (error - pid->prev_error) / dt;
    }

    // Filter Derivative
    if (pid->cfg->d_filter_type == PID_FILTER_BIQUAD) {
        pid->d_lpf =
            pid_filter_apply_biquad(derivative, &pid->cfg->d_biquad_coeffs, &pid->d_biquad);
    } else {
        // PT1: Alpha calculation
        pid_real_t alpha = 1.0f;
        if (pid->cfg->d_tau > 0.0f) {
            alpha = dt / (dt + pid->cfg->d_tau);
        } else if (pid->cfg->d_lpf_alpha > 0.0f) {
            alpha = pid->cfg->d_lpf_alpha;
        }
        pid->d_lpf += alpha * (derivative - pid->d_lpf);
    }
    pid_real_t d_term = pid->cfg->kd * pid->d_lpf;

    // 6. Feed-Forward
    pid_real_t f_term = pid->cfg->kf * setpoint;

    // 7. Integral (Dynamic Anti-windup)
    pid_real_t i_term_change = pid->cfg->ki * error * dt;

    if (pid->cfg->anti_windup_mode == PID_ANTI_WINDUP_DYNAMIC_CLAMP) {
        // Dynamic Clamping: Restrict I-term based on P+D+FF headroom
        pid_real_t pdff = p_term + d_term + f_term;
        pid_real_t i_max = pid->cfg->out_max - pdff;
        pid_real_t i_min = pid->cfg->out_min - pdff;

        bool sat_max = (pdff > pid->cfg->out_max);
        bool sat_min = (pdff < pid->cfg->out_min);

        if (sat_max) {
            i_max = 0.0f;
            if (i_min > 0.0f) {
                i_min = -i_max;
            }
        } else if (sat_min) {
            i_min = 0.0f;
            if (i_max < 0.0f) {
                i_max = -i_min;
            }
        }

        // Ensure regular ordering for clamp
        if (i_min > i_max) {
            pid_real_t t = i_min;
            i_min = i_max;
            i_max = t;
        }

        pid->integral += i_term_change;
        pid->integral = pid_clamp(pid->integral, i_min, i_max);

    } else if (pid->cfg->anti_windup_mode == PID_ANTI_WINDUP_CLAMP) {
        pid->integral += i_term_change;
        pid->integral = pid_clamp(pid->integral, pid->cfg->out_min, pid->cfg->out_max);
    } else if (pid->cfg->anti_windup_mode == PID_ANTI_WINDUP_CONDITIONAL) {
        // Calculate predicted output with current P, D, FF and OLD Integral
        pid_real_t out_predict = p_term + pid->integral + d_term + f_term;

        bool sat_high = (out_predict >= pid->cfg->out_max);
        bool sat_low = (out_predict <= pid->cfg->out_min);

        // Only integrate if not saturated, or if error helps unsaturate
        bool integrate = true;
        if (sat_high && (i_term_change > 0.0f)) {
            integrate = false;
        } else if (sat_low && (i_term_change < 0.0f)) {
            integrate = false;
        }

        if (integrate) {
            pid->integral += i_term_change;
        }
    } else {
        pid->integral += i_term_change;
    }

    // 8. Output
    pid_real_t out_unclamped = p_term + pid->integral + d_term + f_term;
    pid_real_t out = pid_clamp(out_unclamped, pid->cfg->out_min, pid->cfg->out_max);

    // 9. Back-calculation
    if (pid->cfg->anti_windup_mode == PID_ANTI_WINDUP_BACK_CALC) {
        if (pid->cfg->kw > 0.0f) {
            pid->integral += pid->cfg->kw * (out - out_unclamped) * dt;
        }
    }

    // Update History
    pid->prev_measure = measurement;
    pid->prev_error = error;
    pid->out = out;

    return out;
}

// ... Incremental PID ... //

pid_real_t pid_update_incremental(pid_t *pid, pid_real_t setpoint, pid_real_t measurement,
                                  pid_real_t dt) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return 0.0f;
    }
    if (!pid_inputs_valid(setpoint, measurement, dt)) {
        return 0.0f;
    }

    pid_real_t error = setpoint - measurement;

    if (pid->first_run) {
        pid->prev_measure = measurement;
        pid->prev_error = error;
        pid->d_lpf = 0.0f;
        pid->d_biquad.x1 = 0;
        pid->d_biquad.x2 = 0;
        pid->d_biquad.y1 = 0;
        pid->d_biquad.y2 = 0;
        pid->first_run = false;
        return 0.0f;
    }

    // 1. Proportional (dE)
    pid_real_t p_diff = pid->cfg->kp * (error - pid->prev_error);

    // 2. Integral (E*dt)
    pid_real_t i_diff = pid->cfg->ki * error * dt;

    // 3. Derivative (d(dE)/dt or d(dPV)/dt)
    pid_real_t derivative;
    if (pid->cfg->derivative_on_measurement) {
        derivative = -(measurement - pid->prev_measure) / dt;
    } else {
        derivative = (error - pid->prev_error) / dt;
    }

    pid_real_t old_d_lpf = pid->d_lpf;

    if (pid->cfg->d_filter_type == PID_FILTER_BIQUAD) {
        pid->d_lpf =
            pid_filter_apply_biquad(derivative, &pid->cfg->d_biquad_coeffs, &pid->d_biquad);
    } else {
        pid_real_t alpha = 1.0f;
        if (pid->cfg->d_tau > 0.0f) {
            alpha = dt / (dt + pid->cfg->d_tau);
        } else if (pid->cfg->d_lpf_alpha > 0.0f) {
            alpha = pid->cfg->d_lpf_alpha;
        }

        pid->d_lpf += alpha * (derivative - pid->d_lpf);
    }

    pid_real_t d_diff = pid->cfg->kd * (pid->d_lpf - old_d_lpf);

    // 4. Total Change
    pid_real_t delta_out = p_diff + i_diff + d_diff;

    // 5. Rate Limit
    if (pid->cfg->max_rate > 0.0f) {
        pid_real_t limit = pid->cfg->max_rate * dt;
        delta_out = pid_clamp(delta_out, -limit, limit);
    }

    pid->prev_measure = measurement;
    pid->prev_error = error;

    return delta_out;
}

void pid_set_integral(pid_t *pid, pid_real_t value) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return;
    }
    pid->integral = pid_clamp(value, pid->cfg->out_min, pid->cfg->out_max);
}

void pid_track_manual(pid_t *pid, pid_real_t manual_output, pid_real_t measurement,
                      pid_real_t setpoint) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return;
    }

    // Bumpless: I = u_man - P - D - FF
    pid_real_t error = setpoint - measurement;
    pid_real_t p_term = pid->cfg->kp * error;
    // Assume D=0 for tracking safely
    pid_real_t f_term = pid->cfg->kf * setpoint;

    pid->integral =
        pid_clamp(manual_output - p_term - f_term, pid->cfg->out_min, pid->cfg->out_max);

    pid->prev_measure = measurement;
    pid->prev_error = error;
    pid->internal_setpoint = setpoint;
}

// --- Cascade ---

bool pid_cascade_init(pid_cascade_t *cascade, pid_t *outer, pid_t *inner) {
    if (!cascade || !outer || !inner) {
        return false;
    }
    cascade->outer = outer;
    cascade->inner = inner;
    return true;
}

pid_real_t pid_cascade_update(pid_cascade_t *cascade, pid_real_t setpoint, pid_real_t outer_measure,
                              pid_real_t inner_measure, pid_real_t dt) {
    if (!cascade) {
        return 0.0f;
    }
    pid_real_t inner_sp = pid_update(cascade->outer, setpoint, outer_measure, dt);
    return pid_update(cascade->inner, inner_sp, inner_measure, dt);
}

// --- Filter Init ---

void pid_filter_calc_pt2(pid_biquad_coeffs_t *coeffs, pid_real_t dt, pid_real_t cutoff_freq,
                         pid_real_t q_factor) {
    if (!coeffs || dt <= 0.0f || cutoff_freq <= 0.0f) {
        return;
    }

    pid_real_t w0 = 2.0f * M_PI * cutoff_freq * dt;
    pid_real_t alpha = sinf(w0) / (2.0f * q_factor);
    pid_real_t cos_w0 = cosf(w0);

    pid_real_t a0 = 1.0f + alpha;  // Normalize by a0

    coeffs->b0 = ((1.0f - cos_w0) / 2.0f) / a0;
    coeffs->b1 = (1.0f - cos_w0) / a0;
    coeffs->b2 = coeffs->b0;  // b2 = b0

    coeffs->a1 = (-2.0f * cos_w0) / a0;
    coeffs->a2 = (1.0f - alpha) / a0;
}

void pid_filter_calc_notch(pid_biquad_coeffs_t *coeffs, pid_real_t dt, pid_real_t center_freq,
                           pid_real_t bandwidth) {
    if (!coeffs || dt <= 0.0f || center_freq <= 0.0f) {
        return;
    }

    pid_real_t w0 = 2.0f * M_PI * center_freq * dt;
    pid_real_t q = center_freq / bandwidth;
    pid_real_t alpha = sinf(w0) / (2.0f * q);
    pid_real_t cos_w0 = cosf(w0);

    pid_real_t a0 = 1.0f + alpha;  // Normalize

    coeffs->b0 = 1.0f / a0;
    coeffs->b1 = (-2.0f * cos_w0) / a0;
    coeffs->b2 = 1.0f / a0;

    coeffs->a1 = (-2.0f * cos_w0) / a0;
    coeffs->a2 = (1.0f - alpha) / a0;
}
