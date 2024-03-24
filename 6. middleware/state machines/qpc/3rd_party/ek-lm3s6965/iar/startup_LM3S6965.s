;/***************************************************************************/
; @file     startup_LM3S6965.s for IAR ARM assembler
; @brief    CMSIS Cortex-M4F Core Device Startup File for LM3S6965
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
        DCD     GPIOPortA_IRQHandler        ; GPIO Port A
        DCD     GPIOPortB_IRQHandler        ; GPIO Port B
        DCD     GPIOPortC_IRQHandler        ; GPIO Port C
        DCD     GPIOPortD_IRQHandler        ; GPIO Port D
        DCD     GPIOPortE_IRQHandler        ; GPIO Port E
        DCD     UART0_IRQHandler            ; UART0 Rx and Tx
        DCD     UART1_IRQHandler            ; UART1 Rx and Tx
        DCD     SSI0_IRQHandler             ; SSI0 Rx and Tx
        DCD     I2C0_IRQHandler             ; I2C0 Master and Slave
        DCD     PWMFault_IRQHandler         ; PWM Fault
        DCD     PWMGen0_IRQHandler          ; PWM Generator 0
        DCD     PWMGen1_IRQHandler          ; PWM Generator 1
        DCD     PWMGen2_IRQHandler          ; PWM Generator 2
        DCD     QEI0_IRQHandler             ; Quadrature Encoder 0
        DCD     ADCSeq0_IRQHandler          ; ADC Sequence 0
        DCD     ADCSeq1_IRQHandler          ; ADC Sequence 1
        DCD     ADCSeq2_IRQHandler          ; ADC Sequence 2
        DCD     ADCSeq3_IRQHandler          ; ADC Sequence 3
        DCD     Watchdog_IRQHandler         ; Watchdog timer
        DCD     Timer0A_IRQHandler          ; Timer 0 subtimer A
        DCD     Timer0B_IRQHandler          ; Timer 0 subtimer B
        DCD     Timer1A_IRQHandler          ; Timer 1 subtimer A
        DCD     Timer1B_IRQHandler          ; Timer 1 subtimer B
        DCD     Timer2A_IRQHandler          ; Timer 2 subtimer A
        DCD     Timer2B_IRQHandler          ; Timer 2 subtimer B
        DCD     Comp0_IRQHandler            ; Analog Comparator 0
        DCD     Comp1_IRQHandler            ; Analog Comparator 1
        DCD     Comp2_IRQHandler            ; Analog Comparator 2
        DCD     SysCtrl_IRQHandler          ; System Control (PLL, OSC, BO)
        DCD     FlashCtrl_IRQHandler        ; FLASH Control
        DCD     GPIOPortF_IRQHandler        ; GPIO Port F
        DCD     GPIOPortG_IRQHandler        ; GPIO Port G
        DCD     GPIOPortH_IRQHandler        ; GPIO Port H
        DCD     UART2_IRQHandler            ; UART2 Rx and Tx
        DCD     SSI1_IRQHandler             ; SSI1 Rx and Tx
        DCD     Timer3A_IRQHandler          ; Timer 3 subtimer A
        DCD     Timer3B_IRQHandler          ; Timer 3 subtimer B
        DCD     I2C1_IRQHandler             ; I2C1 Master and Slave
        DCD     QEI1_IRQHandler             ; Quadrature Encoder 1
        DCD     CAN0_IRQHandler             ; CAN0
        DCD     CAN1_IRQHandler             ; CAN1
        DCD     CAN2_IRQHandler             ; CAN2
        DCD     Ethernet_IRQHandler         ; Ethernet
        DCD     Hibernate_IRQHandler        ; Hibernate

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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        PUBWEAK GPIOPortA_IRQHandler
        PUBWEAK GPIOPortB_IRQHandler
        PUBWEAK GPIOPortC_IRQHandler
        PUBWEAK GPIOPortD_IRQHandler
        PUBWEAK GPIOPortE_IRQHandler
        PUBWEAK UART0_IRQHandler
        PUBWEAK UART1_IRQHandler
        PUBWEAK SSI0_IRQHandler
        PUBWEAK I2C0_IRQHandler
        PUBWEAK PWMFault_IRQHandler
        PUBWEAK PWMGen0_IRQHandler
        PUBWEAK PWMGen1_IRQHandler
        PUBWEAK PWMGen2_IRQHandler
        PUBWEAK QEI0_IRQHandler
        PUBWEAK ADCSeq0_IRQHandler
        PUBWEAK ADCSeq1_IRQHandler
        PUBWEAK ADCSeq2_IRQHandler
        PUBWEAK ADCSeq3_IRQHandler
        PUBWEAK Watchdog_IRQHandler
        PUBWEAK Timer0A_IRQHandler
        PUBWEAK Timer0B_IRQHandler
        PUBWEAK Timer1A_IRQHandler
        PUBWEAK Timer1B_IRQHandler
        PUBWEAK Timer2A_IRQHandler
        PUBWEAK Timer2B_IRQHandler
        PUBWEAK Comp0_IRQHandler
        PUBWEAK Comp1_IRQHandler
        PUBWEAK Comp2_IRQHandler
        PUBWEAK SysCtrl_IRQHandler
        PUBWEAK FlashCtrl_IRQHandler
        PUBWEAK GPIOPortF_IRQHandler
        PUBWEAK GPIOPortG_IRQHandler
        PUBWEAK GPIOPortH_IRQHandler
        PUBWEAK UART2_IRQHandler
        PUBWEAK SSI1_IRQHandler
        PUBWEAK Timer3A_IRQHandler
        PUBWEAK Timer3B_IRQHandler
        PUBWEAK I2C1_IRQHandler
        PUBWEAK QEI1_IRQHandler
        PUBWEAK CAN0_IRQHandler
        PUBWEAK CAN1_IRQHandler
        PUBWEAK CAN2_IRQHandler
        PUBWEAK Ethernet_IRQHandler
        PUBWEAK Hibernate_IRQHandler

Default_Handler
GPIOPortA_IRQHandler
GPIOPortB_IRQHandler
GPIOPortC_IRQHandler
GPIOPortD_IRQHandler
GPIOPortE_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
SSI0_IRQHandler
I2C0_IRQHandler
PWMFault_IRQHandler
PWMGen0_IRQHandler
PWMGen1_IRQHandler
PWMGen2_IRQHandler
QEI0_IRQHandler
ADCSeq0_IRQHandler
ADCSeq1_IRQHandler
ADCSeq2_IRQHandler
ADCSeq3_IRQHandler
Watchdog_IRQHandler
Timer0A_IRQHandler
Timer0B_IRQHandler
Timer1A_IRQHandler
Timer1B_IRQHandler
Timer2A_IRQHandler
Timer2B_IRQHandler
Comp0_IRQHandler
Comp1_IRQHandler
Comp2_IRQHandler
SysCtrl_IRQHandler
FlashCtrl_IRQHandler
GPIOPortF_IRQHandler
GPIOPortG_IRQHandler
GPIOPortH_IRQHandler
UART2_IRQHandler
SSI1_IRQHandler
Timer3A_IRQHandler
Timer3B_IRQHandler
I2C1_IRQHandler
QEI1_IRQHandler
CAN0_IRQHandler
CAN1_IRQHandler
CAN2_IRQHandler
Ethernet_IRQHandler
Hibernate_IRQHandler
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

