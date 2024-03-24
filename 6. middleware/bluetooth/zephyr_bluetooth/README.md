# Zephyr_bluetooth

在开源的 bt 协议栈中，zephyr 项目带的 bluetooth 组件是目前商业上使用比较广泛的，但是厂家支持的版本相对来说都比较旧（一般是 2.1 版本以内）。并且厂家自身改动较多，无法同步主线，导致后续无法使用最新的特性。因此，本项目的意义就在于方便同步主线和适配其他平台。项目基于 [Zephyr bluetooth 2.7.5](https://github.com/zephyrproject-rtos/zephyr/releases/tag/v2.7.5) 版本修改和剥离，并且尽量减少源码的改动，方便后续同步主线。

选择 2.7.5 版本主要是因为这个版本目前是 lts 版本，3.0版本以后改动较多，bug也比较多，不能作为长期使用。

## 修改策略

- 源码中所有非 bluetooth 头文件使用 `<zephyr.h>` 替换，包含以下头文件的源码都会替换。

```
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/errno.h>

#include <toolchain.h>
#include <sys/__assert.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <sys/slist.h>
#include <sys/dlist.h>
#include <sys/check.h>
#include <sys/util_macro.h>
#include <sys/atomic.h>
#include <linker/sections.h>

```

- `struct k_sem` 增加 name 成员用于记录, `struct k_queue` 增加 sem 成员用于 IPC通信

- 由于部分 os 只能创建 sem 后返回指针句柄，不能在一开始初始化和结构体句柄，因此，所有关联的 **初始化 api 宏** 都需要在初始化时调用 `k_fifo_init` 或者 `k_lifo_init` 或者  `k_sem_init`，以下宏定义受到影响。

```
K_SEM_DEFINE
K_FIFO_DEFINE
NET_BUF_POOL_DEFINE
NET_BUF_POOL_FIXED_DEFINE
```

- 线程 `k_thread_create` 增加 `name` 参数(替代 `k_thread_name_set` )，并且线程 api 使用一个形参 `void *p1`。

## Porting 层

- os 相关（包含 thread，sem）
- work queue 相关
- poll 相关
- mem slab（内存池）

## HCI 层

- uart h4
- usb

## Kconfig 配置

保留默认的 kconfig 配置，最终生成的配置文件为 `bt_config.h`。命令如下：

```
menuconfig

genconfig --header-path bt_config.h

```

## 源文件和头文件添加

参考 CMakeLists.txt