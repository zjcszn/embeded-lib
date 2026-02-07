import subprocess
import os
import matplotlib.pyplot as plt
import csv
import sys

# Configuration
c_source = "simulation_main.c"
pid_source = "../pid.c"
output_exe = "pid_test"
if os.name == 'nt':
    output_exe += ".exe"

def compile_code():
    print(f"Compiling {c_source} and {pid_source}...")
    try:
        # Check if gcc is available
        subprocess.check_call(["gcc", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        # Compile
        cmd = ["gcc", c_source, pid_source, "-o", output_exe, "-I..", "-lm"]
        subprocess.check_call(cmd)
        print("Compilation successful.")
        return True
    except FileNotFoundError:
        print("Error: GCC not found. Please install GCC or compile manually.")
        return False
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e}")
        return False

def run_simulation():
    if not os.path.exists(output_exe):
        print(f"Executable {output_exe} not found.")
        return None

    print(f"Running simulation {output_exe}...")
    try:
        result = subprocess.run([os.path.abspath(output_exe)], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Execution failed: {e}")
        return None

def plot_results(csv_data):
    times = []
    setpoints = []
    measurements = []
    outputs = []
    integrals = []

    try:
        reader = csv.reader(csv_data.splitlines())
        header = next(reader) # Skip header
        
        for row in reader:
            if not row: continue
            # Format: Time,Setpoint,Measurement,Output,Integral
            times.append(float(row[0]))
            setpoints.append(float(row[1]))
            measurements.append(float(row[2]))
            outputs.append(float(row[3]))
            integrals.append(float(row[4]))
    except Exception as e:
        print(f"Error parsing CSV: {e}")
        return

    # Plot
    plt.figure(figsize=(12, 8))
    
    # Top Plot: PV vs SP
    plt.subplot(2, 1, 1)
    plt.plot(times, setpoints, 'r--', label='Target (SP)')
    plt.plot(times, measurements, 'b-', label='Actual (PV)', linewidth=2)
    plt.title('PID Response Step Test')
    plt.ylabel('Value')
    plt.grid(True, which='both', linestyle='--', alpha=0.7)
    plt.legend()

    # Bottom Plot: Output and Integral
    plt.subplot(2, 1, 2)
    plt.plot(times, outputs, 'g-', label='Output', alpha=0.8)
    plt.plot(times, integrals, 'y-', label='Integral Term', alpha=0.5)
    plt.xlabel('Time (s)')
    plt.ylabel('Control Signal')
    plt.grid(True, which='both', linestyle='--', alpha=0.7)
    plt.legend()

    # Add text for events
    plt.subplot(2, 1, 1)
    plt.axvline(x=1.0, color='k', linestyle=':', alpha=0.3)
    plt.text(1.1, 10, "Step Change", rotation=90, verticalalignment='bottom')
    
    plt.axvline(x=6.0, color='k', linestyle=':', alpha=0.3)
    plt.text(6.1, 10, "Load Disturbance (-20)", rotation=90, verticalalignment='bottom')

    plt.tight_layout()
    output_img = "pid_test_result.png"
    plt.savefig(output_img)
    print(f"Plot saved to {output_img}")

def main():
    if compile_code():
        output = run_simulation()
        if output:
            plot_results(output)
            print("Done.")

if __name__ == "__main__":
    main()
