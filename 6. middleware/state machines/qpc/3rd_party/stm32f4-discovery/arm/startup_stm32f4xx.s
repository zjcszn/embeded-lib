;/***************************************************************************/
; @file     startup_stm32f4xx.s for ARM-KEIL ARM assembler
; @brief    CMSIS Cortex-M4F Core Device Startup File for STM32F40xx devices
; @version  CMSIS 5.9.0
; @date     1 Feb 2023
;
; Modified by Quantum Leaps:
; - Added relocating of the Vector Table to free up the 256B region at 0x0
;   for NULL-pointer protection by the MPU.
; - Modified all exception handlers to branch to assert_failed()
;   instead of locking up the CPU inside an endless loop.
;
; @description
; Created from the CMSIS template for the specified device
; Quantum Leaps, www.state-machine.com
;
; @note
; The symbols Stack_Size and Heap_Size should be provided on the command-
; line options to the assembler, for example as:
;     --pd "Stack_Size SETA 1024" --pd "Heap_Size SETA 0"


;******************************************************************************
; Allocate space for the stack.
;
        AREA    STACK, NOINIT, READWRITE, ALIGN=3
__stack_base
StackMem
        SPACE   Stack_Size    ; provided in command-line option, for example:
                              ; --pd "Stack_Size SETA 512"
__stack_limit
__initial_sp

;******************************************************************************
; Allocate space for the heap.
;
        AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
HeapMem
        SPACE   Heap_Size     ; provided in command-line option, for example:
                              ; --pd "Heap_Size SETA 0"
__heap_limit

; Indicate that the code in this file preserves 8-byte alignment of the stack.
        PRESERVE8

;******************************************************************************
; The vector table.
;
; Place code into the reset code section.
        AREA   RESET, DATA, READONLY, ALIGN=8
        EXPORT  __Vectors
        EXPORT  __Vectors_End
        EXPORT  __Vectors_Size

__Vectors
        DCD     __initial_sp              ; Top of Stack
        DCD     Reset_Handler             ; Reset Handler
        DCD     NMI_Handler               ; NMI Handler
        DCD     HardFault_Handler         ; Hard Fault Handler
        DCD     MemManage_Handler         ; MPU Fault Handler
        DCD     BusFault_Handler          ; Bus Fault Handler
        DCD     UsageFault_Handler        ; Usage Fault Handler
        DCD     Default_Handler           ; Reserved
        DCD     Default_Handler           ; Reserved
        DCD     Default_Handler           ; Reserved
        DCD     Default_Handler           ; Reserved
        DCD     SVC_Handler               ; SVCall Handler
        DCD     DebugMon_Handler          ; Debug Monitor Handler
        DCD     Default_Handler           ; Reserved
        DCD     PendSV_Handler            ; PendSV Handler
        DCD     SysTick_Handler           ; SysTick Handler

        ; IRQ handlers...
        DCD     WWDG_IRQHandler            ; Window WatchDog
        DCD     PVD_IRQHandler             ; PVD through EXTI Line detection
        DCD     TAMP_STAMP_IRQHandler      ; Tamper and TimeStamps through the EXTI line
        DCD     RTC_WKUP_IRQHandler        ; RTC Wakeup through the EXTI line
        DCD     FLASH_IRQHandler           ; FLASH
        DCD     RCC_IRQHandler             ; RCC
        DCD     EXTI0_IRQHandler           ; EXTI Line0
        DCD     EXTI1_IRQHandler           ; EXTI Line1
        DCD     EXTI2_IRQHandler           ; EXTI Line2
        DCD     EXTI3_IRQHandler           ; EXTI Line3
        DCD     EXTI4_IRQHandler           ; EXTI Line4
        DCD     DMA1_Stream0_IRQHandler    ; DMA1 Stream 0
        DCD     DMA1_Stream1_IRQHandler    ; DMA1 Stream 1
        DCD     DMA1_Stream2_IRQHandler    ; DMA1 Stream 2
        DCD     DMA1_Stream3_IRQHandler    ; DMA1 Stream 3
        DCD     DMA1_Stream4_IRQHandler    ; DMA1 Stream 4
        DCD     DMA1_Stream5_IRQHandler    ; DMA1 Stream 5
        DCD     DMA1_Stream6_IRQHandler    ; DMA1 Stream 6
        DCD     ADC_IRQHandler             ; ADC1, ADC2 and ADC3s
        DCD     CAN1_TX_IRQHandler         ; CAN1 TX
        DCD     CAN1_RX0_IRQHandler        ; CAN1 RX0
        DCD     CAN1_RX1_IRQHandler        ; CAN1 RX1
        DCD     CAN1_SCE_IRQHandler        ; CAN1 SCE
        DCD     EXTI9_5_IRQHandler         ; External Line[9:5]s
        DCD     TIM1_BRK_TIM9_IRQHandler   ; TIM1 Break and TIM9
        DCD     TIM1_UP_TIM10_IRQHandler   ; TIM1 Update and TIM10
        DCD     TIM1_TRG_COM_TIM11_IRQHandler ; TIM1 Trigger and Commutation and TIM11
        DCD     TIM1_CC_IRQHandler         ; TIM1 Capture Compare
        DCD     TIM2_IRQHandler            ; TIM2
        DCD     TIM3_IRQHandler            ; TIM3
        DCD     TIM4_IRQHandler            ; TIM4
        DCD     I2C1_EV_IRQHandler         ; I2C1 Event
        DCD     I2C1_ER_IRQHandler         ; I2C1 Error
        DCD     I2C2_EV_IRQHandler         ; I2C2 Event
        DCD     I2C2_ER_IRQHandler         ; I2C2 Error
        DCD     SPI1_IRQHandler            ; SPI1
        DCD     SPI2_IRQHandler            ; SPI2
        DCD     USART1_IRQHandler          ; USART1
        DCD     USART2_IRQHandler          ; USART2
        DCD     USART3_IRQHandler          ; USART3
        DCD     EXTI15_10_IRQHandler       ; External Line[15:10]s
        DCD     RTC_Alarm_IRQHandler       ; RTC Alarm (A and B) through EXTI Line
        DCD     OTG_FS_WKUP_IRQHandler     ; USB OTG FS Wakeup through EXTI line
        DCD     TIM8_BRK_TIM12_IRQHandler  ; TIM8 Break and TIM12
        DCD     TIM8_UP_TIM13_IRQHandler   ; TIM8 Update and TIM13
        DCD     TIM8_TRG_COM_TIM14_IRQHandler ; TIM8 Trigger and Commutation and TIM14
        DCD     TIM8_CC_IRQHandler         ; TIM8 Capture Compare
        DCD     DMA1_Stream7_IRQHandler    ; DMA1 Stream7
        DCD     FSMC_IRQHandler            ; FSMC
        DCD     SDIO_IRQHandler            ; SDIO
        DCD     TIM5_IRQHandler            ; TIM5
        DCD     SPI3_IRQHandler            ; SPI3
        DCD     UART4_IRQHandler           ; UART4
        DCD     UART5_IRQHandler           ; UART5
        DCD     TIM6_DAC_IRQHandler        ; TIM6 and DAC1&2 underrun errors
        DCD     TIM7_IRQHandler            ; TIM7
        DCD     DMA2_Stream0_IRQHandler    ; DMA2 Stream 0
        DCD     DMA2_Stream1_IRQHandler    ; DMA2 Stream 1
        DCD     DMA2_Stream2_IRQHandler    ; DMA2 Stream 2
        DCD     DMA2_Stream3_IRQHandler    ; DMA2 Stream 3
        DCD     DMA2_Stream4_IRQHandler    ; DMA2 Stream 4
        DCD     ETH_IRQHandler             ; Ethernet
        DCD     ETH_WKUP_IRQHandler        ; Ethernet Wakeup through EXTI line
        DCD     CAN2_TX_IRQHandler         ; CAN2 TX
        DCD     CAN2_RX0_IRQHandler        ; CAN2 RX0
        DCD     CAN2_RX1_IRQHandler        ; CAN2 RX1
        DCD     CAN2_SCE_IRQHandler        ; CAN2 SCE
        DCD     OTG_FS_IRQHandler          ; USB OTG FS
        DCD     DMA2_Stream5_IRQHandler    ; DMA2 Stream 5
        DCD     DMA2_Stream6_IRQHandler    ; DMA2 Stream 6
        DCD     DMA2_Stream7_IRQHandler    ; DMA2 Stream 7
        DCD     USART6_IRQHandler          ; USART6
        DCD     I2C3_EV_IRQHandler         ; I2C3 event
        DCD     I2C3_ER_IRQHandler         ; I2C3 error
        DCD     OTG_HS_EP1_OUT_IRQHandler  ; USB OTG HS End Point 1 Out
        DCD     OTG_HS_EP1_IN_IRQHandler   ; USB OTG HS End Point 1 In
        DCD     OTG_HS_WKUP_IRQHandler     ; USB OTG HS Wakeup through EXTI
        DCD     OTG_HS_IRQHandler          ; USB OTG HS
        DCD     DCMI_IRQHandler            ; DCMI
        DCD     CRYP_IRQHandler            ; CRYP crypto
        DCD     HASH_RNG_IRQHandler        ; Hash and Rng
        DCD     FPU_IRQHandler             ; FPU

__Vectors_End

__Vectors_Size  EQU   __Vectors_End - __Vectors


;******************************************************************************
; This is the code for exception handlers.
;
        AREA    |.text|, CODE, READONLY

;******************************************************************************
; This is the code that gets called when the processor first starts execution
; following a reset event.
;
Reset_Handler   PROC
        EXPORT  Reset_Handler   [WEAK]
        IMPORT  SystemInit
        IMPORT  __main
        IMPORT  assert_failed

        LDR     r0,=SystemInit  ; CMSIS system initialization
        BLX     r0

        ; Call the C library entry point that handles startup. This will copy
        ; the .data section initializers from flash to SRAM and zero fill the
        ; .bss section.
        ; NOTE: The __main function clears the C stack as well
        LDR     r0,=__main
        BX      r0

        ; __main calls the main() function, which should not return,
        ; but just in case jump to assert_failed() if main returns.
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_EXIT
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_EXIT
        DCB     "EXIT"
        ALIGN
        ENDP

;******************************************************************************
NMI_Handler     PROC
        EXPORT  NMI_Handler     [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_NMI
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_NMI
        DCB     "NMI"
        ALIGN
        ENDP

;******************************************************************************
HardFault_Handler PROC
        EXPORT  HardFault_Handler [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_HardFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_HardFault
        DCB     "HardFault"
        ALIGN
        ENDP

;******************************************************************************
MemManage_Handler PROC
        EXPORT  MemManage_Handler [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_MemManage
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_MemManage
        DCB     "MemManage"
        ALIGN
        ENDP

;******************************************************************************
BusFault_Handler PROC
        EXPORT  BusFault_Handler  [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_BusFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_BusFault
        DCB     "BusFault"
        ALIGN
        ENDP

;******************************************************************************
UsageFault_Handler PROC
        EXPORT  UsageFault_Handler [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_UsageFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_UsageFault
        DCB     "UsageFault"
        ALIGN
        ENDP

;******************************************************************************
;
; Weak non-fault handlers...
;

;******************************************************************************
SVC_Handler PROC
        EXPORT  SVC_Handler       [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_SVC
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SVC
        DCB     "SVC"
        ALIGN
        ENDP

;******************************************************************************
DebugMon_Handler PROC
        EXPORT  DebugMon_Handler  [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_DebugMon
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_DebugMon
        DCB     "DebugMon"
        ALIGN
        ENDP

;******************************************************************************
PendSV_Handler PROC
        EXPORT  PendSV_Handler    [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_PendSV
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_PendSV
        DCB     "PendSV"
        ALIGN
        ENDP

;******************************************************************************
SysTick_Handler PROC
        EXPORT  SysTick_Handler   [WEAK]
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_SysTick
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SysTick
        DCB     "SysTick"
        ALIGN
        ENDP

;******************************************************************************
Default_Handler PROC
        EXPORT  WWDG_IRQHandler               [WEAK]
        EXPORT  PVD_IRQHandler                [WEAK]
        EXPORT  TAMP_STAMP_IRQHandler         [WEAK]
        EXPORT  RTC_WKUP_IRQHandler           [WEAK]
        EXPORT  FLASH_IRQHandler              [WEAK]
        EXPORT  RCC_IRQHandler                [WEAK]
        EXPORT  EXTI0_IRQHandler              [WEAK]
        EXPORT  EXTI1_IRQHandler              [WEAK]
        EXPORT  EXTI2_IRQHandler              [WEAK]
        EXPORT  EXTI3_IRQHandler              [WEAK]
        EXPORT  EXTI4_IRQHandler              [WEAK]
        EXPORT  DMA1_Stream0_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream1_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream2_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream3_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream4_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream5_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream6_IRQHandler       [WEAK]
        EXPORT  ADC_IRQHandler                [WEAK]
        EXPORT  CAN1_TX_IRQHandler            [WEAK]
        EXPORT  CAN1_RX0_IRQHandler           [WEAK]
        EXPORT  CAN1_RX1_IRQHandler           [WEAK]
        EXPORT  CAN1_SCE_IRQHandler           [WEAK]
        EXPORT  EXTI9_5_IRQHandler            [WEAK]
        EXPORT  TIM1_BRK_TIM9_IRQHandler      [WEAK]
        EXPORT  TIM1_UP_TIM10_IRQHandler      [WEAK]
        EXPORT  TIM1_TRG_COM_TIM11_IRQHandler [WEAK]
        EXPORT  TIM1_CC_IRQHandler            [WEAK]
        EXPORT  TIM2_IRQHandler               [WEAK]
        EXPORT  TIM3_IRQHandler               [WEAK]
        EXPORT  TIM4_IRQHandler               [WEAK]
        EXPORT  I2C1_EV_IRQHandler            [WEAK]
        EXPORT  I2C1_ER_IRQHandler            [WEAK]
        EXPORT  I2C2_EV_IRQHandler            [WEAK]
        EXPORT  I2C2_ER_IRQHandler            [WEAK]
        EXPORT  SPI1_IRQHandler               [WEAK]
        EXPORT  SPI2_IRQHandler               [WEAK]
        EXPORT  USART1_IRQHandler             [WEAK]
        EXPORT  USART2_IRQHandler             [WEAK]
        EXPORT  USART3_IRQHandler             [WEAK]
        EXPORT  EXTI15_10_IRQHandler          [WEAK]
        EXPORT  RTC_Alarm_IRQHandler          [WEAK]
        EXPORT  OTG_FS_WKUP_IRQHandler        [WEAK]
        EXPORT  TIM8_BRK_TIM12_IRQHandler     [WEAK]
        EXPORT  TIM8_UP_TIM13_IRQHandler      [WEAK]
        EXPORT  TIM8_TRG_COM_TIM14_IRQHandler [WEAK]
        EXPORT  TIM8_CC_IRQHandler            [WEAK]
        EXPORT  DMA1_Stream7_IRQHandler       [WEAK]
        EXPORT  FSMC_IRQHandler               [WEAK]
        EXPORT  SDIO_IRQHandler               [WEAK]
        EXPORT  TIM5_IRQHandler               [WEAK]
        EXPORT  SPI3_IRQHandler               [WEAK]
        EXPORT  UART4_IRQHandler              [WEAK]
        EXPORT  UART5_IRQHandler              [WEAK]
        EXPORT  TIM6_DAC_IRQHandler           [WEAK]
        EXPORT  TIM7_IRQHandler               [WEAK]
        EXPORT  DMA2_Stream0_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream1_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream2_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream3_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream4_IRQHandler       [WEAK]
        EXPORT  ETH_IRQHandler                [WEAK]
        EXPORT  ETH_WKUP_IRQHandler           [WEAK]
        EXPORT  CAN2_TX_IRQHandler            [WEAK]
        EXPORT  CAN2_RX0_IRQHandler           [WEAK]
        EXPORT  CAN2_RX1_IRQHandler           [WEAK]
        EXPORT  CAN2_SCE_IRQHandler           [WEAK]
        EXPORT  OTG_FS_IRQHandler             [WEAK]
        EXPORT  DMA2_Stream5_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream6_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream7_IRQHandler       [WEAK]
        EXPORT  USART6_IRQHandler             [WEAK]
        EXPORT  I2C3_EV_IRQHandler            [WEAK]
        EXPORT  I2C3_ER_IRQHandler            [WEAK]
        EXPORT  OTG_HS_EP1_OUT_IRQHandler     [WEAK]
        EXPORT  OTG_HS_EP1_IN_IRQHandler      [WEAK]
        EXPORT  OTG_HS_WKUP_IRQHandler        [WEAK]
        EXPORT  OTG_HS_IRQHandler             [WEAK]
        EXPORT  DCMI_IRQHandler               [WEAK]
        EXPORT  CRYP_IRQHandler               [WEAK]
        EXPORT  HASH_RNG_IRQHandler           [WEAK]
        EXPORT  FPU_IRQHandler                [WEAK]

WWDG_IRQHandler
PVD_IRQHandler
TAMP_STAMP_IRQHandler
RTC_WKUP_IRQHandler
FLASH_IRQHandler
RCC_IRQHandler
EXTI0_IRQHandler
EXTI1_IRQHandler
EXTI2_IRQHandler
EXTI3_IRQHandler
EXTI4_IRQHandler
DMA1_Stream0_IRQHandler
DMA1_Stream1_IRQHandler
DMA1_Stream2_IRQHandler
DMA1_Stream3_IRQHandler
DMA1_Stream4_IRQHandler
DMA1_Stream5_IRQHandler
DMA1_Stream6_IRQHandler
ADC_IRQHandler
CAN1_TX_IRQHandler
CAN1_RX0_IRQHandler
CAN1_RX1_IRQHandler
CAN1_SCE_IRQHandler
EXTI9_5_IRQHandler
TIM1_BRK_TIM9_IRQHandler
TIM1_UP_TIM10_IRQHandler
TIM1_TRG_COM_TIM11_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
TIM4_IRQHandler
I2C1_EV_IRQHandler
I2C1_ER_IRQHandler
I2C2_EV_IRQHandler
I2C2_ER_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
USART3_IRQHandler
EXTI15_10_IRQHandler
RTC_Alarm_IRQHandler
OTG_FS_WKUP_IRQHandler
TIM8_BRK_TIM12_IRQHandler
TIM8_UP_TIM13_IRQHandler
TIM8_TRG_COM_TIM14_IRQHandler
TIM8_CC_IRQHandler
DMA1_Stream7_IRQHandler
FSMC_IRQHandler
SDIO_IRQHandler
TIM5_IRQHandler
SPI3_IRQHandler
UART4_IRQHandler
UART5_IRQHandler
TIM6_DAC_IRQHandler
TIM7_IRQHandler
DMA2_Stream0_IRQHandler
DMA2_Stream1_IRQHandler
DMA2_Stream2_IRQHandler
DMA2_Stream3_IRQHandler
DMA2_Stream4_IRQHandler
ETH_IRQHandler
ETH_WKUP_IRQHandler
CAN2_TX_IRQHandler
CAN2_RX0_IRQHandler
CAN2_RX1_IRQHandler
CAN2_SCE_IRQHandler
OTG_FS_IRQHandler
DMA2_Stream5_IRQHandler
DMA2_Stream6_IRQHandler
DMA2_Stream7_IRQHandler
USART6_IRQHandler
I2C3_EV_IRQHandler
I2C3_ER_IRQHandler
OTG_HS_EP1_OUT_IRQHandler
OTG_HS_EP1_IN_IRQHandler
OTG_HS_WKUP_IRQHandler
OTG_HS_IRQHandler
DCMI_IRQHandler
CRYP_IRQHandler
HASH_RNG_IRQHandler
FPU_IRQHandler
        CPSID   i                 ; disable all interrupts
        LDR     r0,=str_Undefined
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_Undefined
        DCB     "Undefined"
        ALIGN
        ENDP

        ALIGN               ; make sure the end of this section is aligned

;******************************************************************************
; The function expected of the C library startup code for defining the stack
; and heap memory locations.  For the C library version of the startup code,
; provide this function so that the C library initialization code can find out
; the location of the stack and heap.
;
    IF :DEF: __MICROLIB
        EXPORT  __initial_sp
        EXPORT  __stack_limit
        EXPORT  __heap_base
        EXPORT  __heap_limit
    ELSE
        IMPORT  __use_two_region_memory
        EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
        LDR     R0, =__heap_base
        LDR     R1, =__stack_limit
        LDR     R2, =__heap_limit
        LDR     R3, =__stack_base
        BX      LR
        ENDP
    ENDIF
        ALIGN               ; make sure the end of this section is aligned

    END                     ; end of module

