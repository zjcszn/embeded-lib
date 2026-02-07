# 软件 I2C 从机模块操作指引 (Soft I2C Slave User Guide)

本文档旨在指导用户将 `soft_i2c_slave` 模块集成到您的嵌入式工程中。

## 1. 模块简介
本模块提供了一个硬件无关的软件 I2C 从机协议栈。通过硬件抽象层 (HAL) 与底层驱动解耦，使其能够轻松移植到任何支持 GPIO 中断的 MCU 平台上。

### 关键特性
*   **平台无关性**: 仅依赖标准 C 库。
*   **事件驱动**: 通过回调函数处理 `START`, `STOP`, `RECV`, `TRNS` 等事件。
*   **MISRA-C 兼容**: 严格的代码规范，适合高可靠性应用。

## 2. 集成步骤

### 第一步：实现硬件接口 (The Hardware Ops)
您需要实现 `i2c_slave_ops_t` 结构体定义的 5 个函数：

```c
/* 示例: 实现 STM32 LL 库的 GPIO 操作 */
static uint8_t port_read_scl(void) {
    return LL_GPIO_IsInputPinSet(GPIOD, LL_GPIO_PIN_2);
}
/* ... 其他函数类似 ... */

const i2c_slave_ops_t my_ops = {
    .read_scl = port_read_scl,
    .read_sda = port_read_sda,
    .set_sda_out = port_set_sda_out,
    .set_sda_in = port_set_sda_in,
    .write_sda = port_write_sda,
    .delay = NULL /* 本实现不需要延时 */
};
```

### 第二步：实现事件回调 (The Event Handler)
编写一个函数处理 I2C 事件逻辑：

```c
void my_i2c_callback(i2c_slave_ev_t ev, uint8_t *data) {
    switch (ev) {
        case I2C_SLAVE_EV_START:
            /* 开始信号，data 指向地址字节 */
            break;
        case I2C_SLAVE_EV_RECV:
            /* 接收到数据，data 指向接收到的字节 */
            break;
        case I2C_SLAVE_EV_TRNS:
            /* 主机请求数据，将要发送的数据写入 *data */
            *data = 0x55; 
            break;
        case I2C_SLAVE_EV_STOP:
            /* 停止信号 */
            break;
        default:
            break;
    }
}
```

### 第三步：初始化与调用
在 `main.c` 中初始化，并在 GPIO 中断中调用处理函数：

```c
i2c_slave_t i2c_handle;

int main(void) {
    /* 1. 配置参数 */
    i2c_slave_cfg_t cfg = {
        .addr = 0xA0,         /* 7位地址左移一位 */
        .ops = &my_ops,
        .callback = my_i2c_callback
    };

    /* 2. 初始化 */
    i2c_slave_init(&i2c_handle, &cfg);
    
    /* ... 开启中断 ... */
    while (1) {}
}

/* GPIO 中断服务函数 */
void EXTI2_IRQHandler(void) {
    /* SCL 中断处理 */
    if (check_scl_interrupt()) {
        i2c_slave_scl_handler(&i2c_handle);
    }
    /* SDA 中断处理 */
    if (check_sda_interrupt()) {
        i2c_slave_sda_handler(&i2c_handle);
    }
}
```

## 3. 注意事项
1.  **中断优先级**: 为了确保时序准确，请将 I2C 使用的 GPIO 中断优先级设为最高。
2.  **时钟延展**: 本模块不支持时钟延展，请确保主机 I2C 时钟频率不要过高（建议 < 100kHz）。
3.  **驱动能力**: 配置 SDA 引脚为开漏输出 (Open-Drain)，并确保外部有合适的上拉电阻。

---
**技术支持**: 如有问题，请参考源代码中的 `soft_i2c_slave_port.c` 参考实现。
