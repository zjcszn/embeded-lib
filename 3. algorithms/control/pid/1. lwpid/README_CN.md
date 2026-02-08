# 通用 Robust PID 控制库使用指引

本库提供了一个现代、健壮且通用的PID控制器实现，适用于嵌入式系统。它包含了抗积分饱和（Anti-windup）、微分项低通滤波（Derivative LPF）、测量值微分（Derivative on Measurement）、设定值斜坡（Setpoint Ramp）及串级PID（Cascade PID）等高级特性。

## 1. 功能结构

### 核心文件
- **`pid.h`**: 定义了PID的数据结构（配置结构体 `pid_cfg_t` 和 运行时结构体 `pid_t`）以及API接口。
- **`pid.c`**: 实现了PID的核心算法，包括位置式、增量式更新函数及串级控制逻辑。

### 主要特性
1.  **多模式抗积分饱和 (Advanced Anti-windup)**:
    -   **Conditional (默认)**: 仅在未饱和或反向修正时积分。
    -   **Back-Calculation**: 动态调整积分项以快速退出饱和（需配置 `kw`）。
    -   **Clamping**: 硬限幅积分值。
2.  **设定值斜坡控制 (Setpoint Ramp)**: 限制设定值的变化率，防止系统受到阶跃信号的剧烈冲击。
3.  **串级控制 (Cascade PID)**: 原生支持串级（外环位置-内环速度）控制结构。
4.  **微分噪声抑制 (Derivative LPF)**: 内置一阶低通滤波器或二阶 Biquad 滤波器。
5.  **2DOF 控制 (Setpoint Weighting)**: 通过 `beta` 和 `gamma` 参数灵活调整设定值对比例和微分项的权重，实现踢除设定值突变（Kick-free）或调节超调量。
6.  **死区控制 (Linear Deadband)**: 防止微小误差引起的执行器抖动。
7.  **前馈控制 (Feed-Forward)**: 提高系统对设定值变化的响应速度。

## 2. 数据结构

### 配置结构体 (`pid_cfg_t`)
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `kp`, `ki`, `kd`, `kf` | `float` | PID 及前馈增益 |
| `out_max`, `out_min` | `float` | 输出限幅 |
| `deadband` | `float` | 线性死区 (0 为禁用)。注意：若启用 2DOF (beta!=1)，死区仅对积分项生效。 |
| `max_rate` | `float` | 输出变化率限制 (Unit/s, 仅增量式) |
| `max_setpoint_ramp` | `float` | **[新增]** 设定值最大变化率 (Unit/s)，0 为禁用 |
| `anti_windup_mode` | `enum` | **[新增]** 抗饱和模式: `CONDITIONAL`, `BACK_CALC`, `CLAMP`, `NONE` |
| `kw` | `float` | **[新增]** 抗饱和追踪增益 (仅用于 `BACK_CALC` 模式) |
| `beta` | `float` | **[新增]** 比例项设定值权重 (0~1)。1=标准, 0=反馈比例(I-PD) |
| `gamma` | `float` | **[新增]** 微分项设定值权重 (0~1)。1=误差微分, 0=测量微分 |
| `d_tau` | `float` | 微分滤波器时间常数 (秒) |
| `d_lpf_alpha` | `float` | **[弃用]** 手动 Alpha (使用 d_tau 替代) |

## 3. 使用方法

### 初始化示例

```c
#include "pid.h"

// 定义配置 (支持高级特性)
static const pid_cfg_t motor_pos_cfg = {
    .kp = 2.0f, .ki = 0.1f, .kd = 0.05f,
    .out_max = 100.0f, .out_min = -100.0f,
    .max_setpoint_ramp = 50.0f,         // 限制设定值每秒变化不超过 50 单位
    .anti_windup_mode = PID_ANTI_WINDUP_BACK_CALC, // 使用反向计算抗饱和
    .kw = 1.0f,                         // 抗饱和追踪增益
    .d_lpf_alpha = 0.1f
};

pid_t motor_pos;

void init(void) {
    pid_init(&motor_pos, &motor_pos_cfg);
}
```

### 串级控制 (Cascade Control)

适用于位置-速度双闭环系统。

```c
// 定义两个 PID 及其配置
pid_t pos_pid, vel_pid;
pid_cascade_t cascade_sys;

void cascade_init_sys(void) {
    // 初始化单个 PID (省略配置定义)
    pid_init(&pos_pid, &pos_cfg);
    pid_init(&vel_pid, &vel_cfg);
    
    // 初始化串级结构联结
    pid_cascade_init(&cascade_sys, &pos_pid, &vel_pid);
}

void loop(void) {
    float target_pos = 100.0f;
    float current_pos = get_pos();
    float current_vel = get_vel();
    float dt = 0.001f;

    // 更新串级 PID
    // 自动计算外环输出作为内环设定值，返回内环最终输出
    float motor_pwm = pid_cascade_update(&cascade_sys, target_pos, current_pos, current_vel, dt);
    
    set_motor(motor_pwm);
}
```

### 增量式 PID
与标准版一致，使用 `pid_update_incremental`。


### 时间戳调用
⚠️ **注意**: 本库**不**自动计算 `dt`。用户必须在调用 `pid_update` 时传入精确的时间间隔（秒）。
推荐使用高精度硬件定时器或系统滴答计数器计算两次调用之间的差值。


## 4. 优缺点分析

### 优点
*   **鲁棒性强**: 结合了抗饱和与滤波，适应性好。
*   **接口清晰**: 配置与状态分离，易于管理多个PID实例。
*   **易于移植**: 标准C编写，无特殊依赖。

### 缺点/注意事项
*   **浮点运算**: 核心算法使用 `float`，在无 FPU 的低端单片机上可能会有额外开销。

## 5. 高级功能指南 (Advanced Features)

### 5.1 手动/自动无扰切换 (Manual/Auto Bumpless Transfer)

当系统需要人工介入控制（手动模式）时，PID 控制器应暂停计算，但内部状态必须跟随手动输出，以确保切回自动模式瞬间输出平滑过渡。

**使用步骤**:
1.  **切入手动模式**: 停止通过 `pid_update` 控制执行器。
2.  **手动期间**: 周期性调用 `pid_track_manual`，传入当前的手动输出值、PV 和 SP。
3.  **切回自动模式**: 恢复调用 `pid_update`。

```c
// 伪代码示例
void control_loop(void) {
    if (manual_mode) {
        float manual_out = read_potentiometer();
        set_actuator(manual_out);
        
        // 关键：让 PID 跟踪手动输出
        pid_track_manual(&pid, manual_out, measurement, setpoint);
    } else {
        float auto_out = pid_update(&pid, setpoint, measurement, dt);
        set_actuator(auto_out);
    }
}
```

### 5.2 强制积分预设 (Forced Integral Setup)

在某些特定工况下（如系统启动、故障恢复），可能需要将积分项预设为特定值（例如已知的负载平衡点）。

```c
// 将积分项强制设为 50.0 (会被限制在 out_min/max 之间)
pid_set_integral(&pid, 50.0f);
```

### 5.3 2DOF 控制 (二自由度 PID)

通过调整 `beta` (比例权重) 和 `gamma` (微分权重)，可以解耦跟踪性能（对设定值响应）和抗扰性能（对负载响应）。

*   **标准 PID**: `beta = 1.0`, `gamma = 1.0` (对误差操作)
*   **微分先行 (Kick-free)**: `gamma = 0.0` (对测量值微分，避免 SP 突变导致 D 项冲击)
*   **I-PD 控制**: `beta = 0.0`, `gamma = 0.0` (仅积分项响应 SP 变化，P/D 仅抑制干扰。响应最平滑，但较慢)
*   **软化响应**: `beta = 0.5` (降低设定值变化引起的比例冲击，减少超调)

```c
pid_cfg_t cfg = {
    // ...
    .beta = 1.0f,  // 默认: 比例项完全响应设定值
    .gamma = 0.0f, // 推荐: 微分项仅响应测量值 (无冲击)
};
```
