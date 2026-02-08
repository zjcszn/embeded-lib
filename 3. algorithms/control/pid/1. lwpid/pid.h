/**
 * @file pid.h
 * @brief Robust PID Controller Library
 * @date 2026-02-08
 */

#ifndef PID_CORE_H
#define PID_CORE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Configuration Macros ---

// Logging: Define PID_LOG to printf or other sink if needed
#ifndef PID_LOG
#define PID_LOG(fmt, ...)
#endif

// --- Data Types ---

// float or double
typedef float pid_real_t;

/**
 * @brief Anti-windup Modes
 */
typedef enum {
    PID_ANTI_WINDUP_CONDITIONAL = 0,  // Conditional Integration (Default)
    PID_ANTI_WINDUP_BACK_CALC,        // Back-calculation (Tracking)
    PID_ANTI_WINDUP_CLAMP,            // Integral State Clamping (Static limits)
    PID_ANTI_WINDUP_DYNAMIC_CLAMP,    // Dynamic Clamping based on P/D headroom
    PID_ANTI_WINDUP_NONE              // No Anti-windup
} pid_anti_windup_mode_t;

/**
 * @brief Derivative Filter Type
 */
typedef enum {
    PID_FILTER_NONE = 0,
    PID_FILTER_PT1,    // First Order Low Pass (RC)
    PID_FILTER_BIQUAD  // Second Order (Biquad)
} pid_filter_type_t;

/**
 * @brief Biquad Filter Coefficients
 * y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
 */
typedef struct {
    pid_real_t b0, b1, b2;
    pid_real_t a1, a2;
} pid_biquad_coeffs_t;

/**
 * @brief Biquad Filter State
 */
typedef struct {
    pid_real_t x1, x2;  // Input history
    pid_real_t y1, y2;  // Output history
} pid_filter_biquad_state_t;

/**
 * @brief PID Configuration Structure
 * Parameters are read-only during runtime for safety.
 */
typedef struct {
    pid_real_t kp;  // Proportional Gain
    pid_real_t ki;  // Integral Gain
    pid_real_t kd;  // Derivative Gain
    pid_real_t kf;  // Feed-Forward Gain (Optional)

    pid_real_t out_max;  // Output Upper Limit
    pid_real_t out_min;  // Output Lower Limit

    pid_real_t deadband;  // Linear Deadband (0 to disable)
    pid_real_t max_rate;  // Maximum output change per second (0 to disable)

    // --- Advanced Features ---

    pid_real_t max_setpoint_ramp;             // Max setpoint change/sec (0 to disable)
    pid_real_t kw;                            // Anti-windup Tracking Gain (for BACK_CALC)
    pid_anti_windup_mode_t anti_windup_mode;  // Anti-windup Strategy

    // --- 2DOF (Two-Degree-of-Freedom) Weights ---
    // Proportional Setpoint Weight (0.0 to 1.0)
    // - 1.0 = Standard PID (Error based)
    // - 0.0 = I-PD (Proportional on Measurement only) - smoothest
    // NOTE: If beta != 1.0, P-term will ignore Deadband to maintain disturbance rejection.
    pid_real_t beta;

    // Derivative Setpoint Weight (0.0 to 1.0)
    // - 1.0 = Derivative on Error (Standard)
    // - 0.0 = Derivative on Measurement (Kick-free)
    pid_real_t gamma;

    // --- Filter Configuration ---

    // 1. Derivative Line Filter
    pid_real_t d_tau;        // Derivative Filter Time Constant (Seconds). Preferred over alpha.
    pid_real_t d_lpf_alpha;  // [DEPRECATED] Manual Alpha (0.0-1.0). Used if d_tau <= 0.
                             // NOTE: d_lpf_alpha will be clamped to [0.0, 1.0]

    // 2. Advanced Filter (Biquad) for Derivative Term
    // If set to PID_FILTER_BIQUAD, this overrides d_tau/alpha behavior for D-term.
    pid_filter_type_t d_filter_type;
    pid_biquad_coeffs_t d_biquad_coeffs;  // Pre-calculated coefficients for Biquad

    // 3. Input Filter Configuration (Optional)
    // Pre-filter applied to the Measurement (PV) before it enters the PID loop.
    // Useful for noise reduction or resonance suppression (Notch).
    pid_filter_type_t input_filter_type;      // Input Filter Type
    pid_real_t input_filter_tau;              // Input Filter Time Constant (for PID_FILTER_PT1)
    pid_biquad_coeffs_t input_biquad_coeffs;  // Input Filter Coeffs (for PID_FILTER_BIQUAD)

} pid_cfg_t;

/**
 * @brief PID Runtime Context
 * Internal state - User should NOT modify these directly.
 */
typedef struct {
    // Outputs
    pid_real_t out;  // Final Output

    // Internal State
    pid_real_t integral;      // Accumulated Integral
    pid_real_t prev_measure;  // Previous PV (for D-term)
    pid_real_t prev_error;    // Previous Error

    // Filter State
    pid_real_t d_lpf;                    // PT1 Filter State (Derivative)
    pid_filter_biquad_state_t d_biquad;  // Biquad Filter State (Derivative)

    pid_real_t input_lpf;                    // PT1 Filter State (Input)
    pid_filter_biquad_state_t input_biquad;  // Biquad Filter State (Input)

    pid_real_t internal_setpoint;  // Current internal setpoint (for Ramp Control)
    bool first_run;                // Flag to handle first-run initialization

    // Configuration Reference
    const pid_cfg_t *cfg;  // Pointer to configuration (must be persistent)
} pid_t;

/**
 * @brief Cascade PID Structure
 * Represents a Outer-Inner loop system.
 */
typedef struct {
    pid_t *outer;
    pid_t *inner;
} pid_cascade_t;

/**
 * @brief Initialize the PID controller
 * @param pid Pointer to PID context
 * @param cfg Pointer to configuration (Must be valid and persistent)
 * @return true if initialization successful, false otherwise
 */
bool pid_init(pid_t *pid, const pid_cfg_t *cfg);

/**
 * @brief Reset PID internal state (integral, history)
 * @param pid Pointer to PID context
 */
void pid_reset(pid_t *pid);

/**
 * @brief Update PID controller (Standard Interface)
 *
 * @param pid Pointer to PID context
 * @param setpoint Target value (SP)
 * @param measurement Process Variable (PV)
 * @param dt_seconds Time delta in seconds (> 0)
 * @return Calculated output
 */
pid_real_t pid_update(pid_t *pid, pid_real_t setpoint, pid_real_t measurement,
                      pid_real_t dt_seconds);

/**
 * @brief Update PID controller (Incremental / Velocity Form)
 * Returns the CHANGE in output (Delta U).
 *
 * @param pid Pointer to PID context
 * @param setpoint Target value (SP)
 * @param measurement Process Variable (PV)
 * @param dt_seconds Time delta in seconds (> 0)
 * @param current_output Current actual output (for anti-windup clamping)
 * @return Calculated output change (Delta U)
 */
pid_real_t pid_update_incremental(pid_t *pid, pid_real_t setpoint, pid_real_t measurement,
                                  pid_real_t dt_seconds, pid_real_t current_output);

/**
 * @brief Force set the integral term (Pre-loading)
 * @param pid Pointer to PID context
 * @param value Target integral value (will be clamped to output limits)
 */
void pid_set_integral(pid_t *pid, pid_real_t value);

/**
 * @brief Manual Mode Tracking (Bumpless Transfer)
 * @param pid Pointer to PID context
 * @param manual_output Current manual output value
 * @param measurement Current Process Variable (PV)
 * @param setpoint Current Target value (SP)
 */
void pid_track_manual(pid_t *pid, pid_real_t manual_output, pid_real_t measurement,
                      pid_real_t setpoint);

/* --- Helper Functions for Filter Coefficients --- */

/**
 * @brief Calculate Biquad Coefficients for Low Pass Filter (PT2)
 * @param coeffs Output coeffs structure
 * @param dt Time step in seconds
 * @param cutoff_freq Cutoff frequency in Hz
 * @param q_factor Q factor (0.707 for Butterworth)
 */
void pid_filter_calc_pt2(pid_biquad_coeffs_t *coeffs, pid_real_t dt, pid_real_t cutoff_freq,
                         pid_real_t q_factor);

/**
 * @brief Calculate Biquad Coefficients for Notch Filter
 * @param coeffs Output coeffs structure
 * @param dt Time step in seconds
 * @param center_freq Center frequency in Hz
 * @param bandwidth Bandwidth in Hz
 */
void pid_filter_calc_notch(pid_biquad_coeffs_t *coeffs, pid_real_t dt, pid_real_t center_freq,
                           pid_real_t bandwidth);

/* --- Cascade PID API --- */

bool pid_cascade_init(pid_cascade_t *cascade, pid_t *outer, pid_t *inner);

pid_real_t pid_cascade_update(pid_cascade_t *cascade, pid_real_t setpoint, pid_real_t outer_measure,
                              pid_real_t inner_measure, pid_real_t dt);

#ifdef __cplusplus
}
#endif

#endif  // PID_CORE_H
