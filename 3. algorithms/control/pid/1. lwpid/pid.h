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

// float or double
typedef float pid_real_t;
typedef uint32_t pid_tick_t;

/**
 * @brief Anti-windup Modes
 */
typedef enum {
    PID_ANTI_WINDUP_CONDITIONAL = 0,  // Conditional Integration (Default)
    PID_ANTI_WINDUP_BACK_CALC,        // Back-calculation (Tracking)
    PID_ANTI_WINDUP_CLAMP,            // Integral State Clamping
    PID_ANTI_WINDUP_NONE              // No Anti-windup
} pid_anti_windup_mode_t;

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

    // Advanced Features
    /* Advanced Features */
    pid_real_t max_setpoint_ramp;             // Max setpoint change/sec (0 to disable)
    pid_real_t kw;                            // Anti-windup Tracking Gain (for BACK_CALC)
    pid_anti_windup_mode_t anti_windup_mode;  // Anti-windup Strategy

    pid_real_t d_lpf_alpha;          // Derivative LPF Alpha (0.0 - 1.0)
    bool derivative_on_measurement;  // true: d(PV)/dt, false: d(Error)/dt
} pid_cfg_t;

/**
 * @brief PID Runtime Context
 * Internal state - User should NOT modify these directly.
 */
typedef struct {
    // Outputs
    pid_real_t out;  // Final Output

    // Internal State
    pid_real_t integral;           // Accumulated Integral
    pid_real_t prev_measure;       // Previous PV (for D-term)
    pid_real_t prev_error;         // Previous Error (for Incremental P-term)
    pid_real_t d_lpf;              // Derivative Low-Pass Filter State
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
 * Useful when system re-enables or recovers from error.
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
 * Useful for integrating actuators (like stepper motors).
 *
 * @param pid Pointer to PID context
 * @param setpoint Target value (SP)
 * @param measurement Process Variable (PV)
 * @param dt_seconds Time delta in seconds (> 0)
 * @return Calculated output change (Delta U)
 */
pid_real_t pid_update_incremental(pid_t *pid, pid_real_t setpoint, pid_real_t measurement,
                                  pid_real_t dt_seconds);

/**
 * @brief Force set the integral term (Pre-loading)
 * Useful for initializing the controller to a known state or recovering from specific conditions.
 *
 * @param pid Pointer to PID context
 * @param value Target integral value (will be clamped to output limits)
 */
void pid_set_integral(pid_t *pid, pid_real_t value);

/**
 * @brief Manual Mode Tracking (Bumpless Transfer)
 * Call this periodically when in Manual Mode to keep PID internal state
 * synchronized with the manual output.
 *
 * @param pid Pointer to PID context
 * @param manual_output Current manual output value
 * @param measurement Current Process Variable (PV)
 * @param setpoint Current Target value (SP)
 */
void pid_track_manual(pid_t *pid, pid_real_t manual_output, pid_real_t measurement,
                      pid_real_t setpoint);

/* --- Cascade PID API --- */

/**
 * @brief Initialize Cascade PID
 * @param cascade Pointer to cascade structure
 * @param outer Pointer to initialized outer PID
 * @param inner Pointer to initialized inner PID
 * @return true if successful
 */
bool pid_cascade_init(pid_cascade_t *cascade, pid_t *outer, pid_t *inner);

/**
 * @brief Update Cascade PID
 * @param cascade Pointer to cascade structure
 * @param setpoint Target for outer loop
 * @param outer_measure Measurement for outer loop
 * @param inner_measure Measurement for inner loop
 * @param dt Time step
 * @return Inner loop output
 */
pid_real_t pid_cascade_update(pid_cascade_t *cascade, pid_real_t setpoint, pid_real_t outer_measure,
                              pid_real_t inner_measure, pid_real_t dt);

#ifdef __cplusplus
}
#endif

#endif  // PID_CORE_H