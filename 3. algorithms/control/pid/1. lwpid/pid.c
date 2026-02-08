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

// --- Private Macros ---
#ifndef PID_CHECK_FINITE
#define PID_CHECK_FINITE(val) isfinite(val)
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
    // Check output limits
    if (cfg->out_max < cfg->out_min) {
        return false;
    }

    // Check finite values for all floats
#ifdef PID_CHECK_FINITE
    if (!PID_CHECK_FINITE(cfg->kp) || !PID_CHECK_FINITE(cfg->ki) || !PID_CHECK_FINITE(cfg->kd) ||
        !PID_CHECK_FINITE(cfg->kf) || !PID_CHECK_FINITE(cfg->out_max) ||
        !PID_CHECK_FINITE(cfg->out_min) || !PID_CHECK_FINITE(cfg->deadband) ||
        !PID_CHECK_FINITE(cfg->max_rate) || !PID_CHECK_FINITE(cfg->max_setpoint_ramp) ||
        !PID_CHECK_FINITE(cfg->kw) || !PID_CHECK_FINITE(cfg->d_tau) ||
        !PID_CHECK_FINITE(cfg->d_lpf_alpha) || !PID_CHECK_FINITE(cfg->beta) ||
        !PID_CHECK_FINITE(cfg->gamma)) {
        return false;
    }

    // Check Input Filter
    if (!PID_CHECK_FINITE(cfg->input_filter_tau)) {
        return false;
    }
    if (cfg->input_filter_type == PID_FILTER_BIQUAD) {
        if (!PID_CHECK_FINITE(cfg->input_biquad_coeffs.b0) ||
            !PID_CHECK_FINITE(cfg->input_biquad_coeffs.b1) ||
            !PID_CHECK_FINITE(cfg->input_biquad_coeffs.b2) ||
            !PID_CHECK_FINITE(cfg->input_biquad_coeffs.a1) ||
            !PID_CHECK_FINITE(cfg->input_biquad_coeffs.a2)) {
            return false;
        }
    }

    // Check Derivative Filter
    if (cfg->d_filter_type == PID_FILTER_BIQUAD) {
        if (!PID_CHECK_FINITE(cfg->d_biquad_coeffs.b0) ||
            !PID_CHECK_FINITE(cfg->d_biquad_coeffs.b1) ||
            !PID_CHECK_FINITE(cfg->d_biquad_coeffs.b2) ||
            !PID_CHECK_FINITE(cfg->d_biquad_coeffs.a1) ||
            !PID_CHECK_FINITE(cfg->d_biquad_coeffs.a2)) {
            return false;
        }
    }
#endif

    // Check non-negative parameters
    if ((cfg->deadband < 0.0f) || (cfg->max_rate < 0.0f) || (cfg->max_setpoint_ramp < 0.0f) ||
        (cfg->d_tau < 0.0f)) {
        return false;
    }

    // Check Alpha range
    if ((cfg->d_lpf_alpha < 0.0f) || (cfg->d_lpf_alpha > 1.0f)) {
        return false;
    }

    // Check Beta and Gamma range (0.0 to 1.0)
    if ((cfg->beta < 0.0f) || (cfg->beta > 1.0f) || (cfg->gamma < 0.0f) || (cfg->gamma > 1.0f)) {
        return false;
    }

    // Check Enum Ranges
    if (cfg->anti_windup_mode > PID_ANTI_WINDUP_NONE) {
        return false;
    }
    if (cfg->d_filter_type > PID_FILTER_BIQUAD) {
        return false;
    }
    if (cfg->input_filter_type > PID_FILTER_BIQUAD) {
        return false;
    }
    if (cfg->input_filter_tau < 0.0f) {
        return false;
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
    pid->d_biquad.x1 = 0.0f;
    pid->d_biquad.x2 = 0.0f;
    pid->d_biquad.y1 = 0.0f;
    pid->d_biquad.y2 = 0.0f;

    pid->input_lpf = 0.0f;
    pid->input_biquad.x1 = 0.0f;
    pid->input_biquad.x2 = 0.0f;
    pid->input_biquad.y1 = 0.0f;
    pid->input_biquad.y2 = 0.0f;

    pid->out = 0.0f;
    pid->first_run = true;
}

pid_real_t pid_update(pid_t *pid, pid_real_t setpoint, pid_real_t measurement, pid_real_t dt) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return 0.0f;
    }

    if (!pid_inputs_valid(setpoint, measurement, dt)) {
        PID_LOG("PID Error: Invalid inputs (NaN/Inf or dt<=0)\n");
        return pid->out;  // Return last output
    }

    // 1. Setpoint Ramp
    pid_real_t target_setpoint = setpoint;

    // 0. Input Filter (Pre-filter)
    // Applied to the Measurement (PV) before error calculation.
    pid_real_t filtered_measure = measurement;

    if (pid->first_run) {
        // Initialize Filter State to current measurement to avoid startup jumps
        pid->input_lpf = measurement;
        pid->input_biquad.x1 = measurement;
        pid->input_biquad.x2 = measurement;
        pid->input_biquad.y1 = measurement;
        pid->input_biquad.y2 = measurement;

        pid->internal_setpoint = target_setpoint;
        pid->prev_measure = filtered_measure;
        // Assume error=0 initially if beta/gamma not used history,
        // but for consistency with history calc:
        // prev_error usually stores (SP - PV).
        pid->prev_error = target_setpoint - filtered_measure;
        pid->first_run = false;
    }

    if (pid->cfg->input_filter_type == PID_FILTER_BIQUAD) {
        filtered_measure = pid_filter_apply_biquad(measurement, &pid->cfg->input_biquad_coeffs,
                                                   &pid->input_biquad);
    } else if (pid->cfg->input_filter_type == PID_FILTER_PT1) {
        pid_real_t alpha = 1.0f;
        if (pid->cfg->input_filter_tau > 0.0f) {
            alpha = dt / (dt + pid->cfg->input_filter_tau);
        }
        if (alpha > 1.0f) {
            alpha = 1.0f;
        } else if (alpha < 0.0f) {
            alpha = 0.0f;
        }
        pid->input_lpf += alpha * (measurement - pid->input_lpf);
        filtered_measure = pid->input_lpf;
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
        target_setpoint = pid->internal_setpoint;
    }

    // 2. Error Calculation
    // Always SP - PV
    pid_real_t error = target_setpoint - filtered_measure;

    // 3. Deadband
    // Applies to Error used for I-term.
    if (pid->cfg->deadband > 0.0f) {
        if (fabsf(error) < pid->cfg->deadband) {
            error = 0.0f;
        } else {
            if (error > 0.0f) {
                error -= pid->cfg->deadband;
            } else {
                error += pid->cfg->deadband;
            }
        }
    }

    // 4. Proportional Term (P)
    // Calculated using 2DOF error: (beta * SP) - PV
    pid_real_t p_error = (pid->cfg->beta * target_setpoint) - filtered_measure;

    // Apply deadband to P-term ONLY IF beta == 1.0 (Standard PID compatibility)
    // For 2DOF, P-term remains active on measurement changes to suppress disturbances.
    if ((pid->cfg->beta == 1.0f) && (pid->cfg->deadband > 0.0f)) {
        if (p_error > pid->cfg->deadband) {
            p_error -= pid->cfg->deadband;
        } else if (p_error < -pid->cfg->deadband) {
            p_error += pid->cfg->deadband;
        } else {
            p_error = 0.0f;
        }
    }

    // 4. Proportional
    pid_real_t p_term = pid->cfg->kp * p_error;

    // 5. Derivative Term (D)
    // Calculated using 2DOF input: (gamma * SP) - PV
    // To handle derivative correctly with discontinuous setpoints (if gamma > 0),
    // we assume the setpoint change happened smoothly or we fundamentally
    // want to differentiate the signal (gamma * SP - PV).
    //
    // Robust derivative calculation usually avoids differentiating the Setpoint
    // (gamma=0), making it "Derivative on Measurement" (-dPV/dt).

    // Reconstruct previous inputs to calculate derivative.
    // prev_error = prev_SP - prev_PV  => prev_SP = prev_error + prev_PV
    pid_real_t prev_sp_reconstructed = pid->prev_error + pid->prev_measure;
    pid_real_t d_input = (pid->cfg->gamma * target_setpoint) - filtered_measure;
    pid_real_t prev_d_input = (pid->cfg->gamma * prev_sp_reconstructed) - pid->prev_measure;

    pid_real_t derivative = (d_input - prev_d_input) / dt;

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
        // Safety Clean: Clamp Alpha
        if (alpha > 1.0f) {
            alpha = 1.0f;
        }
        if (alpha < 0.0f) {
            alpha = 0.0f;
        }

        pid->d_lpf += alpha * (derivative - pid->d_lpf);
    }
    pid_real_t d_term = pid->cfg->kd * pid->d_lpf;

    // 6. Feed-Forward
    pid_real_t f_term = pid->cfg->kf * target_setpoint;

    // 7. Integral (Dynamic Anti-windup)
    // I-term always works on regular error (SP-PV) to ensure zero steady-state error
    pid_real_t i_term_change = pid->cfg->ki * error * dt;

    if (pid->cfg->anti_windup_mode == PID_ANTI_WINDUP_DYNAMIC_CLAMP) {
        // Dynamic Clamping: Restrict I-term based on P+D+FF headroom
        pid_real_t pdff = p_term + d_term + f_term;
        pid_real_t i_max = pid->cfg->out_max - pdff;
        pid_real_t i_min = pid->cfg->out_min - pdff;

        // Correct logic if min > max due to saturation
        if (i_max < i_min) {
            // Swap to ensure valid clamp range
            pid_real_t t = i_max;
            i_max = i_min;
            i_min = t;
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
    pid->prev_measure = filtered_measure;
    pid->prev_error = target_setpoint - filtered_measure;  // Store raw error
    pid->out = out;

    return out;
}

// ... Incremental PID ... //

pid_real_t pid_update_incremental(pid_t *pid, pid_real_t setpoint, pid_real_t measurement,
                                  pid_real_t dt, pid_real_t current_output) {
    if ((pid == NULL) || (pid->cfg == NULL)) {
        return 0.0f;
    }
    if (!pid_inputs_valid(setpoint, measurement, dt)) {
        PID_LOG("PID Error: Invalid inputs (incremental)\n");
        return 0.0f;
    }

    // 0. Input Filter (Pre-filter)
    pid_real_t filtered_measure = measurement;

    if (pid->first_run) {
        pid->input_lpf = measurement;
        pid->input_biquad.x1 = measurement;
        pid->input_biquad.x2 = measurement;
        pid->input_biquad.y1 = measurement;
        pid->input_biquad.y2 = measurement;

        pid->prev_measure = filtered_measure;
        pid->prev_error = setpoint - filtered_measure;
        pid->d_lpf = 0.0f;
        pid->d_biquad.x1 = 0;
        pid->d_biquad.x2 = 0;
        pid->d_biquad.y1 = 0;
        pid->d_biquad.y2 = 0;
        pid->first_run = false;
        return 0.0f;
    }

    if (pid->cfg->input_filter_type == PID_FILTER_BIQUAD) {
        filtered_measure = pid_filter_apply_biquad(measurement, &pid->cfg->input_biquad_coeffs,
                                                   &pid->input_biquad);
    } else if (pid->cfg->input_filter_type == PID_FILTER_PT1) {
        pid_real_t alpha = 1.0f;
        if (pid->cfg->input_filter_tau > 0.0f) {
            alpha = dt / (dt + pid->cfg->input_filter_tau);
        }
        if (alpha > 1.0f) {
            alpha = 1.0f;
        } else if (alpha < 0.0f) {
            alpha = 0.0f;
        }
        pid->input_lpf += alpha * (measurement - pid->input_lpf);
        filtered_measure = pid->input_lpf;
    }

    pid_real_t error = setpoint - filtered_measure;

    // 1. Proportional Change (dP)
    // 2DOF P = Kp * (beta * SP - PV)
    // dP = P_current - P_previous
    pid_real_t prev_sp = pid->prev_error + pid->prev_measure;

    pid_real_t p_current = pid->cfg->kp * ((pid->cfg->beta * setpoint) - filtered_measure);
    pid_real_t p_prev = pid->cfg->kp * ((pid->cfg->beta * prev_sp) - pid->prev_measure);

    pid_real_t p_diff = p_current - p_prev;

    // 2. Integral Change (dI)
    // dI = Ki * Error * dt
    pid_real_t i_diff = pid->cfg->ki * error * dt;

    // 3. Derivative Change (dD)
    // D = Kd * d/dt(gamma * SP - PV)
    // dD = D_current - D_previous
    pid_real_t d_input = (pid->cfg->gamma * setpoint) - filtered_measure;
    pid_real_t prev_d_input = (pid->cfg->gamma * prev_sp) - pid->prev_measure;

    pid_real_t derivative = (d_input - prev_d_input) / dt;

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
        // Safety Clean
        if (alpha > 1.0f) {
            alpha = 1.0f;
        }
        if (alpha < 0.0f) {
            alpha = 0.0f;
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

    // 6. Anti-windup (Clamping based on current output)
    // Check if adding delta_out would make current_output exceed limits
    pid_real_t next_out_unclamped = current_output + delta_out;

    if (next_out_unclamped > pid->cfg->out_max) {
        // Clamp delta so we don't exceed max
        // delta_max = out_max - current_output
        // But if current_output is already > max, we should allow negative delta
        if (next_out_unclamped > current_output) {  // only clamp if we are increasing
            pid_real_t space = pid->cfg->out_max - current_output;
            if (delta_out > space) {
                delta_out = (space > 0.0f) ? space : 0.0f;
                // If space < 0 (already saturated), we allow delta=0
                // Or we could force negative delta?
                // Standard incremental: just stop increasing.
            }
        }
    } else if (next_out_unclamped < pid->cfg->out_min) {
        // Clamp delta so we don't exceed min
        if (next_out_unclamped < current_output) {  // only clamp if we are decreasing
            pid_real_t space = pid->cfg->out_min - current_output;
            if (delta_out < space) {
                delta_out = (space < 0.0f) ? space : 0.0f;
            }
        }
    }

    pid->prev_measure = filtered_measure;
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

    // P term with 2DOF
    pid_real_t p_error = (pid->cfg->beta * setpoint) - measurement;
    // Deadband for P-term if beta=1
    if ((pid->cfg->beta == 1.0f) && (pid->cfg->deadband > 0.0f)) {
        if (p_error > pid->cfg->deadband) {
            p_error -= pid->cfg->deadband;
        } else if (p_error < -pid->cfg->deadband) {
            p_error += pid->cfg->deadband;
        } else {
            p_error = 0.0f;
        }
    }

    pid_real_t p_term = pid->cfg->kp * p_error;

    // Assume D=0 for tracking safely
    pid_real_t f_term = pid->cfg->kf * setpoint;

    pid->integral =
        pid_clamp(manual_output - p_term - f_term, pid->cfg->out_min, pid->cfg->out_max);

    pid->prev_measure = measurement;
    pid->prev_error = error;
    pid->internal_setpoint = setpoint;

    // Reset Filter state to prevent "Derivative Kick" when switching to Auto
    pid->d_lpf = 0.0f;
    pid->d_biquad.x1 = 0.0f;
    pid->d_biquad.x2 = 0.0f;
    pid->d_biquad.y1 = 0.0f;
    pid->d_biquad.y2 = 0.0f;

    // Reset Input Filter to current measurement to prevent jumps
    pid->input_lpf = measurement;
    pid->input_biquad.x1 = measurement;
    pid->input_biquad.x2 = measurement;
    pid->input_biquad.y1 = measurement;
    pid->input_biquad.y2 = measurement;
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
    // Check validation
    if (!coeffs || dt <= 0.0f || cutoff_freq <= 0.0f || q_factor < 0.001f) {
        // Safe Fallback: Pass-through
        if (coeffs) {
            coeffs->b0 = 1.0f;
            coeffs->b1 = 0.0f;
            coeffs->b2 = 0.0f;
            coeffs->a1 = 0.0f;
            coeffs->a2 = 0.0f;
        }
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
    if (!coeffs || dt <= 0.0f || center_freq <= 0.0f || bandwidth < 0.001f) {
        // Safe Fallback: Pass-through
        if (coeffs) {
            coeffs->b0 = 1.0f;
            coeffs->b1 = 0.0f;
            coeffs->b2 = 0.0f;
            coeffs->a1 = 0.0f;
            coeffs->a2 = 0.0f;
        }
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
