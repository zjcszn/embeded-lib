# 软件 I2C 驱动使用指南 (Software I2C Driver Usage Guide)

## 1. 简介 (Introduction)
本驱动提供了一个跨平台、易移植的软件模拟 I2C 主机（Master）实现。它通过操作 GPIO 引脚模拟 I2C 时序，支持标准模式（100kHz）和快速模式（400kHz）。

**核心特性：**
- **高可移植性**：硬件依赖层（GPIO 操作）与核心逻辑分离。
- **防止死锁**：内置检测和恢复机制，解决从机拉低 SDA 的卡死问题。
- **可配置时钟延展**：支持启用或禁用 Clock Stretching，适应不同的应用场景（特别是裸机环境）。
- **精准延时**：使用汇编级 NOP 循环实现微秒级延时，保证时序准确性。

---

## 2. 快速入门 (Getting Started)

### 2.1 硬件适配 (Porting)
用户需要定义 `sw_i2c_ops_t` 结构体，提供操作 GPIO 的回调函数：

```c
typedef struct {
    void (*set_scl)(void *handle, uint8_t state); // 设置 SCL 电平 (0: Low, 1: High-Z/Input)
    void (*set_sda)(void *handle, uint8_t state); // 设置 SDA 电平
    uint8_t (*get_scl)(void *handle);             // 读取 SCL 电平
    uint8_t (*get_sda)(void *handle);             // 读取 SDA 电平
} sw_i2c_ops_t;
```

**注意**：I2C 总线是开漏（Open-Drain）输出。
- `set_sda(1)` 应将引脚设为 **输入模式** 或 **高阻态**（依靠外部上拉电阻拉高）。
- `set_sda(0)` 应将引脚设为 **输出低电平**。

### 2.2 初始化 (Initialization)

使用 `sw_i2c_init` 函数初始化设备实例：

```c
sw_i2c_t i2c_dev;

// 假设已实现 my_ops 和 my_param
sw_i2c_init(&i2c_dev, &my_ops, &my_param, 400, 168000000);
```

- **freq_khz**: 目标 I2C 频率（如 100, 400）。
- **sys_clk_hz**: 系统主频（用于计算延时循环次数）。

---

## 3. 基本操作 (Basic Operations)

### 3.1 初始化 (Initialization)
使用 `sw_i2c_init_default` 初始化 I2C 总线，需通过 `sw_i2c_port_cfg_t` 结构体传递参数。

**注意：** 配置结构体指针将被直接保存到 I2C 设备句柄中，因此**必须保证配置结构体的生命周期覆盖整个 I2C 使用周期**（通常建议定义为全局变量或 `static` 变量，**不要**使用局部自动变量）。

```c
#include "sw_i2c_port.h"

sw_i2c_t i2c_dev;

// 1. 定义端口配置 (必须是 static 或全局变量)
static sw_i2c_port_cfg_t cfg = {
    .scl_port = GPIOB,
    .scl_pin  = LL_GPIO_PIN_6,
    .sda_port = GPIOB,
    .sda_pin  = LL_GPIO_PIN_7
};

// 2. 初始化
// params: dev, config, freq_khz, sys_clk_hz (0=auto)
### 3.1 扫描设备 (Scan)
扫描总线上是否存在设备（调试用）：

```c
sw_i2c_scan(&i2c_dev);
```

### 3.2 读写寄存器 (Read/Write Register)
大多数 I2C 传感器使用寄存器读写模式。

**写入寄存器**：
```c
uint8_t data[] = {0x01, 0x02};
// 向设备地址 (dev_dev.dev_addr) 的 0x10 寄存器写入 2 字节数据
sw_i2c_master_mem_write(&i2c_dev, 0x10, 1, data, 2);
```

**读取寄存器**：
```c
uint8_t buf[4];
// 从 0x10 寄存器读取 4 字节数据
sw_i2c_master_mem_read(&i2c_dev, 0x10, 1, buf, 4);
```

**说明**：
- `mem_addr`: 寄存器地址。
- `addr_len`: 寄存器地址长度（通常为 1 字节或 2 字节）。

---


### 3.3 管理从机地址 (Managing Slave Address)
`sw_i2c_init` 初始化时不绑定特定从机地址。如果需要与多个从机通信，或者在不同时间切换设备：

```c
// 设置新的从机地址 (7位地址)
sw_i2c_set_addr(&i2c_dev, 0x55);

// 获取当前地址
uint16_t current_addr = sw_i2c_get_addr(&i2c_dev);
```

### 3.4 设备就绪检测 (Device Readiness Check)
在操作设备前，可以检查设备是否在线（是否响应 ACK）：

```c
// 检查地址 0x55 的设备是否就绪
if (sw_i2c_is_device_ready(&i2c_dev, 0x55) == SOFT_I2C_OK) {
    // 设备存在，可以通信
} else {
    // 设备未响应
}
```

---

## 4. 高级功能 (Advanced Features)

### 4.1 死锁恢复 (Deadlock Recovery)
**现象**：程序重启后初始化 I2C 失败，SDA 线一直被从机拉低（例如从机在发送数据位 0 时主机突然复位，从机仍等待时钟）。

**解决方法**：
在初始化前或出错时调用解锁函数。

```c
// 1. 检查是否存在死锁 (SCL=1 时 SDA=0)
if (sw_i2c_check_stuck(&i2c_dev) != SOFT_I2C_OK) {
    LOG_W("Bus stuck! Attempting unlock...");
    
    // 2. 尝试解锁 (发送 9 个时钟脉冲)
    if (sw_i2c_unlock(&i2c_dev) == SOFT_I2C_OK) {
        LOG_I("Bus unlocked successfully.");
    } else {
        LOG_E("Failed to unlock bus. Hardware reset required?");
    }
}
```

### 4.2 时钟延展配置 (Clock Stretching)
**背景**：I2C 协议允许从机拉低 SCL 表示“忙”。如果从机一直拉低，会导致主机无限等待（死锁）。

**配置策略**：

1.  **启用 (默认)**：
    ```c
    i2c_dev.enable_clock_stretch = true;
    ```
    - 主机会等待从机释放 SCL（带有超时检测）。
    - 适合标准兼容性。

2.  **禁用 (推荐用于裸机)**：
    ```c
    i2c_dev.enable_clock_stretch = false;
    ```
    - 主机**不等待**从机的逻辑延展。
    - 仅保留极短的物理上升时间检测（约 0.5~2us）。
    - 如果 SCL 被拉低，立即返回错误，防止整个单片机程序卡死。

---

## 5. 常见问题 (FAQ)

**Q: 为什么读不到数据？**
A: 
1. 检查 `dev_addr` 是否正确（通常是 7-bit 地址，不含读写位）。
2. 检查 GPIO 初始化是否正确（由于是开漏，必须启用上拉电阻或配置为 OD 模式）。
3. 降低频率 (`freq_khz`) 试试，可能是线路电容过大。

**Q: 为什么 `sw_i2c_init` 返回 `ERR_PARAM`？**
A: 通常是因为 `sys_clk_hz` 设置为 0 且未开启自动检测，或者计算出的延时循环数为 0（主频太低跑不了目标 I2C 频率）。

**Q: 禁用 Clock Stretch 后会不会不稳定？**
A: 对于绝大多数现代 I2C 传感器，它们处理速度很快，不需要延展时钟。禁用它是安全的，并且能显著提高系统鲁棒性（防止死锁）。本驱动在禁用时仍保留了物理上升时间检测，以防止误报。
