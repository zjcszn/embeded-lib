import ctypes
import os
import sys

# Define structures to match C
class PIDConfig(ctypes.Structure):
    _fields_ = [
        ("kp", ctypes.c_float),
        ("ki", ctypes.c_float),
        ("kd", ctypes.c_float),
        ("kf", ctypes.c_float),
        ("out_max", ctypes.c_float),
        ("out_min", ctypes.c_float),
        ("deadband", ctypes.c_float),
        ("max_rate", ctypes.c_float),
        ("max_setpoint_ramp", ctypes.c_float), # New
        ("kw", ctypes.c_float),                # New
        ("anti_windup_mode", ctypes.c_int),    # New (enum is int)
        ("d_lpf_alpha", ctypes.c_float),
        ("derivative_on_measurement", ctypes.c_bool),
    ]

class PIDContext(ctypes.Structure):
    _fields_ = [
        ("out", ctypes.c_float),
        ("integral", ctypes.c_float),
        ("prev_measure", ctypes.c_float),
        ("prev_error", ctypes.c_float),
        ("d_lpf", ctypes.c_float),
        ("internal_setpoint", ctypes.c_float), # New
        ("last_tick", ctypes.c_uint32),
        ("first_run", ctypes.c_bool),
        ("cfg", ctypes.POINTER(PIDConfig)),
    ]

# Enum values
PID_ANTI_WINDUP_CONDITIONAL = 0
PID_ANTI_WINDUP_BACK_CALC = 1

def run_test():
    if os.name == 'nt':
        lib_name = 'pid.dll'
        compiler = 'gcc'
    else:
        lib_name = 'pid.so'
        compiler = 'gcc'

    # Compile source
    src_path = os.path.join(os.path.dirname(__file__), "..", "pid.c")
    cmd = f"{compiler} -shared -o {lib_name} -I.. {src_path}"
    print(f"Compiling: {cmd}")
    os.system(cmd)
    
    if not os.path.exists(lib_name):
        print("Failed to compile PID library")
        return

    pid_lib = ctypes.CDLL(os.path.abspath(lib_name))
    
    pid_lib.pid_init.argtypes = [ctypes.POINTER(PIDContext), ctypes.POINTER(PIDConfig)]
    pid_lib.pid_init.restype = ctypes.c_bool
    
    pid_lib.pid_update.argtypes = [ctypes.POINTER(PIDContext), ctypes.c_float, ctypes.c_float, ctypes.c_float]
    pid_lib.pid_update.restype = ctypes.c_float

    # --- Test 1: Setpoint Ramp ---
    print("\n--- Test Setpoint Ramp ---")
    cfg = PIDConfig()
    cfg.kp = 1.0
    cfg.ki = 0.0
    cfg.out_max = 100.0
    cfg.out_min = -100.0
    cfg.max_setpoint_ramp = 10.0 # 10 units per second
    
    pid = PIDContext()
    pid_lib.pid_init(ctypes.byref(pid), ctypes.byref(cfg))
    
    # Step 1: Initial (setpoint 0, measure 0). Should init internal setpoint to 0.
    pid_lib.pid_update(ctypes.byref(pid), 0.0, 0.0, 0.1)
    
    # Step 2: Setpoint jump to 100. dt = 0.1s. Max change = 10 * 0.1 = 1.0.
    # Output = Kp * (InternalSetpoint - Measure) = 1.0 * (1.0 - 0) = 1.0
    out = pid_lib.pid_update(ctypes.byref(pid), 100.0, 0.0, 0.1)
    print(f"Ramp Step 1: Out = {out} (Expected 1.0)")
    
    if abs(out - 1.0) < 0.01:
        print("PASS: Ramp limited correctly")
    else:
        print(f"FAIL: Expected 1.0, got {out}")

    # --- Test 2: Back-calculation Anti-windup ---
    print("\n--- Test Back-calculation ---")
    cfg_aw = PIDConfig()
    cfg_aw.kp = 1.0
    cfg_aw.ki = 10.0
    cfg_aw.out_max = 10.0 # Low max to saturate easily
    cfg_aw.out_min = -10.0
    cfg_aw.anti_windup_mode = PID_ANTI_WINDUP_BACK_CALC
    cfg_aw.kw = 1.0 # Moderate tracking gain
    
    pid_aw = PIDContext()
    pid_lib.pid_init(ctypes.byref(pid_aw), ctypes.byref(cfg_aw))
    
    # Step 1: Large error. SP=20, PV=0. Error=20.
    # P=20. Max=10. Saturated.
    # Unclamped = 20. Out = 10. Diff = 10 - 20 = -10.
    # I_change = Ki*Err*dt + Kw*Diff*dt = 10*20*0.1 + 1.0*(-10)*0.1 = 20 - 1 = 19.
    # Wait, assuming integration happens.
    
    out = pid_lib.pid_update(ctypes.byref(pid_aw), 20.0, 0.0, 0.1)
    print(f"Back-calc Step 1: Out={out}, Integral={pid_aw.integral}")
    
    # Pure accumulation would be 20.0 (Ki*Err*dt = 10*20*0.1).
    # With Back-calc, Kw=1.0, Out=10, Unclamped=40. Diff=-30. Correction = -3.0.
    # Final = 17.0.
    if abs(pid_aw.integral - 17.0) < 0.1:
         print("PASS: Back-calculation reduced integral accumulation")
    else:
         print(f"FAIL: Expected ~17.0, got {pid_aw.integral}")

if __name__ == "__main__":
    run_test()
