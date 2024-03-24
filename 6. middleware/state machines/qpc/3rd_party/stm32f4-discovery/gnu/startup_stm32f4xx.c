/* File: startup_stm32f4xx.c for GNU-ARM
 * Purpose: startup file for STM32F4xx Cortex-M4F device.
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
void WWDG_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void PVD_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void FLASH_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void RCC_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI3_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI4_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream0_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream1_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream2_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream3_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream4_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream5_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream6_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN1_TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN1_RX0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_BRK_TIM9_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_UP_TIM10_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM2_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM3_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM4_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI2_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void USART1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void USART2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void USART3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_WKUP_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_BRK_TIM12_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_UP_TIM13_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_TRG_COM_TIM14_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_CC_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream7_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void FSMC_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM5_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI3_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void UART4_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART5_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM7_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream0_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream1_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream2_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream3_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream4_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void ETH_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void ETH_WKUP_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN2_TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN2_RX0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN2_RX1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN2_SCE_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream5_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream6_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream7_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void USART6_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_EP1_OUT_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_EP1_IN_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_WKUP_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void DCMI_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void CRYP_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void HASH_RNG_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void FPU_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));


/*..........................................................................*/
__attribute__ ((section(".isr_vector")))
int const g_pfnVectors[] = {
    (int)&__stack_end__,            /* Top of Stack                        */
    (int)&Reset_Handler,            /* Reset Handler                       */
    (int)&NMI_Handler,              /* NMI Handler                         */
    (int)&HardFault_Handler,        /* Hard Fault Handler                  */
    (int)&MemManage_Handler,        /* The MPU fault handler               */
    (int)&BusFault_Handler,         /* The bus fault handler               */
    (int)&UsageFault_Handler,       /* The usage fault handler             */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&SVC_Handler,              /* SVCall handler                      */
    (int)&DebugMon_Handler,         /* Debug monitor handler               */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&PendSV_Handler,           /* The PendSV handler                  */
    (int)&SysTick_Handler,          /* The SysTick handler                 */

    /*IRQ handlers... */
    (int)&WWDG_IRQHandler,          /* Window WatchDog                  */
    (int)&PVD_IRQHandler,           /* PVD through EXTI Line detection  */
    (int)&TAMP_STAMP_IRQHandler,    /* Tamper and TimeStamps through the EXTI line */
    (int)&RTC_WKUP_IRQHandler,      /* RTC Wakeup through the EXTI line */
    (int)&FLASH_IRQHandler,         /* FLASH                            */
    (int)&RCC_IRQHandler,           /* RCC                              */
    (int)&EXTI0_IRQHandler,         /* EXTI Line0                       */
    (int)&EXTI1_IRQHandler,         /* EXTI Line1                       */
    (int)&EXTI2_IRQHandler,         /* EXTI Line2                       */
    (int)&EXTI3_IRQHandler,         /* EXTI Line3                       */
    (int)&EXTI4_IRQHandler,         /* EXTI Line4                       */
    (int)&DMA1_Stream0_IRQHandler,  /* DMA1 Stream 0                    */
    (int)&DMA1_Stream1_IRQHandler,  /* DMA1 Stream 1                    */
    (int)&DMA1_Stream2_IRQHandler,  /* DMA1 Stream 2                    */
    (int)&DMA1_Stream3_IRQHandler,  /* DMA1 Stream 3                    */
    (int)&DMA1_Stream4_IRQHandler,  /* DMA1 Stream 4                    */
    (int)&DMA1_Stream5_IRQHandler,  /* DMA1 Stream 5                    */
    (int)&DMA1_Stream6_IRQHandler,  /* DMA1 Stream 6                    */
    (int)&ADC_IRQHandler,           /* ADC1, ADC2 and ADC3s             */
    (int)&CAN1_TX_IRQHandler,       /* CAN1 TX                          */
    (int)&CAN1_RX0_IRQHandler,      /* CAN1 RX0                         */
    (int)&CAN1_RX1_IRQHandler,      /* CAN1 RX1                         */
    (int)&CAN1_SCE_IRQHandler,      /* CAN1 SCE                         */
    (int)&EXTI9_5_IRQHandler,       /* External Line[9:5]s              */
    (int)&TIM1_BRK_TIM9_IRQHandler, /* TIM1 Break and TIM9              */
    (int)&TIM1_UP_TIM10_IRQHandler, /* TIM1 Update and TIM10            */
    (int)&TIM1_TRG_COM_TIM11_IRQHandler, /* TIM1 Trigger and Commutation and TIM11 */
    (int)&TIM1_CC_IRQHandler,       /* TIM1 Capture Compare             */
    (int)&TIM2_IRQHandler,          /* TIM2                             */
    (int)&TIM3_IRQHandler,          /* TIM3                             */
    (int)&TIM4_IRQHandler,          /* TIM4                             */
    (int)&I2C1_EV_IRQHandler,       /* I2C1 Event                       */
    (int)&I2C1_ER_IRQHandler,       /* I2C1 Error                       */
    (int)&I2C2_EV_IRQHandler,       /* I2C2 Event                       */
    (int)&I2C2_ER_IRQHandler,       /* I2C2 Error                       */
    (int)&SPI1_IRQHandler,          /* SPI1                             */
    (int)&SPI2_IRQHandler,          /* SPI2                             */
    (int)&USART1_IRQHandler,        /* USART1                           */
    (int)&USART2_IRQHandler,        /* USART2                           */
    (int)&USART3_IRQHandler,        /* USART3                           */
    (int)&EXTI15_10_IRQHandler,     /* External Line[15:10]s            */
    (int)&RTC_Alarm_IRQHandler,     /* RTC Alarm (A and B) through EXTI Line */
    (int)&OTG_FS_WKUP_IRQHandler,   /* USB OTG FS Wakeup through EXTI line   */
    (int)&TIM8_BRK_TIM12_IRQHandler,/* TIM8 Break and TIM12             */
    (int)&TIM8_UP_TIM13_IRQHandler, /* TIM8 Update and TIM13            */
    (int)&TIM8_TRG_COM_TIM14_IRQHandler, /* TIM8 Trigger and Commutation and TIM14 */
    (int)&TIM8_CC_IRQHandler,       /* TIM8 Capture Compare             */
    (int)&DMA1_Stream7_IRQHandler,  /* DMA1 Stream7                     */
    (int)&FSMC_IRQHandler,          /* FSMC                             */
    (int)&SDIO_IRQHandler,          /* SDIO                             */
    (int)&TIM5_IRQHandler,          /* TIM5                             */
    (int)&SPI3_IRQHandler,          /* SPI3                             */
    (int)&UART4_IRQHandler,         /* UART4                            */
    (int)&UART5_IRQHandler,         /* UART5                            */
    (int)&TIM6_DAC_IRQHandler,      /* TIM6 and DAC1&2 underrun errors  */
    (int)&TIM7_IRQHandler,          /* TIM7                             */
    (int)&DMA2_Stream0_IRQHandler,  /* DMA2 Stream 0                    */
    (int)&DMA2_Stream1_IRQHandler,  /* DMA2 Stream 1                    */
    (int)&DMA2_Stream2_IRQHandler,  /* DMA2 Stream 2                    */
    (int)&DMA2_Stream3_IRQHandler,  /* DMA2 Stream 3                    */
    (int)&DMA2_Stream4_IRQHandler,  /* DMA2 Stream 4                    */
    (int)&ETH_IRQHandler,           /* Ethernet                         */
    (int)&ETH_WKUP_IRQHandler,      /* Ethernet Wakeup through EXTI line*/
    (int)&CAN2_TX_IRQHandler,       /* CAN2 TX                          */
    (int)&CAN2_RX0_IRQHandler,      /* CAN2 RX0                         */
    (int)&CAN2_RX1_IRQHandler,      /* CAN2 RX1                         */
    (int)&CAN2_SCE_IRQHandler,      /* CAN2 SCE                         */
    (int)&OTG_FS_IRQHandler,        /* USB OTG FS                       */
    (int)&DMA2_Stream5_IRQHandler,  /* DMA2 Stream 5                    */
    (int)&DMA2_Stream6_IRQHandler,  /* DMA2 Stream 6                    */
    (int)&DMA2_Stream7_IRQHandler,  /* DMA2 Stream 7                    */
    (int)&USART6_IRQHandler,        /* USART6                           */
    (int)&I2C3_EV_IRQHandler,       /* I2C3 event                       */
    (int)&I2C3_ER_IRQHandler,       /* I2C3 error                       */
    (int)&OTG_HS_EP1_OUT_IRQHandler,/* USB OTG HS End Point 1 Out       */
    (int)&OTG_HS_EP1_IN_IRQHandler, /* USB OTG HS End Point 1 In        */
    (int)&OTG_HS_WKUP_IRQHandler,   /* USB OTG HS Wakeup through EXTI   */
    (int)&OTG_HS_IRQHandler,        /* USB OTG HS                       */
    (int)&DCMI_IRQHandler,          /* DCMI                             */
    (int)&CRYP_IRQHandler,          /* CRYP crypto                      */
    (int)&HASH_RNG_IRQHandler,      /* Hash and Rng                     */
    (int)&FPU_IRQHandler,           /* FPU                              */
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
