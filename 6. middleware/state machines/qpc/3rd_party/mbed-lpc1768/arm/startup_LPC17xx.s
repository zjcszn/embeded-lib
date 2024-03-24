;*****************************************************************************
; @file     startup_lpc1768.s for ARM-KEIL ARM assembler
; @brief    CMSIS Cortex-M4F Core Device Startup File for lpc1768
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
    ; Initial Vector Table before relocation
        DCD     __initial_sp                ; Top of Stack
        DCD     Reset_Handler               ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     MemManage_Handler           ; MPU fault handler
        DCD     BusFault_Handler            ; Bus fault handler
        DCD     UsageFault_Handler          ; Usage fault handler
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     SVC_Handler                 ; SVCall handler
        DCD     DebugMon_Handler            ; Debug Monitor handler
        DCD     Default_Handler             ; Reserved
        DCD     PendSV_Handler              ; PendSV handler
        DCD     SysTick_Handler             ; SysTick handler
        ALIGN  256  ; Extend the initial Vector Table to the 256B boundary

    ; Relocated Vector Table beyond the 256B region around address 0.
    ; That region is used for NULL-pointer protection by the MPU.
__relocated_vector_table
        DCD     __initial_sp                ; Top of Stack
        DCD     Reset_Handler               ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     MemManage_Handler           ; MPU fault handler
        DCD     BusFault_Handler            ; Bus fault handler
        DCD     UsageFault_Handler          ; Usage fault handler
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     SVC_Handler                 ; SVCall handler
        DCD     DebugMon_Handler            ; Debug Monitor handler
        DCD     Default_Handler             ; Reserved
        DCD     PendSV_Handler              ; PendSV handler
        DCD     SysTick_Handler             ; SysTick handler

        ; IRQ handlers...
        DCD     WDT_IRQHandler              ; WDT
        DCD     TIMER0_IRQHandler           ; TIMER0
        DCD     TIMER1_IRQHandler           ; TIMER1
        DCD     TIMER2_IRQHandler           ; TIMER2
        DCD     TIMER3_IRQHandler           ; TIMER3
        DCD     UART0_IRQHandler            ; UART0
        DCD     UART1_IRQHandler            ; UART1
        DCD     UART2_IRQHandler            ; UART2
        DCD     UART3_IRQHandler            ; UART3
        DCD     PWM1_IRQHandler             ; PWM1
        DCD     I2C0_IRQHandler             ; I2C0
        DCD     I2C1_IRQHandler             ; I2C1
        DCD     I2C2_IRQHandler             ; I2C2
        DCD     SPI_IRQHandler              ; SPI
        DCD     SSP0_IRQHandler             ; SSP0
        DCD     SSP1_IRQHandler             ; SSP1
        DCD     PLL0_IRQHandler             ; PLL0 (Main PLL)
        DCD     RTC_IRQHandler              ; RTC
        DCD     EINT0_IRQHandler            ; EINT0
        DCD     EINT1_IRQHandler            ; EINT1
        DCD     EINT2_IRQHandler            ; EINT2
        DCD     EINT3_IRQHandler            ; EINT3
        DCD     ADC_IRQHandler              ; ADC
        DCD     BOD_IRQHandler              ; BOD
        DCD     USB_IRQHandler              ; USB
        DCD     CAN_IRQHandler              ; CAN
        DCD     DMA_IRQHandler              ; GP DMA
        DCD     I2S_IRQHandler              ; I2S
        DCD     ENET_IRQHandler             ; Ethernet
        DCD     RIT_IRQHandler              ; RITINT
        DCD     MCPWM_IRQHandler            ; Motor Control PWM
        DCD     QEI_IRQHandler              ; Quadrature Encoder
        DCD     PLL1_IRQHandler             ; PLL1 (USB PLL)
        DCD     USBActivity_IRQHandler      ; USB Activity interrupt to wakeup
        DCD     CANActivity_IRQHandler      ; CAN Activity interrupt to wakeup
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

        ; relocate the Vector Table
        LDR     r0,=0xE000ED08  ; System Control Block/Vector Table Offset Reg
        LDR     r1,=__relocated_vector_table
        STR     r1,[r0]         ; SCB->VTOR := __relocated_vector_table

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
        EXPORT  WDT_IRQHandler          [WEAK]
        EXPORT  TIMER0_IRQHandler       [WEAK]
        EXPORT  TIMER1_IRQHandler       [WEAK]
        EXPORT  TIMER2_IRQHandler       [WEAK]
        EXPORT  TIMER3_IRQHandler       [WEAK]
        EXPORT  UART0_IRQHandler        [WEAK]
        EXPORT  UART1_IRQHandler        [WEAK]
        EXPORT  UART2_IRQHandler        [WEAK]
        EXPORT  UART3_IRQHandler        [WEAK]
        EXPORT  PWM1_IRQHandler         [WEAK]
        EXPORT  I2C0_IRQHandler         [WEAK]
        EXPORT  I2C1_IRQHandler         [WEAK]
        EXPORT  I2C2_IRQHandler         [WEAK]
        EXPORT  SPI_IRQHandler          [WEAK]
        EXPORT  SSP0_IRQHandler         [WEAK]
        EXPORT  SSP1_IRQHandler         [WEAK]
        EXPORT  PLL0_IRQHandler         [WEAK]
        EXPORT  RTC_IRQHandler          [WEAK]
        EXPORT  EINT0_IRQHandler        [WEAK]
        EXPORT  EINT1_IRQHandler        [WEAK]
        EXPORT  EINT2_IRQHandler        [WEAK]
        EXPORT  EINT3_IRQHandler        [WEAK]
        EXPORT  ADC_IRQHandler          [WEAK]
        EXPORT  BOD_IRQHandler          [WEAK]
        EXPORT  USB_IRQHandler          [WEAK]
        EXPORT  CAN_IRQHandler          [WEAK]
        EXPORT  DMA_IRQHandler          [WEAK]
        EXPORT  I2S_IRQHandler          [WEAK]
        EXPORT  ENET_IRQHandler         [WEAK]
        EXPORT  RIT_IRQHandler          [WEAK]
        EXPORT  MCPWM_IRQHandler        [WEAK]
        EXPORT  QEI_IRQHandler          [WEAK]
        EXPORT  PLL1_IRQHandler         [WEAK]
        EXPORT  USBActivity_IRQHandler  [WEAK]
        EXPORT  CANActivity_IRQHandler  [WEAK]

WDT_IRQHandler
TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
TIMER3_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
UART2_IRQHandler
UART3_IRQHandler
PWM1_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
I2C2_IRQHandler
SPI_IRQHandler
SSP0_IRQHandler
SSP1_IRQHandler
PLL0_IRQHandler
RTC_IRQHandler
EINT0_IRQHandler
EINT1_IRQHandler
EINT2_IRQHandler
EINT3_IRQHandler
ADC_IRQHandler
BOD_IRQHandler
USB_IRQHandler
CAN_IRQHandler
DMA_IRQHandler
I2S_IRQHandler
ENET_IRQHandler
RIT_IRQHandler
MCPWM_IRQHandler
QEI_IRQHandler
PLL1_IRQHandler
USBActivity_IRQHandler
CANActivity_IRQHandler
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

