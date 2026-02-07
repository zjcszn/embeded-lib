# OneWire HAL 模块使用指引

本模块提供了一个通用的、硬件无关的 1-Wire 协议中间件（HAL）。它设计用于在不同的硬件平台上（ARM, RISC-V, FPGA软核等）轻松移植和使用。

## 1. 目录结构

```
.
├── onewire.c           # 协议层核心实现 (硬件无关)
├── onewire.h           # 协议层接口声明
├── onewire_config.h    # 配置文件 (临界区、延时)
├── bsp_onewire.c       # 板级支持包 (GPIO驱动实现)
└── bsp_onewire.h       # 板级支持包接口
```

## 2. 移植指南

要将此模块移植到新的硬件平台，通常只需修改以下两个部分：

### 2.1 修改 `onewire_config.h`

此文件负责定义临界区保护（关中断）和微秒延时函数。模块已经内置了对 Windows (模拟), Linux (模拟), RISC-V 和 ARM Cortex-M 的自动检测。

如果不符合您的环境，请手动修改：

```c
/* 示例：自定义平台 */
#define ONEWIRE_ENTER_CRITICAL()  my_disable_irq()
#define ONEWIRE_EXIT_CRITICAL()   my_enable_irq()
#define ONEWIRE_DELAY_US(us)      my_delay_us(us)
```

**注意**: 本模块现在采用了 **Bit-Level** 的临界区保护。这意味着中断仅在单个 Bit 的读写过程中被屏蔽（约 60-70us），大大降低了对系统实时性的影响，而不再是在整个 Byte 传输期间屏蔽中断。

### 2.2 实现 GPIO 操作 (`bsp_onewire.c`)

在 `bsp_onewire.c` 中，您需要实现 `onewire_ops_t` 结构体定义的两个回调函数：

1.  `set_pin(void *arg, uint8_t level)`: 设置 GPIO 电平 (0 为低电平，1 为高电平/释放总线)。
    *   **注意**: 1-Wire 是开漏驱动。写 1 时应设置为输入模式或高阻态（如果是开漏输出则写 1）。
2.  `get_pin(void *arg)`: 读取 GPIO 输入电平。

## 3. 使用示例

### 3.1 初始化

```c
#include "bsp_onewire.h"

int main(void) {
    /* 初始化总线 (具体的 GPIO 端口定义取决于您的 BSP 实现) */
    onewire_bus_t *bus = bsp_onewire_init(GPIOA, GPIO_PIN_5);
    
    /* 复位总线并检测设备 */
    if (onewire_reset(bus)) {
        printf("Device Detected!\n");
    } else {
        printf("No Device.\n");
    }
}
```

### 3.2 搜索 ROM (遍历所有设备)

本模块内置了标准的 Maxim Application Note 187 搜索算法。

```c
onewire_search_state_t search_state;
onewire_search_start(&search_state);

while (onewire_search_next(bus, &search_state)) {
    printf("Found ROM: ");
    for(int i=0; i<8; i++) {
        printf("%02X ", search_state.rom_id[i]);
    }
    printf("\n");
}
```

### 3.3 读写操作

#### 单设备操作 (Skip ROM)
如果总线上只有一个设备，可以使用 `onewire_skip_rom` 跳过 ROM 匹配过程，直接发送功能命令。

```c
/* 跳过 ROM (广播)，仅当总线上只有一个设备时使用 */
onewire_skip_rom(bus); 

/* 发送命令，例如读取温度 (DS18B20 Converse T) */
onewire_write_byte(bus, 0x44); 
```

#### 多设备操作 (Match ROM)
如果总线上有多个设备，必须先匹配目标设备的 ROM ID。

```c
uint8_t target_rom[8] = {0x28, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x12};

/* 匹配特定设备 */
onewire_match_rom(bus, target_rom);

/* 发送命令给该设备 */
onewire_write_byte(bus, 0xBE); // Read Scratchpad
```

## 4. 常见问题 (FAQ)

*   **Q: 为什么读写不稳定？**
    *   A: 1-Wire 对时序极其敏感。请检查 `ONEWIRE_DELAY_US` 是否精确，并确保 `ONEWIRE_ENTER_CRITICAL` 正确关闭了中断，防止中断打断时序。
*   **Q: 支持多路 1-Wire 吗？**
    *   A: 支持。`onewire_bus_t` 可以在初始化时绑定不同的端口数据，您可以创建多个 `bus` 实例。
*   **Q: 搜索算法是否标准？**
    *   A: 是的，`onewire_search_next` 函数严格遵循 Maxim Application Note 187 的标准搜索算法实现，支持 CRC 校验和冲突检测。

## 5. 许可证

(在此处添加您的许可证信息，例如 MIT, Apache-2.0, 或私有协议)
