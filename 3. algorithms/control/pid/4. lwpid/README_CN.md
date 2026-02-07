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
4.  **微分噪声抑制 (Derivative LPF)**: 内置一阶低通滤波器，平滑微分项。
5.  **测量值微分 (Kick-free)**: 可选对测量值而非误差进行微分，避免设定值突变引起的输出尖峰。
6.  **死区控制 (Linear Deadband)**: 防止微小误差引起的执行器抖动。
7.  **前馈控制 (Feed-Forward)**: 提高系统对设定值变化的响应速度。

## 2. 数据结构

### 配置结构体 (`pid_cfg_t`)
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `kp`, `ki`, `kd`, `kf` | `float` | PID 及前馈增益 |
| `out_max`, `out_min` | `float` | 输出限幅 |
| `deadband` | `float` | 线性死区 (0 为禁用) |
| `max_rate` | `float` | 输出变化率限制 (Unit/s, 仅增量式) |
| `max_setpoint_ramp` | `float` | **[新增]** 设定值最大变化率 (Unit/s)，0 为禁用 |
| `anti_windup_mode` | `enum` | **[新增]** 抗饱和模式: `CONDITIONAL`, `BACK_CALC`, `CLAMP`, `NONE` |
| `kw` | `float` | **[新增]** 抗饱和追踪增益 (仅用于 `BACK_CALC` 模式) |
| `d_lpf_alpha` | `float` | 微分滤波系数 (0.0~1.0) |
| `derivative_on_measurement` | `bool` | 是否使用测量值微分 |

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
使用 `pid_update_tick`，自动计算 `dt`。


### 核心文件
- **`pid.h`**: 定义了PID的数据结构（配置结构体 `pid_cfg_t` 和 运行时结构体 `pid_t`）以及API接口。
- **`pid.c`**: 实现了PID的核心算法，包括位置式和增量式更新函数。

### 主要特性
1.  **抗积分饱和 (Dynamic Anti-windup)**: 使用条件积分法（Conditional Integration），仅在输出未饱和或误差方向有助于退出饱和时进行积分，有效防止积分项失控。
2.  **微分噪声抑制 (Derivative LPF)**: 内置一阶低通滤波器，用于平滑微分项，防止由于测量噪声引起的输出抖动。
3.  **模式选择 (Mode Selection)**: 支持“误差微分”（Standard）和“测量值微分”（Derivative on Measurement, Kick-free）两种模式。
4.  **死区控制 (Linear Deadband)**: 支持设定线性死区，忽略微小误差，防止执行器频繁抖动。
5.  **前馈控制 (Feed-Forward)**: 支持前馈增益 `kf`，提高系统响应速度。
6.  **双重更新接口**: 同时提供标准的位置式 PID (`pid_update`) 和增量式 PID (`pid_update_incremental`) 接口。

## 2. 数据结构

### 配置结构体 (`pid_cfg_t`)
该结构体用于存储PID的参数，通常定义为 `const` 或在初始化后保持不变。

| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `kp` | `pid_real_t` | 比例增益 (Proportional Gain) |
| `ki` | `pid_real_t` | 积分增益 (Integral Gain) |
| `kd` | `pid_real_t` | 微分增益 (Derivative Gain) |
| `kf` | `pid_real_t` | 前馈增益 (Feed-Forward Gain) |
| `out_max` | `pid_real_t` | 输出上限 |
| `out_min` | `pid_real_t` | 输出下限 |
| `deadband` | `pid_real_t` | 死区阈值 (0 表示禁用) |
| `max_rate` | `pid_real_t` | 输出变化率限制 (单位/秒)，仅用于增量式PID，0 表示禁用 |
| `d_lpf_alpha` | `pid_real_t` | 微分低通滤波系数 (0.0 - 1.0)，建议值 0.1 - 0.3 |
| `derivative_on_measurement` | `bool` | `true`: 对测量值微分 (防止设定值突变冲击); `false`: 对误差微分 |

### 运行时结构体 (`pid_t`)
该结构体维护PID控制器的内部状态，如积分累加值、历史误差等。用户不应直接修改其内部成员。

## 3. 使用方法

### 初始化 (Initialization)

```c
#include "pid.h"

// 1. 定义配置参数 (建议定义为全局或持久变量)
static const pid_cfg_t motor_pid_cfg = {
    .kp = 1.5f,
    .ki = 0.05f,
    .kd = 0.01f,
    .kf = 0.0f,
    .out_max = 100.0f,
    .out_min = -100.0f,
    .deadband = 0.1f,
    .d_lpf_alpha = 0.2f,        // 启用微分滤波
    .derivative_on_measurement = true // 防止设定值突变冲击
};

// 2. 定义运行时句柄
pid_t motor_pid;

// 3. 在系统启动时初始化
void system_init(void) {
    if (!pid_init(&motor_pid, &motor_pid_cfg)) {
        // 初始化失败处理 (如参数错误)
    }
}
```

### 控制循环 (Control Loop)

在定时器中断或主循环中定期调用更新函数。

```c
void control_loop(void) {
    float target_speed = 50.0f;          // 目标值 (Setpoint)
    float current_speed = get_speed();   // 测量值 (Measurement)
    float dt = 0.001f;                   // 采样周期 (1ms)

    // 计算PID输出
    float output = pid_update(&motor_pid, target_speed, current_speed, dt);

    // 应用输出到执行器
    set_motor_pwm(output);
}
```

### 增量式 PID (Incremental PID)

适用于步进电机、积分型执行机构，或需要平滑切换的场景。

**与位置式 PID 的区别**：
*   **位置式 (`pid_update`)**: 输出的是**目标绝对值** (例如：PWM 占空比 50%)。
    *   使用 `out_min` / `out_max` 限制输出范围。
*   **增量式 (`pid_update_incremental`)**: 输出的是**控制量的变化值 ($\Delta u$)** (例如：PWM 增加 1%)。
    *   使用 `max_rate` 限制输出的变化速率 (Slew Rate)，单位是 **Unit/s**。
    *   **注意**: 增量式模式下忽略 `out_min` / `out_max`，用户需在外部叠加 $\Delta u$ 后自行限制最终输出范围。

```c
// 配置示例
static const pid_cfg_t stepper_cfg = {
    // ... Kp, Ki, Kd ...
    .max_rate = 100.0f, // 限制每秒最大变化量为 100
};

void stepper_control_loop(void) {
    float dt = 0.01f; // 10ms
    // 计算输出增量 (已包含 max_rate 限制)
    float delta = pid_update_incremental(&stepper_pid, target, measured, dt);
    
    // 用户需自行维护最终输出，并进行绝对值限幅
    static float current_output = 0.0f;
    current_output += delta;
    
    // 限制最终输出范围
    if (current_output > 1000.0f) current_output = 1000.0f;
    if (current_output < 0.0f) current_output = 0.0f;
    
    drive_stepper_speed(current_output);
}
```

### 时间戳辅助函数

如果无法直接获得 `dt`，可以使用 `pid_update_tick`，传入当前系统 Tick。

```c
void loop(void) {
    uint32_t now = HAL_GetTick();
    // 假设 1 tick = 1 ms = 0.001 s
    float output = pid_update_tick(&motor_pid, target, measured, now, 0.001f);
}
```

## 4. 优缺点分析

### 优点
*   **鲁棒性强**: 结合了抗饱和与滤波，适应性好。
*   **接口清晰**: 配置与状态分离，易于管理多个PID实例。
*   **易于移植**: 标准C编写，无特殊依赖。

### 缺点/注意事项
*   **浮点运算**: 核心算法使用 `float`，在无 FPU 的低端单片机上可能会有额外开销。
