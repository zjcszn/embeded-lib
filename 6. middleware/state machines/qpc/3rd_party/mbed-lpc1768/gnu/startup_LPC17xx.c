/* File: startup_LPC17xx.c for GNU-ARM
 * Purpose: startup file for LPC17xx Cortex-M4 device.
 *          Should be used with GCC 'GNU Tools ARM Embedded'
 * Version: CMSIS 5.0.1
 * Date: 2017-09-13
 *
 * Modified by Quantum Leaps:
 * - Added relocating of the Vector Table to free up the 256B region at 0x0
 *   for NULL-pointer protection by the MPU.
 * - Modified all exception handlers to branch to assert_failed()
 *   instead of locking up the CPU inside an endless loop.
 *
 * Created from the CMSIS template for the specified device
 * Quantum Leaps, www.state-machine.com
 *
 * NOTE:
 * The function assert_failed defined at the end of this file defines
 * the error/assertion handling policy for the application and might
 * need to be customized for each project. This function is defined in
 * assembly to re-set the stack pointer, in case it is corrupted by the
 * time assert_failed is called.
 */

/* start and end of stack defined in the linker script ---------------------*/
/*extern int __stack_start__;*/
extern int __stack_end__;

/* Weak prototypes for error handlers --------------------------------------*/
/**
* \note
* The function assert_failed defined at the end of this file defines
* the error/assertion handling policy for the application and might
* need to be customized for each project. This function is defined in
* assembly to avoid accessing the stack, which might be corrupted by
* the time assert_failed is called.
*/
__attribute__ ((naked, noreturn))
void assert_failed(char const *module, int loc);

/* Function prototypes -----------------------------------------------------*/
void Default_Handler(void);  /* Default empty handler */
void Reset_Handler(void);    /* Reset Handler */
void SystemInit(void);       /* CMSIS system initialization */

/*----------------------------------------------------------------------------
* weak aliases for each Exception handler to the Default_Handler.
* Any function with the same name will override these definitions.
*/
/* Cortex-M Processor fault exceptions... */
void NMI_Handler           (void) __attribute__ ((weak));
void HardFault_Handler     (void) __attribute__ ((weak));
void MemManage_Handler     (void) __attribute__ ((weak));
void BusFault_Handler      (void) __attribute__ ((weak));
void UsageFault_Handler    (void) __attribute__ ((weak));

/* Cortex-M Processor non-fault exceptions... */
void SVC_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));

/* external interrupts...   */
void WDT_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER2_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER3_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SSP0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SSP1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void PLL0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void EINT0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void EINT1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void EINT2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void EINT3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void BOD_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void USB_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void I2S_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void ENET_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void RIT_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void MCPWM_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void PLL1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void USBActivity_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void CANActivity_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));


/*..........................................................................*/
__attribute__ ((section(".isr_vector")))
int const g_pfnVectors[] = {
    /* Initial Vector Table before relocation */
    (int)&__stack_end__,          /* Top of Stack                    */
    (int)&Reset_Handler,          /* Reset Handler                   */
    (int)&NMI_Handler,            /* NMI Handler                     */
    (int)&HardFault_Handler,      /* Hard Fault Handler              */
    (int)&MemManage_Handler,      /* The MPU fault handler           */
    (int)&BusFault_Handler,       /* The bus fault handler           */
    (int)&UsageFault_Handler,     /* The usage fault handler         */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&SVC_Handler,            /* SVCall handler                  */
    (int)&DebugMon_Handler,       /* Debug monitor handler           */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&PendSV_Handler,         /* The PendSV handler              */
    (int)&SysTick_Handler,        /* The SysTick handler             */
    /* pad the initial VT to the total size of 256B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    /* Relocated Vector Table beyond the 256B region around address 0.
    * That region is used for NULL-pointer protection by the MPU.
    */
    (int)&__stack_end__,          /* Top of Stack                    */
    (int)&Reset_Handler,          /* Reset Handler                   */
    (int)&NMI_Handler,            /* NMI Handler                     */
    (int)&HardFault_Handler,      /* Hard Fault Handler              */
    (int)&MemManage_Handler,      /* The MPU fault handler           */
    (int)&BusFault_Handler,       /* The bus fault handler           */
    (int)&UsageFault_Handler,     /* The usage fault handler         */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&SVC_Handler,            /* SVCall handler                  */
    (int)&DebugMon_Handler,       /* Debug monitor handler           */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&PendSV_Handler,         /* The PendSV handler              */
    (int)&SysTick_Handler,        /* The SysTick handler             */

    /*IRQ handlers... */
    (int)&WDT_IRQHandler,         /* WDT                              */
    (int)&TIMER0_IRQHandler,      /* TIMER0                           */
    (int)&TIMER1_IRQHandler,      /* TIMER1                           */
    (int)&TIMER2_IRQHandler,      /* TIMER2                           */
    (int)&TIMER3_IRQHandler,      /* TIMER3                           */
    (int)&UART0_IRQHandler,       /* UART0                            */
    (int)&UART1_IRQHandler,       /* UART1                            */
    (int)&UART2_IRQHandler,       /* UART2                            */
    (int)&UART3_IRQHandler,       /* UART3                            */
    (int)&PWM1_IRQHandler,        /* PWM1                             */
    (int)&I2C0_IRQHandler,        /* I2C0                             */
    (int)&I2C1_IRQHandler,        /* I2C1                             */
    (int)&I2C2_IRQHandler,        /* I2C2                             */
    (int)&SPI_IRQHandler,         /* SPI                              */
    (int)&SSP0_IRQHandler,        /* SSP0                             */
    (int)&SSP1_IRQHandler,        /* SSP1                             */
    (int)&PLL0_IRQHandler,        /* PLL0 (Main PLL)                  */
    (int)&RTC_IRQHandler,         /* RTC                              */
    (int)&EINT0_IRQHandler,       /* EINT0                            */
    (int)&EINT1_IRQHandler,       /* EINT1                            */
    (int)&EINT2_IRQHandler,       /* EINT2                            */
    (int)&EINT3_IRQHandler,       /* EINT3                            */
    (int)&ADC_IRQHandler,         /* ADC                              */
    (int)&BOD_IRQHandler,         /* BOD                              */
    (int)&USB_IRQHandler,         /* USB                              */
    (int)&CAN_IRQHandler,         /* CAN                              */
    (int)&DMA_IRQHandler,         /* GP DMA                           */
    (int)&I2S_IRQHandler,         /* I2S                              */
    (int)&ENET_IRQHandler,        /* Ethernet                         */
    (int)&RIT_IRQHandler,         /* RITINT                           */
    (int)&MCPWM_IRQHandler,       /* Motor Control PWM                */
    (int)&QEI_IRQHandler,         /* Quadrature Encoder               */
    (int)&PLL1_IRQHandler,        /* PLL1 (USB PLL)                   */
    (int)&USBActivity_IRQHandler, /* USB Activity interrupt to wakeup */
    (int)&CANActivity_IRQHandler, /* CAN Activity interrupt to wakeup */
};

/* reset handler -----------------------------------------------------------*/
__attribute__((naked)) void Reset_Handler(void);
void Reset_Handler(void) {
    extern int main(void);
    extern int __libc_init_array(void);
    extern unsigned __data_start;  /* start of .data in the linker script */
    extern unsigned __data_end__;  /* end of .data in the linker script */
    extern unsigned const __data_load; /* initialization values for .data  */
    extern unsigned __bss_start__; /* start of .bss in the linker script */
    extern unsigned __bss_end__;   /* end of .bss in the linker script */
    extern void software_init_hook(void) __attribute__((weak));

    /* relocate the Vector Table to leave room for the NULL-pointer region
    * System Control Block/Vector Table Offset Reg := relocated Vector Table
    */
    *(int const * volatile *)0xE000ED08 = &g_pfnVectors[256/sizeof(int)];

    SystemInit(); /* CMSIS system initialization */

    /* copy the data segment initializers from flash to RAM... */
    unsigned const *src = &__data_load;
    unsigned *dst;
    for (dst = &__data_start; dst < &__data_end__; ++dst, ++src) {
        *dst = *src;
    }

    /* zero fill the .bss segment in RAM... */
    for (dst = &__bss_start__; dst < &__bss_end__; ++dst) {
        *dst = 0;
    }

    /* init hook provided? */
    if (&software_init_hook != (void (*)(void))(0)) {
        /* give control to the RTOS */
        software_init_hook(); /* this will also call __libc_init_array */
    }
    else {
        /* call all static constructors in C++ (harmless in C programs) */
        __libc_init_array();
        (void)main(); /* application's entry point; should never return! */
    }

    /* the previous code should not return, but assert just in case... */
    __asm volatile ("  CPSID i");
    assert_failed("Reset_Handler", 1U);
}

/* fault exception handlers ------------------------------------------------*/
__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("NMI", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void HardFault_Handler(void);
void HardFault_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("HardFault", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void MemManage_Handler(void);
void MemManage_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("MemManage", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void BusFault_Handler(void);
void BusFault_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("BusFault", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void UsageFault_Handler(void);
void UsageFault_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("UsageFault", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("Default", 1U);
}
