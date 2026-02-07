#include <stddef.h>
#include <stdio.h>

#include "pid.h"


int main() {
    printf("sizeof(pid_real_t): %zu\n", sizeof(pid_real_t));
    printf("sizeof(pid_cfg_t): %zu\n", sizeof(pid_cfg_t));
    printf("Offset kp: %zu\n", offsetof(pid_cfg_t, kp));
    printf("Offset ki: %zu\n", offsetof(pid_cfg_t, ki));
    printf("Offset out_max: %zu\n", offsetof(pid_cfg_t, out_max));
    printf("Offset deadband: %zu\n", offsetof(pid_cfg_t, deadband));
    printf("Offset max_rate: %zu\n", offsetof(pid_cfg_t, max_rate));
    printf("Offset max_setpoint_ramp: %zu\n", offsetof(pid_cfg_t, max_setpoint_ramp));
    printf("Offset kw: %zu\n", offsetof(pid_cfg_t, kw));
    printf("Offset anti_windup_mode: %zu\n", offsetof(pid_cfg_t, anti_windup_mode));
    printf("Offset d_lpf_alpha: %zu\n", offsetof(pid_cfg_t, d_lpf_alpha));
    printf("Offset derivative_on_measurement: %zu\n",
           offsetof(pid_cfg_t, derivative_on_measurement));

    printf("\nsizeof(pid_t): %zu\n", sizeof(pid_t));
    printf("Offset out: %zu\n", offsetof(pid_t, out));
    printf("Offset integral: %zu\n", offsetof(pid_t, integral));

    return 0;
}
