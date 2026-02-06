
/* Includes ------------------------------------------------------------------*/
#include "sw_i2c.h"

/* Private macro definition --------------------------------------------------*/
#define LOG_TAG "I2C_SW"

// Auto-detect Cortex-M if not defined
#ifndef SW_I2C_IS_CORTEX_M
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_6M__) || \
    defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__) || defined(__CORTEX_M)
#define SW_I2C_IS_CORTEX_M 1
#else
#define SW_I2C_IS_CORTEX_M 0
#endif
#endif

#if SOFT_I2C_USE_LOG
#include "elog.h"
#include "stdio.h"

#define SW_I2C_LOG(...) log_d(__VA_ARGS__)
#else
#define SW_I2C_LOG(...) ((void) 0U)
#endif

// Default Timeout in ms
#define SW_I2C_TIMEOUT_MS 1

// Calibration Loop Counts
#ifndef SW_I2C_CALIB_POINT1_LOOPS
#define SW_I2C_CALIB_POINT1_LOOPS 1000
#endif

#ifndef SW_I2C_CALIB_POINT2_LOOPS
#define SW_I2C_CALIB_POINT2_LOOPS 11000
#endif

// Estimated Cycles per Polling Loop (Read SCL + Branch + Decrement)
// Conservative estimate for Cortex-M
#define SW_I2C_POLL_CYCLES 40

#ifndef SW_I2C_CYCLES_PER_LOOP
// Use compiler macros for reliable architecture detection
#if defined(__ARM_ARCH_7EM__) && defined(__ARM_FEATURE_DSP)
// Cortex-M7 (ARMv7E-M with DSP, typical for M7)
// Dual-issue pipeline executes simple loops much faster (~2 cycles/loop)
#define SW_I2C_CYCLES_PER_LOOP 2
#else
// Cortex-M0/M3/M4/M33 (Standard 3-stage or equivalent pipeline) -> ~5 cycles/loop
#define SW_I2C_CYCLES_PER_LOOP 5
#endif
#endif

/* Private type definition ---------------------------------------------------*/

// [Decoupled] SysTick Register Definitions for Cortex-M
// Address is fixed at 0xE000E010 for all Cortex-M cores
#if SW_I2C_IS_CORTEX_M

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SW_I2C_SysTick_Type;

#define SW_I2C_SysTick_BASE               0xE000E010UL
#define SW_I2C_SysTick                    ((SW_I2C_SysTick_Type *) SW_I2C_SysTick_BASE)

/* SysTick Control Register Definitions */
#define SW_I2C_SysTick_CTRL_ENABLE_Pos    0U
#define SW_I2C_SysTick_CTRL_ENABLE_Msk    (1UL << SW_I2C_SysTick_CTRL_ENABLE_Pos)
#define SW_I2C_SysTick_CTRL_CLKSOURCE_Pos 2U
#define SW_I2C_SysTick_CTRL_CLKSOURCE_Msk (1UL << SW_I2C_SysTick_CTRL_CLKSOURCE_Pos)

/* Compiler Intrinsics for Interrupt Control */
#if defined(__GNUC__) || defined(__clang__)

static inline void __attribute__((always_inline)) __disable_irq(void) {
    __asm__ volatile("cpsid i" : : : "memory");
}

static inline uint32_t __attribute__((always_inline)) __get_PRIMASK(void) {
    uint32_t result;
    __asm__ volatile("mrs %0, primask" : "=r"(result));
    return result;
}

static inline void __attribute__((always_inline)) __set_PRIMASK(uint32_t priMask) {
    __asm__ volatile("msr primask, %0" : : "r"(priMask) : "memory");
}

#elif defined(__CC_ARM)
/* Keil ARMCC (v5) typically provides these as intrinsics.
 * If not, they are register-based. Assuming Standard Library is available.
 * If zero-dependency is strictly required for ARMCC, we would add __asm here.
 */
#elif defined(__ICCARM__)
#include <intrinsics.h>
#endif

#endif

/* Private inline function definition ----------------------------------------*/

/**
 * @brief  Delay loops using assembly to prevent optimization
 *         Approx cycles = loops * SW_I2C_CYCLES_PER_LOOP
 */
void __attribute__((noinline)) sw_i2c_delay_ticks(uint32_t loops) {
    if (loops == 0) {
        return;
    }
#if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
    __asm__ volatile(
        "1: \n"
        " nop \n"
        " subs %0, %0, #1 \n"
        " bne 1b \n"
        : "+r"(loops));
#elif defined(__ICCARM__)
    __asm volatile(
        "1: \n"
        " nop \n"
        " subs %0, %0, #1 \n"
        " bne 1b \n"
        : "+r"(loops));
#else
    // Fallback for other compilers
    volatile uint32_t i = loops;
    while (i--)
        ;
#endif
}

static inline void i2c_delay_hold(sw_i2c_t *dev) {
    sw_i2c_delay_ticks(dev->ticks_hold);
}

static inline void i2c_delay_setup(sw_i2c_t *dev) {
    sw_i2c_delay_ticks(dev->ticks_setup);
}

static inline void i2c_delay_high(sw_i2c_t *dev) {
    sw_i2c_delay_ticks(dev->ticks_high);
}

static inline void i2c_scl_write(sw_i2c_t *dev, uint8_t state) {
    dev->ops.set_scl(dev->user_data, state);
}

static inline void i2c_sda_write(sw_i2c_t *dev, uint8_t state) {
    dev->ops.set_sda(dev->user_data, state);
}

static inline uint8_t i2c_sda_read(sw_i2c_t *dev) {
    return dev->ops.get_sda(dev->user_data);
}

static inline uint8_t i2c_scl_read(sw_i2c_t *dev) {
    return dev->ops.get_scl(dev->user_data);
}

/**
 * @brief  Release SCL and wait for it to go High (Clock Stretching)
 */
static int i2c_scl_high_check(sw_i2c_t *dev) {
    i2c_scl_write(dev, 1);

    // Check if SCL is actually High
    if (dev->ops.get_scl) {
        // If Clock Stretching is disabled, we still need to allow some time for
        // the SCL line to physically rise due to bus capacitance.
        // We use 'ticks_setup' (approx 1/4 period) as a reasonable "Rise Time" allowance.
        // This prevents false positives due to IO hysteresis or slow rise times.
        if (!dev->enable_clock_stretch) {
            uint32_t timeout = dev->ticks_setup;
            while (i2c_scl_read(dev) == 0) {
                if (--timeout == 0) {
                    SW_I2C_LOG("Clock stretching detected but disabled!\r\n");
                    return 1;  // Error
                }
            }
            return 0;  // OK
        }

        // Use calculated timeout ticks
        uint32_t timeout = dev->ticks_timeout;
        while (i2c_scl_read(dev) == 0) {
            if (--timeout == 0) {
                return 1;
            }
        }
    }
    return 0;
}

/* Private function ----------------------------------------------------------*/

static void sw_i2c_calibration(sw_i2c_t *i2c_dev) {
#if SW_I2C_IS_CORTEX_M
    // 1. Check if SysTick is effectively enabled and running
    uint32_t ctrl_before = SW_I2C_SysTick->CTRL;
    uint32_t load_before = SW_I2C_SysTick->LOAD;

    // 2. Disable interrupts to ensure atomic measurement
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    // 3. Configure SysTick if not already running correctly
    // We need 24-bit max range to measure long delays
    SW_I2C_SysTick->LOAD = 0x00FFFFFF;
    SW_I2C_SysTick->VAL = 0UL;
    SW_I2C_SysTick->CTRL = SW_I2C_SysTick_CTRL_CLKSOURCE_Msk | SW_I2C_SysTick_CTRL_ENABLE_Msk;

    // 4. Measure Point 1: N1
    uint32_t n1 = SW_I2C_CALIB_POINT1_LOOPS;
    uint32_t start_tick = SW_I2C_SysTick->VAL;
    sw_i2c_delay_ticks(n1);
    uint32_t end_tick = SW_I2C_SysTick->VAL;

    // Correct down-counter delta calculation
    uint32_t t1 = (start_tick >= end_tick) ? (start_tick - end_tick)
                                           : (start_tick + (0x00FFFFFF - end_tick) + 1);

    // 5. Measure Point 2: N2
    uint32_t n2 = SW_I2C_CALIB_POINT2_LOOPS;
    start_tick = SW_I2C_SysTick->VAL;
    sw_i2c_delay_ticks(n2);
    end_tick = SW_I2C_SysTick->VAL;
    uint32_t t2 = (start_tick >= end_tick) ? (start_tick - end_tick)
                                           : (start_tick + (0x00FFFFFF - end_tick) + 1);

    // 6. Restore SysTick/Interrupts
    SW_I2C_SysTick->LOAD = load_before;
    SW_I2C_SysTick->VAL = 0UL;  // Clear value to force reload
    SW_I2C_SysTick->CTRL = ctrl_before;
    __set_PRIMASK(primask);

    // 7. Calculate: CPL = (T2 - T1) / (N2 - N1)
    // Note: On high-performance cores (e.g., H7) with dual-issue/superscalar usage,
    // the loop may execute in ~1-2 cycles. Result is stored as float for precision.
    uint32_t delta_n = n2 - n1;
    float cpl = (float) (t2 - t1) / delta_n;

    // Sanity check
    if (cpl < 0.1f) {
        cpl = 1.0f;
    }

    // 8. Calculate: Overhead = T1 - (N1 * CPL)
    // Overhead = b = y - kx
    float pure_cost = n1 * cpl;
    uint32_t overhead = 0;
    if (t1 > pure_cost) {
        overhead = (uint32_t) (t1 - pure_cost);
    }
    // Removed 8-bit cap for overhead

    i2c_dev->cycles_per_loop = cpl;
    i2c_dev->ticks_overhead = overhead;
#else
    // Non-Cortex-M: Use defaults
    if (i2c_dev->cycles_per_loop == 0.0f) {
        i2c_dev->cycles_per_loop = (float) SW_I2C_CYCLES_PER_LOOP;
    }
    i2c_dev->ticks_overhead = 0;
#endif
}

static void i2c_start(sw_i2c_t *dev) {
    i2c_sda_write(dev, 1);
    i2c_scl_write(dev, 1);
    i2c_delay_high(dev);
    i2c_sda_write(dev, 0);
    i2c_delay_high(dev);
    i2c_scl_write(dev, 0);
}

static void i2c_stop(sw_i2c_t *dev) {
    i2c_scl_write(dev, 0);
    i2c_delay_hold(dev);
    i2c_sda_write(dev, 0);
    i2c_delay_setup(dev);
    i2c_scl_write(dev, 1);
    i2c_delay_high(dev);
    i2c_sda_write(dev, 1);
    i2c_delay_high(dev);
}

static void i2c_restart(sw_i2c_t *dev) {
    i2c_scl_write(dev, 0);
    i2c_delay_hold(dev);
    i2c_sda_write(dev, 1);
    i2c_delay_setup(dev);
    i2c_scl_write(dev, 1);
    i2c_delay_high(dev);
    i2c_sda_write(dev, 0);
    i2c_delay_high(dev);
    i2c_scl_write(dev, 0);
}

static int i2c_wait_ack(sw_i2c_t *dev) {
    i2c_scl_write(dev, 0);
    i2c_delay_hold(dev);
    i2c_sda_write(dev, 1);
    i2c_delay_setup(dev);

    if (i2c_scl_high_check(dev)) {
        return SOFT_I2C_ERR_TIMEOUT;
    }
    i2c_delay_high(dev);

    uint8_t ack = i2c_sda_read(dev);
    i2c_scl_write(dev, 0);

    return ack == 0 ? SOFT_I2C_OK : SOFT_I2C_ERR_NACK;
}

static void i2c_send_ack(sw_i2c_t *dev) {
    i2c_scl_write(dev, 0);
    i2c_delay_hold(dev);
    i2c_sda_write(dev, 0);
    i2c_delay_setup(dev);
    if (i2c_scl_high_check(dev)) {
    }
    i2c_delay_high(dev);
    i2c_scl_write(dev, 0);
    i2c_delay_hold(dev);
    i2c_sda_write(dev, 1);
}

static void i2c_send_nack(sw_i2c_t *dev) {
    i2c_scl_write(dev, 0);
    i2c_delay_hold(dev);
    i2c_sda_write(dev, 1);
    i2c_delay_setup(dev);
    if (i2c_scl_high_check(dev)) {
    }
    i2c_delay_high(dev);
    i2c_scl_write(dev, 0);
}

static int i2c_send_byte(sw_i2c_t *dev, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        i2c_scl_write(dev, 0);
        i2c_delay_hold(dev);
        i2c_sda_write(dev, (byte & 0x80) ? 1 : 0);
        i2c_delay_setup(dev);
        if (i2c_scl_high_check(dev)) {
            return SOFT_I2C_ERR_TIMEOUT;
        }
        i2c_delay_high(dev);
        byte <<= 1;
    }
    i2c_scl_write(dev, 0);
    return i2c_wait_ack(dev);
}

static uint8_t i2c_recv_byte(sw_i2c_t *dev) {
    uint8_t byte = 0;
    i2c_sda_write(dev, 1);
    for (int i = 0; i < 8; i++) {
        i2c_scl_write(dev, 0);
        i2c_delay_hold(dev);
        i2c_delay_setup(dev);
        if (i2c_scl_high_check(dev)) {
            return 0;
        }
        i2c_delay_high(dev);
        byte = (byte << 1) | i2c_sda_read(dev);
    }
    i2c_scl_write(dev, 0);
    return byte;
}

/* Public functions ----------------------------------------------------------*/

sw_i2c_err_t sw_i2c_init(sw_i2c_t *i2c_dev, const sw_i2c_ops_t *ops, void *user_data,
                         uint32_t freq_khz, uint32_t sys_clk_hz) {
    if (!i2c_dev || !ops || !ops->set_scl || !ops->set_sda || !ops->get_sda) {
        return SOFT_I2C_ERR_BUS;
    }
    if (sys_clk_hz == 0) {
        return SOFT_I2C_ERR_PARAM;
    }

    // Check if sys_clk is sufficient for target freq (approx check)
    if (sys_clk_hz / (freq_khz * 1000) < 20) {
        // SW_I2C_LOG("SysClk %u too low for %ukHz\r\n", sys_clk_hz, freq_khz);
        return SOFT_I2C_ERR_PARAM;
    }

    i2c_dev->ops = *ops;
    i2c_dev->user_data = user_data;
    i2c_dev->sys_clk_hz = sys_clk_hz;

    // Calculate Timeout Loops
    // Total Cycles = (sys_clk_hz / 1000) * TIMEOUT_MS
    // Loops = Total Cycles / POLL_CYCLES
    uint32_t timeout_cycles = (sys_clk_hz / 1000) * SW_I2C_TIMEOUT_MS;
    i2c_dev->ticks_timeout = timeout_cycles / SW_I2C_POLL_CYCLES;
    if (i2c_dev->ticks_timeout == 0) {
        i2c_dev->ticks_timeout = 1000;  // Safety floor
    }

    // Default to Disabled as per user request (safer for bare-metal)
    // Default to Disabled as per user request (safer for bare-metal)
    i2c_dev->enable_clock_stretch = false;

    // Auto-calibration if cycles_per_loop is 0 or if explicitly requested
    // We choose to ALWAYS run calibration if it's not pre-set, to ensure overhead is calculated
    sw_i2c_calibration(i2c_dev);

    // Calculate timing parameters
    if (sw_i2c_set_speed(i2c_dev, freq_khz) != SOFT_I2C_OK) {
        return SOFT_I2C_ERR_PARAM;
    }

    i2c_sda_write(i2c_dev, 1);
    i2c_scl_write(i2c_dev, 1);

    SW_I2C_LOG("Soft I2C Initialized @ %u kHz (SysClk: %u MHz)\r\n", freq_khz,
               sys_clk_hz / 1000000);
    return SOFT_I2C_OK;
}

sw_i2c_err_t sw_i2c_set_speed(sw_i2c_t *i2c_dev, uint32_t freq_khz) {
    if (freq_khz == 0) {
        freq_khz = 100;
    }

    // Convert ns to cycles: cycles = ns * (sys_clk_hz / 1000000000)
    // Avoid overflow: cycles = sys_clk_hz / freq_hz
    uint32_t total_cycles = i2c_dev->sys_clk_hz / (freq_khz * 1000);
    uint32_t overhead = i2c_dev->ticks_overhead;

    // Minimum cycles check: must be at least overhead + some loops
    if (total_cycles <= overhead) {
        // Frequency too high for the overhead
        return SOFT_I2C_ERR_PARAM;
    }

    if ((total_cycles - overhead) < (uint32_t) (i2c_dev->cycles_per_loop * 4)) {
        return SOFT_I2C_ERR_PARAM;
    }

    /* Target Timing Allocation:
     * High Period: 50%
     * Low Period: 50%
     *   -> Hold: 25% (or minimal)
     *   -> Setup: 25% (or maximal)
     */

    // Distribute Total Cycles
    uint32_t cycles_high = total_cycles / 2;
    uint32_t cycles_low = total_cycles - cycles_high;
    uint32_t cycles_hold, cycles_setup;

    float cpl = i2c_dev->cycles_per_loop;
    if (cpl < 0.1f) {
        cpl = (float) SW_I2C_CYCLES_PER_LOOP;  // Safety fallback
    }

    // --- High Period Calculation ---
    // Effective High Cycles = Target - Overhead
    uint32_t eff_cycles_high = (cycles_high > overhead) ? (cycles_high - overhead) : 0;
    uint32_t loops_high = (uint32_t) (eff_cycles_high / cpl);

    if (loops_high == 0) {
        loops_high = 1;  // Ensure at least 1 loop if possible, though overhead might dominate
    }

    // --- Low Period Distribution ---
    if (freq_khz >= 400) {
        // High Speed: Minimal Hold, Max Setup
        // Hold is typically short, often just overhead is enough.
        // We reserve minimal loops for Hold.

        // Calculate Hold Target (Minimal)
        // Let's target approx 1 * CPL + Overhead for Hold if possible,
        // essentially just function call overhead + 1 loop
        cycles_hold = (uint32_t) (cpl) + overhead;

        // But we shouldn't exceed reasonable part of low period
        if (cycles_hold > cycles_low / 2) {
            cycles_hold = cycles_low / 2;
        }

        cycles_setup = cycles_low - cycles_hold;
    } else {
        // Low Speed: Symmetric Hold/Setup
        cycles_hold = cycles_low / 2;
        cycles_setup = cycles_low - cycles_hold;
    }

    // --- Hold Period Calculation ---
    uint32_t eff_cycles_hold = (cycles_hold > overhead) ? (cycles_hold - overhead) : 0;
    uint32_t loops_hold = (uint32_t) (eff_cycles_hold / cpl);

    // --- Setup Period Calculation ---
    uint32_t eff_cycles_setup = (cycles_setup > overhead) ? (cycles_setup - overhead) : 0;
    uint32_t loops_setup = (uint32_t) (eff_cycles_setup / cpl);

    // Apply safety minimums
    if (loops_hold == 0) {
        loops_hold = 1;
    }
    if (loops_setup == 0) {
        loops_setup = 1;
    }

    i2c_dev->freq_khz = freq_khz;
    i2c_dev->ticks_high = loops_high;
    i2c_dev->ticks_hold = loops_hold;
    i2c_dev->ticks_setup = loops_setup;

    return SOFT_I2C_OK;
}

sw_i2c_err_t sw_i2c_scan(sw_i2c_t *i2c_dev) {
    SW_I2C_LOG("Scanning I2C bus...\r\n");
    sw_i2c_err_t ret;
    int found = 0;

    // Scan all 7-bit addresses
    for (uint8_t i = 0x00; i < 0x80; i++) {
        // Skip reserved addresses if needed (0x00-0x07, 0x78-0x7F)
        // But scanning everything can be useful for debugging.

        i2c_start(i2c_dev);
        ret = (sw_i2c_err_t) i2c_send_byte(i2c_dev, (i << 1));  // Write address
        i2c_stop(i2c_dev);

        if (ret == SOFT_I2C_OK) {
            SW_I2C_LOG("Device found at 0x%02X\r\n", i);
            found++;
        }
    }

    SW_I2C_LOG("Scan complete. Found %d device(s).\r\n", found);
    (void) found;
    return SOFT_I2C_OK;
}

sw_i2c_err_t sw_i2c_master_reset(sw_i2c_t *i2c_dev) {
    i2c_scl_write(i2c_dev, 1);
    i2c_sda_write(i2c_dev, 1);
    for (int i = 0; i < 9; i++) {
        i2c_scl_write(i2c_dev, 0);
        i2c_delay_hold(i2c_dev);
        i2c_delay_setup(i2c_dev);
        i2c_scl_write(i2c_dev, 1);
        i2c_delay_high(i2c_dev);
    }
    i2c_stop(i2c_dev);
    return SOFT_I2C_OK;
}

sw_i2c_err_t sw_i2c_master_write(sw_i2c_t *i2c_dev, const uint8_t *data, uint16_t len) {
    int ret;
    i2c_start(i2c_dev);

    ret = i2c_send_byte(i2c_dev, i2c_dev->dev_addr << 1);
    if (ret != SOFT_I2C_OK) {
        i2c_stop(i2c_dev);
        SW_I2C_LOG("I2C NACK at ADDR\r\n");
        return (sw_i2c_err_t) ret;
    }

    for (int i = 0; i < len; i++) {
        ret = i2c_send_byte(i2c_dev, data[i]);
        if (ret != SOFT_I2C_OK) {
            i2c_stop(i2c_dev);
            SW_I2C_LOG("I2C NACK at DATA[%d]\r\n", i);
            return (sw_i2c_err_t) ret;
        }
    }

    i2c_stop(i2c_dev);
    return SOFT_I2C_OK;
}

sw_i2c_err_t sw_i2c_master_read(sw_i2c_t *i2c_dev, uint8_t *data, uint16_t len) {
    int ret;
    i2c_start(i2c_dev);

    ret = i2c_send_byte(i2c_dev, (i2c_dev->dev_addr << 1) | 1);
    if (ret != SOFT_I2C_OK) {
        i2c_stop(i2c_dev);
        return (sw_i2c_err_t) ret;
    }

    for (int i = 0; i < len; i++) {
        data[i] = i2c_recv_byte(i2c_dev);
        if (i < len - 1) {
            i2c_send_ack(i2c_dev);
        } else {
            i2c_send_nack(i2c_dev);
        }
    }

    i2c_stop(i2c_dev);
    return SOFT_I2C_OK;
}

sw_i2c_err_t sw_i2c_master_mem_write(sw_i2c_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len,
                                     const uint8_t *data, uint16_t len) {
    int ret;
    i2c_start(i2c_dev);

    ret = i2c_send_byte(i2c_dev, i2c_dev->dev_addr << 1);
    if (ret != SOFT_I2C_OK) {
        goto error;
    }

    for (int i = addr_len - 1; i >= 0; i--) {
        ret = i2c_send_byte(i2c_dev, (mem_addr >> (8 * i)) & 0xFF);
        if (ret != SOFT_I2C_OK) {
            goto error;
        }
    }

    for (int i = 0; i < len; i++) {
        ret = i2c_send_byte(i2c_dev, data[i]);
        if (ret != SOFT_I2C_OK) {
            goto error;
        }
    }

    i2c_stop(i2c_dev);
    return SOFT_I2C_OK;

error:
    i2c_stop(i2c_dev);
    return (sw_i2c_err_t) ret;
}

sw_i2c_err_t sw_i2c_master_mem_read(sw_i2c_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len,
                                    uint8_t *data, uint16_t len) {
    int ret;
    i2c_start(i2c_dev);

    ret = i2c_send_byte(i2c_dev, i2c_dev->dev_addr << 1);
    if (ret != SOFT_I2C_OK) {
        goto error;
    }

    for (int i = addr_len - 1; i >= 0; i--) {
        ret = i2c_send_byte(i2c_dev, (mem_addr >> (8 * i)) & 0xFF);
        if (ret != SOFT_I2C_OK) {
            goto error;
        }
    }

    i2c_restart(i2c_dev);

    ret = i2c_send_byte(i2c_dev, (i2c_dev->dev_addr << 1) | 1);
    if (ret != SOFT_I2C_OK) {
        goto error;
    }

    for (int i = 0; i < len; i++) {
        data[i] = i2c_recv_byte(i2c_dev);
        if (i < len - 1) {
            i2c_send_ack(i2c_dev);
        } else {
            i2c_send_nack(i2c_dev);
        }
    }

    i2c_stop(i2c_dev);
    return SOFT_I2C_OK;

error:
    i2c_stop(i2c_dev);
    return (sw_i2c_err_t) ret;
}

sw_i2c_err_t sw_i2c_check_stuck(sw_i2c_t *i2c_dev) {
    if (!i2c_dev) {
        return SOFT_I2C_ERR_PARAM;
    }

    // 1. Set SCL High
    i2c_scl_write(i2c_dev, 1);
    i2c_delay_high(i2c_dev);

    // 2. Check if SCL is actually High (Clock Stretching check)
    if (i2c_scl_read(i2c_dev) == 0) {
        // SCL is stuck Low, likely master or another master holding it?
        // Or extreme clock stretching.
        return SOFT_I2C_ERR_BUS;
    }

    // 3. Check SDA
    if (i2c_sda_read(i2c_dev) == 0) {
        // SDA is Low while SCL is High -> Stuck
        return SOFT_I2C_ERR_BUS;
    }

    return SOFT_I2C_OK;
}

sw_i2c_err_t sw_i2c_unlock(sw_i2c_t *i2c_dev) {
    if (!i2c_dev) {
        return SOFT_I2C_ERR_PARAM;
    }

    // Try up to 9 clocks to clear stuck data (slave might be outputting Low bit)
    for (int i = 0; i < 9; i++) {
        // Check if released
        if (sw_i2c_check_stuck(i2c_dev) == SOFT_I2C_OK) {
            // Generate STOP to reset slave state machine
            i2c_stop(i2c_dev);
            return SOFT_I2C_OK;
        }

        // Toggle SCL
        i2c_scl_write(i2c_dev, 0);
        i2c_delay_hold(i2c_dev);
        i2c_delay_setup(i2c_dev);
        i2c_scl_write(i2c_dev, 1);
        i2c_delay_high(i2c_dev);
    }

    // Final check
    if (sw_i2c_check_stuck(i2c_dev) == SOFT_I2C_OK) {
        i2c_stop(i2c_dev);
        return SOFT_I2C_OK;
    }

    return SOFT_I2C_ERR_BUS;
}

sw_i2c_err_t sw_i2c_set_addr(sw_i2c_t *i2c_dev, uint16_t dev_addr) {
    if (!i2c_dev) {
        return SOFT_I2C_ERR_PARAM;
    }
    i2c_dev->dev_addr = dev_addr;
    return SOFT_I2C_OK;
}

uint16_t sw_i2c_get_addr(sw_i2c_t *i2c_dev) {
    return i2c_dev ? i2c_dev->dev_addr : 0;
}

sw_i2c_err_t sw_i2c_is_device_ready(sw_i2c_t *i2c_dev, uint16_t dev_addr) {
    sw_i2c_err_t ret;
    uint16_t original_addr;

    if (!i2c_dev) {
        return SOFT_I2C_ERR_PARAM;
    }

    // Backup current address
    original_addr = i2c_dev->dev_addr;

    // Set temporary address
    i2c_dev->dev_addr = dev_addr;

    // Attempt simple write (0 bytes) to check for ACK
    // Implementation detail: sw_i2c_master_write sends ADDR+W and checks ACK
    // If len is 0, it just pings the address.
    ret = sw_i2c_master_write(i2c_dev, NULL, 0);

    // Restore address
    i2c_dev->dev_addr = original_addr;

    return ret == SOFT_I2C_OK ? SOFT_I2C_OK : SOFT_I2C_ERR_NACK;
}

sw_i2c_err_t sw_i2c_write_reg8(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint8_t val) {
    if (!i2c_dev) {
        return SOFT_I2C_ERR_PARAM;
    }
    return sw_i2c_master_mem_write(i2c_dev, reg_addr, 1, &val, 1);
}

sw_i2c_err_t sw_i2c_read_reg8(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint8_t *val) {
    if (!i2c_dev || !val) {
        return SOFT_I2C_ERR_PARAM;
    }
    return sw_i2c_master_mem_read(i2c_dev, reg_addr, 1, val, 1);
}

sw_i2c_err_t sw_i2c_write_reg16(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint16_t val) {
    if (!i2c_dev) {
        return SOFT_I2C_ERR_PARAM;
    }
    uint8_t data[2];
    data[0] = (uint8_t) (val >> 8);
    data[1] = (uint8_t) (val & 0xFF);
    return sw_i2c_master_mem_write(i2c_dev, reg_addr, 1, data, 2);
}

sw_i2c_err_t sw_i2c_read_reg16(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint16_t *val) {
    if (!i2c_dev || !val) {
        return SOFT_I2C_ERR_PARAM;
    }
    uint8_t data[2];
    sw_i2c_err_t ret = sw_i2c_master_mem_read(i2c_dev, reg_addr, 1, data, 2);
    if (ret == SOFT_I2C_OK) {
        *val = (uint16_t) ((data[0] << 8) | data[1]);
    }
    return ret;
}

sw_i2c_err_t sw_i2c_clock_stretch_enable(sw_i2c_t *i2c_dev, bool enable) {
    if (!i2c_dev) {
        return SOFT_I2C_ERR_PARAM;
    }
    i2c_dev->enable_clock_stretch = enable;
    return SOFT_I2C_OK;
}
