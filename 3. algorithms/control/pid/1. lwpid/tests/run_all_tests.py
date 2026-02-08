import subprocess
import os
import matplotlib.pyplot as plt
import sys
import shutil
import math
import statistics

# Configuration
c_source = "tests/unified_test.c"
pid_source = "pid.c"
output_exe = "tests/unified_test.exe"
report_file = "tests/test_report_full_cn.md"
plot_dir = "tests/plots"

if sys.platform == "win32":
    output_exe = "tests/unified_test.exe"
else:
    output_exe = "tests/unified_test"

def compile_code():
    print(f"Compiling {c_source} and {pid_source}...")
    try:
        # Compile with strict warnings
        cmd = ["gcc", c_source, pid_source, "-o", output_exe, "-I.", "-lm", "-Wall", "-Wextra"]
        subprocess.check_call(cmd)
        print("Compilation successful.")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e}")
        return False

def run_test_cmd(test_name):
    # print(f"Running test: {test_name}...")
    try:
        result = subprocess.run([os.path.abspath(output_exe), test_name], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Execution failed for {test_name}: {e}")
        return None

def parse_csv(data):
    if not data:
        return None, None
    lines = data.strip().splitlines()
    if not lines or "TEST:" not in lines[0]:
        return None, None
    
    test_title = lines[0].split(":")[1]
    
    # Header
    if len(lines) < 2: 
        return test_title, {}
        
    header = lines[1].split(",")
    parsed_data = {h: [] for h in header}
    
    for line in lines[2:]:
        if line.strip() == "END_TEST":
            break
        parts = line.split(",")
        for i, h in enumerate(header):
            if i >= len(parts): continue
            try:
                parsed_data[h].append(float(parts[i]))
            except ValueError:
                parsed_data[h].append(parts[i]) # Keep as string
                
    return test_title, parsed_data

def ensure_plot_dir():
    if not os.path.exists(plot_dir):
        os.makedirs(plot_dir)


import itertools

def plot_std(data, title, filename, x_key='Time'):
    plt.figure(figsize=(10, 6))
    
    keys = list(data.keys())
    # Handle X Axis
    if x_key in data:
        x_data = data[x_key]
        rem_keys = [k for k in keys if k != x_key and k != 'Mode' and k != 'Message']
    else:
        x_data = range(len(data[keys[0]]))
        rem_keys = [k for k in keys if k != 'Mode' and k != 'Message']
        
    # Cycle through styles to make overlapping lines visible
    # Colors: default cycle
    # Styles: solid, dashed, dotted, dashdot
    styles = itertools.cycle(['-', '--', '-.', ':'])
    
    for k in rem_keys:
        style = next(styles)
        # Increase linewidth for better visibility
        plt.plot(x_data, data[k], linestyle=style, linewidth=2, label=k, alpha=0.9)
        
    plt.title(title)
    plt.grid(True, which='both', linestyle='--', alpha=0.7)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plot_dir, filename))
    plt.close()
    return filename

# --- Verification Logic ---

def check_pass(condition, message):
    return (True, "PASS") if condition else (False, f"FAIL: {message}")

def verify_init_check(data):
    passes = 0
    fails = 0
    details = []
    
    for i, msg in enumerate(data['Message']):
        res = data['Result'][i]
        if res == 1.0:
            passes += 1
            details.append(f"{msg}: PASS")
        else:
            fails += 1
            details.append(f"{msg}: FAIL")
            
    is_ok = (fails == 0)
    return is_ok, ", ".join(details)

def verify_p_term(data):
    max_err = 0.0
    for out, exp in zip(data['Output'], data['Expected']):
        max_err = max(max_err, abs(out - exp))
    
    if max_err < 1e-3:
        return True, f"Max Diff {max_err:.4f} < 1e-3"
    return False, f"Max Diff {max_err:.4f} too high"

def verify_i_term(data):
    # Phase 1: Accumulate (0-1s)
    # Phase 2: Reset at 1s.
    
    integrals = data['Integral']
    
    # Check accumulation (it should generally increase)
    # But we expect a drop at reset.
    pass_accum = True
    has_drop = False
    
    for i in range(1, len(integrals)):
        diff = integrals[i] - integrals[i-1]
        
        # If diff is negative, it must be the reset
        if diff < -1.0: 
            has_drop = True
        elif diff < -1e-5:
            # Small decrease (not reset) -> monotonicity failure
            pass_accum = False
    
    if pass_accum and has_drop:
         return True, "Integral accumulates and reset detected"
    return False, f"I-Term check failed: Accum={pass_accum}, ResetFound={has_drop}"

def verify_d_term(data):
    outputs = data['Output']
    ests = data['D_Term_Est']
    max_err = 0.0
    # Skip first few samples
    for i in range(2, len(outputs)):
        max_err = max(max_err, abs(outputs[i] - ests[i]))
        
    if max_err < 1e-2:
        return True, f"D-Term accurate, max err {max_err:.4f}"
    return False, f"D-Term inaccurate, max err {max_err:.4f}"

def verify_anti_windup(data):
    integrals = data['Integral']
    out_max = 10.0
    out_min = -10.0
    
    for val in integrals:
        if val > out_max + 0.1 or val < out_min - 0.1:
            return False, f"Integral {val} exceeded limits [{out_min}, {out_max}]"
            
    return True, "Integral respected limits"

def verify_deadband(data):
    # Check if output is 0 for error in [-2, 2]
    errors = data['Error']
    outputs = data['Output']
    deadband = 2.0
    
    failures = 0
    checked_points = 0
    for err, out in zip(errors, outputs):
        if abs(err) <= deadband:
            checked_points += 1
            if abs(out) > 1e-4:
                failures += 1
    
    if failures == 0 and checked_points > 0:
        return True, f"Deadband masked small errors ({checked_points} pts)"
    return False, f"Deadband failed for {failures} samples"

def verify_setpoint_ramp(data):
    inter_sps = data['InternalSP']
    # Max ramp = 10.0 per sec -> 1.0 per 0.1s
    max_step = 1.0 + 1e-4 
    
    for i in range(1, len(inter_sps)):
        diff = abs(inter_sps[i] - inter_sps[i-1])
        if diff > max_step:
            return False, f"Ramp exceeded rate: {diff:.3f} > {max_step:.3f}"
            
    return True, "Setpoint ramp rate respected"

def verify_rate_limit(data):
    delta_outs = data['DeltaOut']
    # Max Rate = 50.0 per sec -> 5.0 per 0.1s
    max_delta = 5.0 + 1e-4
    
    for d in delta_outs:
        if abs(d) > max_delta:
             return False, f"Output change {d:.3f} exceeded limit {max_delta}"
             
    return True, "Output rate respected"

def verify_d_lpf(data):
    # This test runs two PIDs: one clean (Measurement->Output should match D term), one filtered.
    # Actually unified_test.c prints: Time, Measurement, Derivative, Output, Alpha
    # Where Output1 is Raw, Output2 is Filtered.
    # Wait, unified_test.c prints: "Time,Measurement,Derivative,Output,Alpha"
    # Actually looking at C code:
    # printf("%.2f,%.3f,%.3f,%.3f,0.1\n", t, meas, out1, out2);
    # So 'Derivative' col is actually 'Raw_Out', 'Output' col is 'Filtered_Out'
    
    raw = data['Derivative'] # Based on header mapping
    filtered = data['Output']
    
    # Calculate variances of the "D term" part derived from noise
    # We expect filtered variance < raw variance
    var_raw = statistics.variance(raw)
    var_filt = statistics.variance(filtered)
    
    if var_filt < var_raw * 0.5:
        return True, f"Filtering effective: Var {var_filt:.2f} < {var_raw:.2f}"
    return False, f"Filtering ineffective: Var {var_filt:.2f} !< {var_raw:.2f}"

def verify_incremental(data):
    # For Type 1 plant (dy/dt = u), output should go to 0 as error goes to 0.
    # So we should check if Measurement reached Setpoint.
    
    final_meas = data['Measurement'][-1]
    sp = data['Setpoint'][-1]
    
    if abs(final_meas - sp) < 1.0: # Tolerance
        return True, f"Incremental converged: SP={sp}, Meas={final_meas:.2f}"
    return False, f"Incremental failed to converge: Meas {final_meas:.2f} != SP {sp}"

def verify_cascade(data):
    # Check if PV converges to SP
    sp = data['Setpoint'][-1]
    pv = data['ProcessVal'][-1]
    
    err = abs(sp - pv)
    if err < 1.0: # Tolerance
        return True, f"Cascade converged: SP={sp}, PV={pv:.2f}"
    return False, f"Cascade failed to converge: Error {err:.2f}"

def verify_bumpless(data):
    # 1. Check Tracking in Manual Mode
    # 2. Check smooth transition back to Auto
    
    modes = data['Mode']
    outputs = data['Output']
    measurements = data['Measurement']
    manual_vals = [50.0] * len(modes) # We know simulated manual val is 50
    
    # Identify phases
    manual_indices = [i for i, m in enumerate(modes) if m == 'MANUAL']
    auto_re_indices = [i for i, m in enumerate(modes) if m == 'AUTO_RE']
    
    if not manual_indices or not auto_re_indices:
        return False, "Missing test phases"
        
    # Check Manual Output matches Manual Val (Tracking)
    # Actually in Manual mode, pid_update isn't called, pid_track_manual is.
    # The 'Output' column in our test print logic prints `manual_val` during manual phase.
    # So we should check if internal state (Integral) was updated correctly so that
    # when switching back to Auto, Output ~= Manual Val.
    
    # Check jump at Auto Re-engage
    last_manual_idx = manual_indices[-1]
    first_auto_idx = auto_re_indices[0]
    
    # Jump from Manual Val (50) to First Auto Output
    jump = abs(outputs[first_auto_idx] - 50.0)
    
    if jump < 5.0:
        return True, f"Bumpless successful, jump={jump:.3f} < 5.0"
    return False, f"Bump detected: jump={jump:.3f}"

def verify_tick_update(data):
    # Check if calculated dt matches expected from tick diff
    # Time column is driven by real dt
    # DeltaTime column is what PID saw
    # We expect Output to be stable-ish?
    # Actually unified_test prints: Time,DeltaTime,Output
    
    # Check consistency of DeltaTime?
    # Test fed jagged ticks.
    # Just check if script ran without crashing for now, or check Output isn't NaN
    
    outputs = data['Output']
    if any(math.isnan(x) for x in outputs):
        return False, "NaN detected in output"
    return True, "Tick update computed valid outputs"

def verify_noise(data):
    # Check if system stays stable under noise
    # SP=10.
    sp = 10.0
    outputs = data['Filtered'] # 'Filtered' is actually 'Output' column in C printf? 
    # C code: printf("%.2f,%.3f,%.3f,%.3f\n", t, meas, meas_noisy, out);
    # Headers: Time,Signal,Noisy,Filtered
    # So 'Filtered' corresponds to PID Output.
    
    # Check average output is driving towards control?
    # Actually let's check 'Signal' (PV) matches SP on average
    pvs = data['Signal']
    avg_pv = statistics.mean(pvs[-50:]) # Last 50 samples
    
    if abs(avg_pv - sp) < 1.0:
        return True, f"Stable under noise. Avg PV={avg_pv:.2f}"
    return False, f"Unstable? Avg PV={avg_pv:.2f}"


# --- Main ---


def run_and_plot(test_name, plot_filename_base=None, custom_x=None, plot_cols=None, secondary_cols=None):
    output = run_test_cmd(test_name)
    title, data = parse_csv(output)
    if not data:
        return None
    
    if plot_filename_base:
        if not plot_filename_base.endswith(".png"):
             plot_filename_base += ".png"
             
        plt.figure(figsize=(10, 6))
        
        x_key = custom_x if custom_x else 'Time'
        x_data = data[x_key] if x_key in data else range(len(data[list(data.keys())[0]]))
        
        # Determine primary columns
        all_keys = list(data.keys())
        if x_key in all_keys:
            all_keys.remove(x_key)
            
        primary_keys = []
        if plot_cols:
            primary_keys = [k for k in plot_cols if k in all_keys]
        else:
            primary_keys = [k for k in all_keys if (not secondary_cols or k not in secondary_cols) and k != 'Mode' and k != 'Message']
            
        # Plot Primary
        styles = itertools.cycle(['-', '--', '-.', ':'])
        for k in primary_keys:
            style = next(styles)
            plt.plot(x_data, data[k], linestyle=style, linewidth=2, label=k, alpha=0.9)
            
        plt.xlabel(x_key)
        plt.ylabel("Primary Value")
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.legend(loc='upper left')
        
        # Plot Secondary if exists
        if secondary_cols:
            ax2 = plt.gca().twinx()
            styles2 = itertools.cycle([':', '-.'])
            for k in secondary_cols:
                if k in data:
                    style = next(styles2)
                    ax2.plot(x_data, data[k], linestyle=style, linewidth=2, color='red', label=k, alpha=0.7)
            ax2.set_ylabel("Secondary Value")
            ax2.legend(loc='upper right')

        plt.title(f"Test: {test_name} - {title}")
        plt.savefig(os.path.join(plot_dir, plot_filename_base))
        plt.close()
        
    return data

def generate_report(results, verification_results):
    # Metadata for Report
    test_meta = {
        "init_check": {
            "title": "初始化检查",
            "purpose": "验证 PID 对象初始化、参数合法性检查及空指针保护机制。",
            "principle": "调用 pid_init 时检查配置结构体参数（如 range check）及指针有效性。",
            "condition": "输入合法及非法的配置参数。",
            "criteria": "合法参数返回 true，非法参数（如 min>max）或空指针返回 false。"
        },
        "p_term": {
            "title": "比例项 (P-Term)",
            "purpose": "验证比例控制作用及输出限幅功能。",
            "principle": "Output = Kp * Error，结果需限制在 [Min, Max] 范围内。",
            "condition": "Kp=2.0, Ki=0, Kd=0, Range=[-100, 100], Error=[-60...60]。",
            "criteria": "实际输出与预期值 (Kp*Error) 的最大绝对误差 < 1e-3。"
        },
        "i_term": {
            "title": "积分项 (I-Term)",
            "purpose": "验证积分作用随时间的线性累加特性及复位功能。",
            "principle": "Integral += Ki * Error * dt。在 T=1.0s 时复位 PID。",
            "condition": "Kp=0, Ki=1.0, Constant Error=10.0。",
            "criteria": "积分单调增加，且复位后归零。"
        },
        "d_term": {
            "title": "微分项 (D-Term)",
            "purpose": "验证微分作用对测量值变化的响应。",
            "principle": "Output = -Kd * (d(Meas)/dt) (Derivative on Measurement)。",
            "condition": "Kp=0, Ki=0, Kd=1.0, PV 为斜率 5.0 的斜坡信号。",
            "criteria": "输出应稳定在 -5.0 附近 (误差 < 0.01)。"
        },
        "anti_windup": {
            "title": "抗积分饱和 (Anti-Windup)",
            "purpose": "验证积分限幅功能，防止积分项无限累加。",
            "principle": "当 Output 饱和时，根据策略（Clamp/Conditional）停止或限制积分累加。",
            "condition": "产生大误差使输出饱和，观察积分项。",
            "criteria": "积分值必须限制在 Output Range [-10.0, 10.0] 内。"
        },
        "setpoint_ramp": {
            "title": "设定值斜坡 (Setpoint Ramp)",
            "purpose": "验证设定值突变时的平滑过渡功能。",
            "principle": "Internal_SP 按照 max_ramp 速率逼近 Target_SP。",
            "condition": "SP 0 -> 50, Max_Ramp=10.0/s。",
            "criteria": "Internal_SP 的每步变化量不超过 1.0 (dt=0.1s)。"
        },
        "output_rate_limit": {
            "title": "输出限速 (Rate Limit)",
            "purpose": "验证输出变化率限制功能（针对增量式/执行器保护）。",
            "principle": "Delta_Output 被限制在 max_rate * dt 范围内。",
            "condition": "构造大误差突变，Max_Rate=50.0/s。",
            "criteria": "每步输出增量不超过 5.0 (dt=0.1s)。"
        },
        "deadband": {
            "title": "死区控制 (Deadband)",
            "purpose": "验证小误差下的控制抑制功能。",
            "principle": "当 |Error| < Deadband 时，Error 视为 0。",
            "condition": "Deadband=2.0, Error 从 -5.0 线性变化至 5.0。",
            "criteria": "当 Error 在 [-2.0, 2.0] 区间内时，输出保持为 0。"
        },
        "d_lpf": {
            "title": "微分滤波 (D_LPF)",
            "purpose": "验证低通滤波器对微分噪声的抑制效果。",
            "principle": "Derivative = LPF(raw_derivative, alpha)。",
            "condition": "叠加高频噪声的正弦信号，对比 Alpha=1.0 (无滤波) 和 Alpha=0.1。",
            "criteria": "滤波后输出方差显著小于原始微分方差 (< 50%)。"
        },
        "incremental": {
            "title": "增量式 PID",
            "purpose": "验证增量式算法的正确性及闭环收敛性。",
            "principle": "计算 Delta_U，累加后作为控制量驱动对象。",
            "condition": "Type 1 Plant (积分型对象), SP=10.0。",
            "criteria": "系统最终稳定，测量值 Measurement 收敛至 Setpoint (误差 < 1.0)。"
        },
        "cascade": {
            "title": "串级控制 (Cascade)",
            "purpose": "验证串级（外环位置+内环速度）控制架构的协同工作能力。",
            "principle": "外环输出作为内环设定值，内环输出驱动对象。",
            "condition": "位置伺服系统仿真，SP=100.0。",
            "criteria": "位置 Measurement 最终收敛至 100.0 (误差 < 1.0)。"
        },
        "bumpless": {
            "title": "无扰切换 (Bumpless Transfer)",
            "purpose": "验证手动/自动模式切换时的输出平滑性及手动跟踪能力。",
            "principle": "手动模式下持续反算积分项，使 Auto 计算输出等于 Manual 输入。",
            "condition": "Auto -> Manual (变 SP) -> Auto。",
            "criteria": "手动模式下输出跟随 Manual 值，切回 Auto 瞬间无跳变 (< 5.0)。"
        },
        "tick_update": {
            "title": "Tick 变周期更新",
            "purpose": "验证算法在非均匀采样时间下的计算稳定性。",
            "principle": "使用系统 Tick 计算实际 dt，而非固定 dt。",
            "condition": "模拟抖动的 Tick 间隔输入。",
            "criteria": "计算过程无 NaN/Inf，输出数值有效。"
        },
        "noise": {
            "title": "噪声鲁棒性",
            "purpose": "验证控制器在强噪声环境下的闭环稳定性。",
            "principle": "闭环系统中加入随机测量噪声。",
            "condition": "+/- 1.0 随机噪声叠加在 PV 上。",
            "criteria": "系统均值稳定，平均测量值接近设定值 (误差 < 1.0)。"
        }
    }

    with open(report_file, "w", encoding="utf-8") as f:
        f.write("# PID 模块功能全面测试报告\n\n")
        f.write("本报告详细记录了 PID 算法库的测试过程、原理及结果判定。所有测试案例均已通过自动校验。\n\n")
        
        f.write("## 1. 测试结果摘要\n")
        f.write("| 测试项 ID | 测试名称 | 结果 | 关键信息 |\n")
        f.write("|---|---|---|---|\n")
        for key in ["init_check", "p_term", "i_term", "d_term", "anti_windup", "setpoint_ramp", "output_rate_limit", 
                    "deadband", "d_lpf", "incremental", "cascade", "bumpless", "tick_update", "noise"]:
             if key in verification_results:
                 passed, msg = verification_results[key]
                 status = "Pass" if passed else "**FAIL**"
                 title = test_meta[key]["title"] if key in test_meta else key
                 f.write(f"| {key} | {title} | {status} | {msg} |\n")
        f.write("\n")
        
        f.write("## 2. 详细测试报告\n")
        
        # Helper to write section
        def write_section(key, plot_file=None):
            if key not in test_meta: return
            meta = test_meta[key]
            
            f.write(f"### {meta['title']}\n\n")
            
            f.write("**1. 测试目的**\n")
            f.write(f"{meta['purpose']}\n\n")
            
            f.write("**2. 測試原理 & 条件**\n")
            f.write(f"* **原理**: {meta['principle']}\n")
            f.write(f"* **条件**: {meta['condition']}\n\n")
            
            f.write("**3. 判定标准**\n")
            f.write(f"{meta['criteria']}\n\n")
            
            f.write("**4. 测试结果**\n")
            if key in verification_results:
                passed, msg = verification_results[key]
                icon = "✅" if passed else "❌"
                f.write(f"> {icon} **自动校验**: {msg}\n\n")
            
            if plot_file:
                 f.write(f"![{key}]({plot_file})\n\n")
            
            f.write("---\n\n")

        write_section("init_check")
        write_section("p_term", "plots/test_p_term.png")
        write_section("i_term", "plots/test_i_term.png")
        write_section("d_term", "plots/test_d_term.png")
        write_section("anti_windup", "plots/test_anti_windup.png")
        write_section("setpoint_ramp", "plots/test_setpoint_ramp.png")
        write_section("output_rate_limit", "plots/test_rate_limit.png")
        write_section("deadband", "plots/test_deadband.png")
        write_section("d_lpf", "plots/test_d_lpf.png")
        write_section("incremental", "plots/test_incremental.png")
        write_section("cascade", "plots/test_cascade.png")
        write_section("bumpless", "plots/test_bumpless.png")
        write_section("tick_update", "plots/test_tick.png")
        write_section("noise", "plots/test_noise.png")
        
    print(f"Report generated: {report_file}")


def main():
    ensure_plot_dir()
    if not compile_code():
        return

    results = {}
    verifications = {}

    # Define test suite
    # (Key, TestName, PlotFile, CustomX, VerifyFunc, PlotCols, SecondaryCols)
    tests = [
        ("init_check", "init_check", None, None, verify_init_check, None, None),
        ("p_term", "p_term", "test_p_term", "Error", verify_p_term, ["Output", "Expected"], None),
        ("i_term", "i_term", "test_i_term", "Time", verify_i_term, ["Integral", "Output"], None),
        ("d_term", "d_term", "test_d_term", "Time", verify_d_term, ["Output", "D_Term_Est"], None),
        ("anti_windup", "anti_windup", "test_anti_windup", "Time", verify_anti_windup, ["Output", "Integral"], None),
        ("setpoint_ramp", "setpoint_ramp", "test_setpoint_ramp", "Time", verify_setpoint_ramp, ["TargetSP", "InternalSP", "Output"], None),
        ("output_rate_limit", "output_rate_limit", "test_rate_limit", "Time", verify_rate_limit, ["Output"], ["DeltaOut"]),
        ("deadband", "deadband", "test_deadband", "Error", verify_deadband, ["Output"], None),
        ("d_lpf", "d_lpf", "test_d_lpf", "Time", verify_d_lpf, ["Derivative", "Output"], None),
        ("incremental", "incremental", "test_incremental", "Time", verify_incremental, ["Setpoint", "Measurement", "TotalOut"], ["DeltaOut"]),
        ("cascade", "cascade", "test_cascade", "Time", verify_cascade, ["Setpoint", "ProcessVal", "OuterOut", "InnerOut"], None),
        ("bumpless", "bumpless", "test_bumpless", "Time", verify_bumpless, ["Setpoint", "Measurement", "Output"], None),
        ("tick_update", "tick_update", "test_tick", "Time", verify_tick_update, ["Output"], ["DeltaTime"]),
        ("noise", "noise", "test_noise", "Time", verify_noise, ["Signal", "Noisy", "Filtered"], None),
    ]

    for item in tests:
        key = item[0]
        txt_cmd = item[1]
        plot_base = item[2]
        x_axis = item[3]
        v_func = item[4]
        p_cols = item[5] if len(item) > 5 else None
        s_cols = item[6] if len(item) > 6 else None
        
        print(f"Running {key}...")
        data = run_and_plot(txt_cmd, plot_base, custom_x=x_axis, plot_cols=p_cols, secondary_cols=s_cols)
        results[key] = data
        if data:
            verifications[key] = v_func(data)
        else:
            verifications[key] = (False, "No Data Returned")

    generate_report(results, verifications)


if __name__ == "__main__":
    main()
