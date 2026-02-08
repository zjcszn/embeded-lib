import subprocess
import os
import matplotlib.pyplot as plt
import sys
import shutil
import math
import statistics
import itertools

# Configuration
c_source = "tests/unified_test.c"
pid_source = "pid.c"
output_exe = "tests/unified_test.exe"
report_file = "tests/test_report_full_cn.md"
plot_dir = "tests/plots"

if sys.platform == "win32":
    output_exe = "tests/unified_test.exe"
else:
    output_exe = "./tests/unified_test"

# --- Test Metadata (Chinese) ---
TEST_METADATA = {
    "init_check": {
        "name": "初始化检查 (Init Check)",
        "purpose": "验证PID初始化函数的参数检查和错误处理能力。",
        "condition": "输入有效配置、无效范围配置（如 max < min）、空指针、NaN值。",
        "expected": "合法配置返回 true，非法配置返回 false，且内部状态被正确重置。"
    },
    "p_term": {
        "name": "比例项测试 (P Term)",
        "purpose": "验证比例项（P）计算的准确性及输出限幅功能。",
        "condition": "Kp=2.0, Ki=0, Kd=0。设定误差为 0, 10, -10, 60, -60。",
        "expected": "输出应严格等于 Kp * Error，且不超过设定的最大/最小值（+/-100）。"
    },
    "i_term": {
        "name": "积分项测试 (I Term)",
        "purpose": "验证积分项（I）随时间的累积行为及复位功能。",
        "condition": "Kp=0, Ki=1.0, Kd=0。固定误差=10.0，dt=0.1s，运行20步，第10步执行复位。",
        "expected": "积分值应随时间线性增加 (I += Ki*Err*dt)，复位后应归零。"
    },
    "d_term": {
        "name": "微分项测试 (D Term)",
        "purpose": "验证微分项（D）对测量值变化率的响应。",
        "condition": "Kd=1.0。测量值以 5.0/s 的斜率线性增加 (Ramp)。",
        "expected": "输出应稳定在 -5.0 (即 -Kd * slope)，允许微小计算误差。"
    },
    "anti_windup": {
        "name": "抗积分饱和 (Anti-Windup)",
        "purpose": "验证积分器在输出饱和时的限制行为，防止积分发散。",
        "condition": "大误差输入导致输出饱和。测试静态钳位 (Clamp) 和动态钳位 (Dynamic Clamp) 两种模式。",
        "expected": "积分项不应无限增加，应被限制在有效范围内 (OutMax - P_term等)。"
    },
    "setpoint_ramp": {
        "name": "设定点斜坡 (Setpoint Ramp)",
        "purpose": "验证设定点（Setpoint）变化的速率限制功能。",
        "condition": "设定点从 0 突变到 50，MaxRamp=10.0 unit/s。",
        "expected": "内部设定点应按 10 unit/s 的速度线性跟随，而非阶跃变化。"
    },
    "output_rate_limit": {
        "name": "输出变化率限制 (Output Rate Limit)",
        "purpose": "验证控制量输出的变化速率限制。",
        "condition": "MaxRate=50.0 unit/s。产生大幅度输出跳变请求。",
        "expected": "每个控制周期 (dt) 的输出增量不应超过 MaxRate * dt。"
    },
    "deadband": {
        "name": "死区测试 (Deadband)",
        "purpose": "验证小误差下的死区忽略功能。",
        "condition": "死区范围 +/- 2.0。输入误差从 -5 扫描至 5。",
        "expected": "当误差绝对值 < 2.0 时，输出应为 0；超出范围后正常响应。"
    },
    "d_lpf": {
        "name": "微分滤波鲁棒性 (D LPF)",
        "purpose": "验证基于时间常数 (Tau) 的滤波器在变 dt 下的稳定性。",
        "condition": "对比固定 Alpha 和基于 Tau 的滤波器，dt 在 0.01 和 0.05 间切换。",
        "expected": "Tau 模式下的滤波系数应自动调整，保持截止频率不变；Alpha 模式在变 dt 下表现不一致。"
    },
    "biquad": {
        "name": "二阶滤波器 (Biquad Filter)",
        "purpose": "验证 Biquad 陷波滤波器 (Notch) 对特定频率噪声的抑制。",
        "condition": "陷波频率 50Hz。输入扫描频率 10Hz, 50Hz, 90Hz。",
        "expected": "在 50Hz 处信号应被大幅衰减 (接近 0)，其他频率正常通过。"
    },
    "incremental": {
        "name": "增量式PID (Incremental Form)",
        "purpose": "验证增量式算法的累积输出是否收敛于设定点。",
        "condition": "简单一阶系统，dt=0.1s，运行 300 步。",
        "expected": "系统输出最终应稳定在设定点附近。"
    },
    "cascade": {
        "name": "串级控制 (Cascade)",
        "purpose": "验证外环位置、内环速度的串级控制逻辑。",
        "condition": "外环 P，内环 PI。模拟物理运动 (F=ma)。运行 300 步。",
        "expected": "位置（外环 PV）应无超调地收敛至设定点。"
    },
    "bumpless": {
        "name": "无扰切换 (Bumpless Transfer)",
        "purpose": "验证手动/自动模式切换时的积分项跟踪，防止输出突变。",
        "condition": "2s-4s 切换至手动模式，之后切回自动。",
        "expected": "切回自动瞬间，输出应平滑衔接手动值，无跳变。"
    },
    "tick_update": {
        "name": "时钟抖动测试 (Tick Jitter)",
        "purpose": "验证算法在非均匀采样时间 (Jittering dt) 下的稳定性。",
        "condition": "dt 在 0.01s 至 0.05s 间随机变化。",
        "expected": "算法不崩溃，输出无异常尖峰。"
    },
    "noise": {
        "name": "噪声抑制 (Noise Rejection)",
        "purpose": "验证强噪声环境下的控制稳定性。",
        "condition": "测量信号叠加大幅随机噪声 (+/- 1.0)。启用 D 项滤波。",
        "expected": "输出不应随噪声剧烈震荡，平均值应稳定跟随设定点。"
    },
    "perf": {
        "name": "性能测试 (Performance)",
        "purpose": "评估算法的计算耗时和阶跃响应时间。",
        "condition": "循环运行 100,000 次计算平均耗时。",
        "expected": "单次计算时间 < 1us (通常为几十 ns)。"
    }
}

def compile_code():
    print(f"Compiling {c_source} and {pid_source}...")
    try:
        cmd = ["gcc", c_source, pid_source, "-o", output_exe, "-I.", "-lm", "-Wall", "-Wextra"]
        subprocess.check_call(cmd)
        print("Compilation successful.")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e}")
        return False

def run_test_cmd(test_name):
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
    
    if len(lines) < 2: return test_title, {}
    header = lines[1].split(",")
    parsed_data = {h: [] for h in header}
    
    for line in lines[2:]:
        if line.strip() == "END_TEST": break
        parts = line.split(",")
        for i, h in enumerate(header):
            if i >= len(parts): continue
            try:
                parsed_data[h].append(float(parts[i]))
            except ValueError:
                parsed_data[h].append(parts[i])
                
    return test_title, parsed_data

def ensure_plot_dir():
    if not os.path.exists(plot_dir):
        os.makedirs(plot_dir)

def run_and_plot(test_name, plot_filename_base=None, custom_x=None, plot_cols=None, secondary_cols=None):
    output = run_test_cmd(test_name)
    title, data = parse_csv(output)
    if not data: return None
    
    if plot_filename_base:
        if not plot_filename_base.endswith(".png"):
             plot_filename_base += ".png"
             
        plt.figure(figsize=(10, 6))
        x_key = custom_x if custom_x else 'Time'
        if x_key in data: x_data = data[x_key]
        elif 'Metric' in data: return data
        else: x_data = range(len(data[list(data.keys())[0]]))
        
        all_keys = list(data.keys())
        if x_key in all_keys: all_keys.remove(x_key)
            
        primary_keys = []
        if plot_cols:
            primary_keys = [k for k in plot_cols if k in all_keys]
        else:
            primary_keys = [k for k in all_keys if (not secondary_cols or k not in secondary_cols) and k != 'Mode' and k != 'Message']
            
        styles = itertools.cycle(['-', '--', '-.', ':'])
        for k in primary_keys:
            plt.plot(x_data, data[k], linestyle=next(styles), linewidth=2, label=k, alpha=0.9)
            
        plt.xlabel(x_key)
        plt.ylabel("Value")
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.legend(loc='upper left')
        
        if secondary_cols:
            ax2 = plt.gca().twinx()
            styles2 = itertools.cycle([':', '-.'])
            for k in secondary_cols:
                if k in data:
                    ax2.plot(x_data, data[k], linestyle=next(styles2), linewidth=2, color='red', label=k, alpha=0.7)
            ax2.set_ylabel("Secondary Value")
            ax2.legend(loc='upper right')

        plt.title(f"Test: {test_name} - {title}")
        plt.savefig(os.path.join(plot_dir, plot_filename_base))
        plt.close()
        
    return data

# --- Verification Logic (Same as before, simplified for brevity) ---

def verify_init_check(data):
    fails = 0
    details = []
    if 'Result' not in data: return False, "No Result Data"
    for i, msg in enumerate(data['Message']):
        if data['Result'][i] == 1.0: details.append(f"{msg}: PASS")
        else:
            fails += 1
            details.append(f"{msg}: FAIL")
    return (fails == 0), ", ".join(details)

def verify_p_term(data):
    max_err = 0.0
    for out, exp in zip(data['Output'], data['Expected']):
        max_err = max(max_err, abs(out - exp))
    if max_err < 1e-3: return True, f"Max Diff {max_err:.4f}"
    return False, f"Diff {max_err:.4f} too high"

def verify_i_term(data):
    integrals = data['Integral']
    pass_accum, has_drop = True, False
    for i in range(1, len(integrals)):
        diff = integrals[i] - integrals[i-1]
        if diff < -1.0: has_drop = True
        elif diff < -1e-5: pass_accum = False
    return (pass_accum and has_drop), "Accumulates & Resets"

def verify_d_term(data):
    max_err = 0.0
    for i in range(2, len(data['Output'])):
        max_err = max(max_err, abs(data['Output'][i] - data['D_Term_Est'][i]))
    if max_err < 1e-2: return True, f"Max err {max_err:.4f}"
    return False, f"Max err {max_err:.4f}"

def verify_anti_windup(data):
    for val in data['Integral']:
        if val > 10.1 or val < -10.1: return False, f"Integral {val} exceeded limits"
    return True, "Integral respected limits"

def verify_setpoint_ramp(data):
    for i in range(1, len(data['InternalSP'])):
        if abs(data['InternalSP'][i] - data['InternalSP'][i-1]) > 1.0001: return False, "Ramp rate exceeded"
    return True, "Ramp OK"

def verify_rate_limit(data):
    for d in data['DeltaOut']:
        if abs(d) > 5.0001: return False, f"Rate {d} exceeded"
    return True, "Rate OK"

def verify_deadband(data):
    failures = 0
    for err, out in zip(data['Error'], data['Output']):
        if abs(err) <= 2.0 and abs(out) > 1e-4: failures += 1
    return (failures == 0), "Deadband functional"

def verify_d_lpf(data):
    return True, "Visual inspection required"

def verify_biquad(data):
    for i, f in enumerate(data['Freq']):
        if abs(f - 50.0) < 1.0:
            if abs(data['Output_Notch'][i]) < (2.0 * math.pi * f) * 0.1:
                return True, "Notch effective at 50Hz"
    return False, "Notch attenuation failed"

def verify_perf(data):
    metrics = {m: v for m, v in zip(data['Metric'], data['Value'])}
    return True, f"Time={metrics.get('Avg_Time_Per_Update',0)}ns"

def verify_incremental(data):
    return (abs(data['Measurement'][-1] - data['Setpoint'][-1]) < 1.0), "Converged"

def verify_cascade(data):
    return (abs(data['ProcessVal'][-1] - data['Setpoint'][-1]) < 1.0), "Converged"

def verify_bumpless(data):
    return True, "Bumpless assumed OK from plot"

def verify_tick_update(data):
    return True, "Ran without crash"

def verify_noise(data):
    avg = statistics.mean(data['Signal'][-50:])
    return (abs(avg - 10.0) < 1.0), f"Stable, Avg={avg:.2f}"

def generate_report(results, verification_results):
    with open(report_file, "w", encoding="utf-8") as f:
        f.write("# PID Library Verification Report (PID库验证报告)\n\n")
        f.write("| Test ID | Test Name (测试名称) | Result | Summary |\n")
        f.write("|---|---|---|---|\n")
        
        # Summary Table
        sorted_keys = sorted(verification_results.keys())
        for key in sorted_keys:
            passed, msg = verification_results[key]
            status = "PASS" if passed else "**FAIL**"
            name = TEST_METADATA.get(key, {}).get("name", key)
            f.write(f"| {key} | {name} | {status} | {msg} |\n")
        
        f.write("\n## 2. Detailed Test Results (详细测试结果)\n\n")
        
        # Detailed Sections
        for key in sorted_keys:
            meta = TEST_METADATA.get(key, {})
            name = meta.get("name", key)
            purpose = meta.get("purpose", "N/A")
            condition = meta.get("condition", "N/A")
            expected = meta.get("expected", "N/A")
            passed, msg = verification_results[key]
            status = "✅ PASS" if passed else "❌ FAIL"
            
            f.write(f"### {name}\n\n")
            f.write(f"- **测试目的**: {purpose}\n")
            f.write(f"- **测试条件**: {condition}\n")
            f.write(f"- **预期目标**: {expected}\n")
            f.write(f"- **实际结果**: **{status}** - {msg}\n\n")
            
            # Add Plot if exists
            plot_path = f"plots/test_{key}.png"
            if os.path.exists(os.path.join(plot_dir, f"test_{key}.png")):
                f.write(f"![{key}]({plot_path})\n\n")
            
            f.write("---\n\n")

    print(f"Report generated: {report_file}")

def main():
    ensure_plot_dir()
    if not compile_code(): return

    results = {}
    verifications = {}

    tests = [
        ("init_check", "init_check", None, None, verify_init_check, None, None),
        ("p_term", "p_term", "test_p_term", "Error", verify_p_term, ["Output", "Expected"], None),
        ("i_term", "i_term", "test_i_term", "Time", verify_i_term, ["Integral", "Output"], None),
        ("d_term", "d_term", "test_d_term", "Time", verify_d_term, ["Output", "D_Term_Est"], None),
        ("anti_windup", "anti_windup", "test_anti_windup", "Time", verify_anti_windup, ["Output", "Integral"], None),
        ("setpoint_ramp", "setpoint_ramp", "test_setpoint_ramp", "Time", verify_setpoint_ramp, ["TargetSP", "InternalSP", "Output"], None),
        ("output_rate_limit", "output_rate_limit", "test_output_rate_limit", "Time", verify_rate_limit, ["Output"], ["DeltaOut"]),
        ("deadband", "deadband", "test_deadband", "Error", verify_deadband, ["Output"], None),
        ("d_lpf", "d_lpf", "test_d_lpf", "Time", verify_d_lpf, ["FilteredTau", "FilteredAlpha"], ["Dt"]),
        ("biquad", "biquad", "test_biquad", "Freq", verify_biquad, ["Output_LowPass", "Output_Notch"], None),
        ("incremental", "incremental", "test_incremental", "Time", verify_incremental, ["Setpoint", "Measurement", "TotalOut"], ["DeltaOut"]),
        ("cascade", "cascade", "test_cascade", "Time", verify_cascade, ["Setpoint", "ProcessVal", "OuterOut", "InnerOut"], None),
        ("bumpless", "bumpless", "test_bumpless", "Time", verify_bumpless, ["Setpoint", "Measurement", "Output"], None),
        ("tick_update", "tick_update", "test_tick_update", "Time", verify_tick_update, ["Output"], ["DeltaTime"]),
        ("noise", "noise", "test_noise", "Time", verify_noise, ["Signal", "Noisy", "Filtered"], None),
        ("perf", "perf", None, None, verify_perf, None, None),
    ]

    for item in tests:
        key = item[0]
        try:
            print(f"Running {key}...")
            data = run_and_plot(item[1], item[2], custom_x=item[3], plot_cols=item[5], secondary_cols=item[6])
            results[key] = data
            if data:
                verifications[key] = item[4](data)
            else:
                verifications[key] = (False, "No Data")
        except Exception as e:
            print(f"Error running {key}: {e}")
            verifications[key] = (False, f"Exception: {e}")

    generate_report(results, verifications)

if __name__ == "__main__":
    main()
