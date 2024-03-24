/* File: startup_LM3S6965.c for GNU-ARM
 * Purpose: startup file for TM4C123GH6PM Cortex-M4 device.
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
void GPIOPortA_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortB_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortC_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortD_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortE_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMFault_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Watchdog_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer0A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer0B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer1A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer1B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer2A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer2B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp2_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SysCtrl_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void FlashCtrl_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortF_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortG_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortH_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer3A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer3B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Ethernet_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void Hibernate_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));



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
    (int)&GPIOPortA_IRQHandler,   /* GPIO Port A                     */
    (int)&GPIOPortB_IRQHandler,   /* GPIO Port B                     */
    (int)&GPIOPortC_IRQHandler,   /* GPIO Port C                     */
    (int)&GPIOPortD_IRQHandler,   /* GPIO Port D                     */
    (int)&GPIOPortE_IRQHandler,   /* GPIO Port E                     */
    (int)&UART0_IRQHandler,       /* UART0 Rx and Tx                 */
    (int)&UART1_IRQHandler,       /* UART1 Rx and Tx                 */
    (int)&SSI0_IRQHandler,        /* SSI0 Rx and Tx                  */
    (int)&I2C0_IRQHandler,        /* I2C0 Master and Slave           */
    (int)&PWMFault_IRQHandler,    /* PWM Fault                       */
    (int)&PWMGen0_IRQHandler,     /* PWM Generator 0                 */
    (int)&PWMGen1_IRQHandler,     /* PWM Generator 1                 */
    (int)&PWMGen2_IRQHandler,     /* PWM Generator 2                 */
    (int)&QEI0_IRQHandler,        /* Quadrature Encoder 0            */
    (int)&ADCSeq0_IRQHandler,     /* ADC Sequence 0                  */
    (int)&ADCSeq1_IRQHandler,     /* ADC Sequence 1                  */
    (int)&ADCSeq2_IRQHandler,     /* ADC Sequence 2                  */
    (int)&ADCSeq3_IRQHandler,     /* ADC Sequence 3                  */
    (int)&Watchdog_IRQHandler,    /* Watchdog timer                  */
    (int)&Timer0A_IRQHandler,     /* Timer 0 subtimer A              */
    (int)&Timer0B_IRQHandler,     /* Timer 0 subtimer B              */
    (int)&Timer1A_IRQHandler,     /* Timer 1 subtimer A              */
    (int)&Timer1B_IRQHandler,     /* Timer 1 subtimer B              */
    (int)&Timer2A_IRQHandler,     /* Timer 2 subtimer A              */
    (int)&Timer2B_IRQHandler,     /* Timer 2 subtimer B              */
    (int)&Comp0_IRQHandler,       /* Analog Comparator 0             */
    (int)&Comp1_IRQHandler,       /* Analog Comparator 1             */
    (int)&Comp2_IRQHandler,       /* Analog Comparator 2             */
    (int)&SysCtrl_IRQHandler,     /* System Control (PLL, OSC, BO)   */
    (int)&FlashCtrl_IRQHandler,   /* FLASH Control                   */
    (int)&GPIOPortF_IRQHandler,   /* GPIO Port F                     */
    (int)&GPIOPortG_IRQHandler,   /* GPIO Port G                     */
    (int)&GPIOPortH_IRQHandler,   /* GPIO Port H                     */
    (int)&UART2_IRQHandler,       /* UART2 Rx and Tx                 */
    (int)&SSI1_IRQHandler,        /* SSI1 Rx and Tx                  */
    (int)&Timer3A_IRQHandler,     /* Timer 3 subtimer A              */
    (int)&Timer3B_IRQHandler,     /* Timer 3 subtimer B              */
    (int)&I2C1_IRQHandler,        /* I2C1 Master and Slave           */
    (int)&QEI1_IRQHandler,        /* Quadrature Encoder 1            */
    (int)&CAN0_IRQHandler,        /* CAN0                            */
    (int)&CAN1_IRQHandler,        /* CAN1                            */
    (int)&CAN2_IRQHandler,        /* CAN2                            */
    (int)&Ethernet_IRQHandler,    /* Ethernet                        */
    (int)&Hibernate_IRQHandler,   /* Hibernate                       */
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
