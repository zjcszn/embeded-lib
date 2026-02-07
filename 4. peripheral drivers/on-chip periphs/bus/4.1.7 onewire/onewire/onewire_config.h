#ifndef __ONEWIRE_CONFIG_H__
#define __ONEWIRE_CONFIG_H__

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

/* Windows platform */
#define ONEWIRE_ENTER_CRITICAL()
#define ONEWIRE_EXIT_CRITICAL()

/* Mock delay for Windows (not real-time accurate) */
static __inline void onewire_delay_us(uint32_t us) {
    /* Large enough simple loop or Sleep(0) to yield */
    /* Note: Windows Sleep is ms precision, not suitable for precise 1-wire timing */
    /* For simulation logic only */
    (void) us;
}
#define ONEWIRE_DELAY_US(us) onewire_delay_us(us)

#elif defined(__linux__)
#include <unistd.h>

/* Linux platform */
#define ONEWIRE_ENTER_CRITICAL()
#define ONEWIRE_EXIT_CRITICAL()

/* use usleep for linux (min 1us resolution, but scheduling jitter affects usage) */
#define ONEWIRE_DELAY_US(us) usleep(us)

#elif defined(__riscv)
#include "main.h"

/* RISC-V platform (assuming standard or HAL provided disable/enable irq) */
/* If __disable_irq is not available in your specific RISC-V env, replace with inline asm */
/* e.g., __asm volatile("csrci mstatus, 8") */
#define ONEWIRE_ENTER_CRITICAL() __disable_irq()
#define ONEWIRE_EXIT_CRITICAL()  __enable_irq()

#define ONEWIRE_DELAY_US(us)     delay_us(us)

#else
/* Default (typically ARM Cortex-M) */
#include "main.h"

#define ONEWIRE_ENTER_CRITICAL() __disable_irq()
#define ONEWIRE_EXIT_CRITICAL()  __enable_irq()

#define ONEWIRE_DELAY_US(us)     delay_us(us)

#endif

#endif /* __ONEWIRE_CONFIG_H__ */
