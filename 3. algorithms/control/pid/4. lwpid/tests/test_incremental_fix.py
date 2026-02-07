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
        ("max_rate", ctypes.c_float),  # New field
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
        ("last_tick", ctypes.c_uint32),
        ("first_run", ctypes.c_bool),
        ("cfg", ctypes.POINTER(PIDConfig)),
    ]

def run_test():
    # Load the shared library
    # Assuming pid.dll or pid.so exists. If not, we might need to compile it first.
    # For now, let's assume we can compile it using gcc.
    
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
    
    # Setup function signatures
    pid_lib.pid_init.argtypes = [ctypes.POINTER(PIDContext), ctypes.POINTER(PIDConfig)]
    pid_lib.pid_init.restype = ctypes.c_bool
    
    pid_lib.pid_update_incremental.argtypes = [ctypes.POINTER(PIDContext), ctypes.c_float, ctypes.c_float, ctypes.c_float]
    pid_lib.pid_update_incremental.restype = ctypes.c_float

    # Create config
    cfg = PIDConfig()
    cfg.kp = 1.0
    cfg.ki = 0.0
    cfg.kd = 0.0
    cfg.out_max = 100.0
    cfg.out_min = -100.0 # Standard range
    cfg.max_rate = 10.0  # Limit change to 10 per second
    
    pid = PIDContext()
    
    # Init
    pid_lib.pid_init(ctypes.byref(pid), ctypes.byref(cfg))
    
    # Test 1: First run should be 0 change
    delta = pid_lib.pid_update_incremental(ctypes.byref(pid), 10.0, 0.0, 0.1)
    print(f"Test 1 (First Run): Delta = {delta} (Expected 0.0)")
    
    # Test 2: Second run, Error = 10, PrevError = 10. Kp * (Err - PrevErr) = 0.
    # We need to change error to see P action.
    # Let's say measurement moves to 5. Error becomes 5.
    # Delta Error = 5 - 10 = -5.
    # P_diff = 1.0 * -5 = -5.
    # Rate limit check: max_rate = 10, dt = 0.1 -> max_change = 1.0
    # Expected delta clamped: -1.0
    
    delta = pid_lib.pid_update_incremental(ctypes.byref(pid), 10.0, 5.0, 0.1)
    print(f"Test 2 (Negative Step): Delta = {delta} (Expected -1.0 due to rate limit)")
    
    if abs(delta - (-1.0)) < 0.001:
        print("PASS: Rate limiting worked correctly for negative change")
    else:
        print(f"FAIL: Expected -1.0, got {delta}")

    # Test 3: Small change within limit
    # Measurement moves to 5.1. Error = 4.9. Prev Error = 5.0.
    # Delta Error = -0.1. P_diff = -0.1.
    # Limit = 1.0. 
    # Expected: -0.1
    delta = pid_lib.pid_update_incremental(ctypes.byref(pid), 10.0, 5.1, 0.1)
    print(f"Test 3 (Small Step): Delta = {delta} (Expected -0.1)")

    if abs(delta - (-0.1)) < 0.001:
        print("PASS: Small change not limiting worked correctly")
    else:
         print(f"FAIL: Expected -0.1, got {delta}")

if __name__ == "__main__":
    run_test()
