;/***************************************************************************/
; @file     startup_LPC17xx.s for IAR ARM assembler
; @brief    CMSIS Cortex-M4F Core Device Startup File for LPC17xx
; @version  CMSIS 5.9.0
; @date     1 Feb 2023
;
; Modified by Quantum Leaps:
; - Added relocating of the Vector Table to free up the 256B region at 0x0
;   for NULL-pointer protection by the MPU.
; - Modified all exception handlers to branch to assert_failed()
;   instead of locking up the CPU inside an endless loop.
; *
; * @description
; * Created from the CMSIS template for the specified device
; * Quantum Leaps, www.state-machine.com
; *

        MODULE  ?cstartup

        ; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(8)

        PUBLIC  __vector_table
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

;******************************************************************************
; The vector table
;
        DATA
__vector_table
    ; Initial Vector Table before relocation
        DCD     sfe(CSTACK)
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
        ALIGNROM 8   ; Extend the initial Vector Table to the 2^8==256B

    ; Relocated Vector Table beyond the 256B region around address 0.
    ; That region is used for NULL-pointer protection by the MPU.
__relocated_vector_table
        DCD     sfe(CSTACK)
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

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;******************************************************************************
; This is the code for exception handlers.
;
        SECTION .text:CODE:REORDER:NOROOT(2)

;******************************************************************************
; This is the code that gets called when the CPU first starts execution
; following a reset event.
;
        PUBWEAK Reset_Handler
        EXTERN  SystemInit
        EXTERN  __iar_program_start
        EXTERN  assert_failed

Reset_Handler
        ; relocate the Vector Table
        LDR     r0,=0xE000ED08  ; System Control Block/Vector Table Offset Reg
        LDR     r1,=__relocated_vector_table
        STR     r1,[r0]         ; SCB->VTOR := __relocated_vector_table

        LDR     r0,=SystemInit  ; CMSIS system initialization
        BLX     r0

        ; pre-fill the CSTACK with 0xDEADBEEF...................
        LDR     r0,=0xDEADBEEF
        MOV     r1,r0
        LDR     r2,=sfb(CSTACK)
        LDR     r3,=sfe(CSTACK)
Reset_stackInit_fill:
        STMIA   r2!,{r0,r1}
        CMP     r2,r3
        BLT.N   Reset_stackInit_fill

        LDR     r0,=__iar_program_start ; IAR startup code
        BLX     r0

        ; __iar_program_start calls the main() function, which should not return,
        ; but just in case jump to assert_failed() if main returns.
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_EXIT
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_EXIT
        DCB     "EXIT"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK NMI_Handler
NMI_Handler
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_NMI
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_NMI
        DCB     "NMI"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK HardFault_Handler
HardFault_Handler
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_HardFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_HardFault
        DCB     "HardFault"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK MemManage_Handler
MemManage_Handler
        LDR     r0,=str_MemManage
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_MemManage
        DCB     "MemManage"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK BusFault_Handler
BusFault_Handler
        LDR     r0,=str_BusFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_BusFault
        DCB     "BusFault"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK UsageFault_Handler
UsageFault_Handler
        LDR     r0,=str_UsageFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_UsageFault
        DCB     "UsageFault"
        ALIGNROM 2

;******************************************************************************
;
; Weak non-fault handlers...
;

;******************************************************************************
        PUBWEAK SVC_Handler
SVC_Handler
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_SVC
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SVC
        DCB     "SVC"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK DebugMon_Handler
DebugMon_Handler
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_DebugMon
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_DebugMon
        DCB     "DebugMon"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK PendSV_Handler
PendSV_Handler
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_PendSV
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_PendSV
        DCB     "PendSV"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK SysTick_Handler
SysTick_Handler
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_SysTick
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SysTick
        DCB     "SysTick"
        ALIGNROM 2

;******************************************************************************
; Weak IRQ handlers...
;
        PUBWEAK  Default_Handler
        PUBWEAK  WDT_IRQHandler
        PUBWEAK  TIMER0_IRQHandler
        PUBWEAK  TIMER1_IRQHandler
        PUBWEAK  TIMER2_IRQHandler
        PUBWEAK  TIMER3_IRQHandler
        PUBWEAK  UART0_IRQHandler
        PUBWEAK  UART1_IRQHandler
        PUBWEAK  UART2_IRQHandler
        PUBWEAK  UART3_IRQHandler
        PUBWEAK  PWM1_IRQHandler
        PUBWEAK  I2C0_IRQHandler
        PUBWEAK  I2C1_IRQHandler
        PUBWEAK  I2C2_IRQHandler
        PUBWEAK  SPI_IRQHandler
        PUBWEAK  SSP0_IRQHandler
        PUBWEAK  SSP1_IRQHandler
        PUBWEAK  PLL0_IRQHandler
        PUBWEAK  RTC_IRQHandler
        PUBWEAK  EINT0_IRQHandler
        PUBWEAK  EINT1_IRQHandler
        PUBWEAK  EINT2_IRQHandler
        PUBWEAK  EINT3_IRQHandler
        PUBWEAK  ADC_IRQHandler
        PUBWEAK  BOD_IRQHandler
        PUBWEAK  USB_IRQHandler
        PUBWEAK  CAN_IRQHandler
        PUBWEAK  DMA_IRQHandler
        PUBWEAK  I2S_IRQHandler
        PUBWEAK  ENET_IRQHandler
        PUBWEAK  RIT_IRQHandler
        PUBWEAK  MCPWM_IRQHandler
        PUBWEAK  QEI_IRQHandler
        PUBWEAK  PLL1_IRQHandler
        PUBWEAK  USBActivity_IRQHandler
        PUBWEAK  CANActivity_IRQHandler

Default_Handler
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
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_Undefined
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_Undefined
        DCB     "Undefined"
        ALIGNROM 2

        END                      ; end of module

