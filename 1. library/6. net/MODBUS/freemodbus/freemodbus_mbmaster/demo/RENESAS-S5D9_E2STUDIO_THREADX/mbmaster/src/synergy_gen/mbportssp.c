/* generated thread source file - do not edit */
#include "mbportssp.h"

TX_THREAD mbportssp;
void mbportssp_create(void);
static void mbportssp_func(ULONG thread_input);
static uint8_t mbportssp_stack[1024] BSP_PLACE_IN_SECTION_V2(".stack.mbportssp") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_timer0) && !defined(SSP_SUPPRESS_ISR_GPT0)
SSP_VECTOR_DEFINE_CHAN(gpt_counter_overflow_isr, GPT, COUNTER_OVERFLOW, 0);
#endif
#endif
static gpt_instance_ctrl_t g_timer0_ctrl;
static const timer_on_gpt_cfg_t g_timer0_extend = { .gtioca = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW }, .gtiocb = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
		.shortest_pwm_signal = GPT_SHORTEST_LEVEL_OFF, };
static const timer_cfg_t g_timer0_cfg = { .mode = TIMER_MODE_PERIODIC, .period =
		1, .unit = TIMER_UNIT_PERIOD_MSEC, .duty_cycle = 50, .duty_cycle_unit =
		TIMER_PWM_UNIT_RAW_COUNTS, .channel = 0, .autostart = true,
		.p_callback = mbport_timer_ms, .p_context = &g_timer0, .p_extend =
				&g_timer0_extend, .irq_ipl = (12), };
/* Instance structure to use this module. */
const timer_instance_t g_timer0 = { .p_ctrl = &g_timer0_ctrl, .p_cfg =
		&g_timer0_cfg, .p_api = &g_timer_on_gpt };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer0) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI3_TXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI3_TXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer0_ctrl;
transfer_info_t g_transfer0_info = { .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
		.repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
		.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
				TRANSFER_ADDR_MODE_INCREMENTED, .size = TRANSFER_SIZE_1_BYTE,
		.mode = TRANSFER_MODE_NORMAL, .p_dest = (void *) NULL, .p_src =
				(void const *) NULL, .num_blocks = 0, .length = 0, };
const transfer_cfg_t g_transfer0_cfg = { .p_info = &g_transfer0_info,
		.activation_source = ELC_EVENT_SCI3_TXI, .auto_enable = false,
		.p_callback = NULL, .p_context = &g_transfer0, .irq_ipl =
				(BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 = { .p_ctrl = &g_transfer0_ctrl, .p_cfg =
		&g_transfer0_cfg, .p_api = &g_transfer_on_dtc };
#if SCI_UART_CFG_RX_ENABLE
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_uart0) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_rxi_isr, SCI, RXI, 3);
#endif
#endif
#endif
#if SCI_UART_CFG_TX_ENABLE
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_uart0) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_txi_isr, SCI, TXI, 3);
#endif
#endif
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_uart0) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_tei_isr, SCI, TEI, 3);
#endif
#endif
#endif
#if SCI_UART_CFG_RX_ENABLE
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_uart0) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_eri_isr, SCI, ERI, 3);
#endif
#endif
#endif
sci_uart_instance_ctrl_t g_uart0_ctrl;

/** UART extended configuration for UARTonSCI HAL driver */
const uart_on_sci_cfg_t g_uart0_cfg_extend =
		{ .clk_src = SCI_CLK_SRC_INT, .baudclk_out = false, .rx_edge_start =
				true, .noisecancel_en = false, .p_extpin_ctrl = NULL,
				.bitrate_modulation = true, .rx_fifo_trigger =
						SCI_UART_RX_FIFO_TRIGGER_1, .baud_rate_error_x_1000 =
						(uint32_t)(2.0 * 1000), .uart_comm_mode =
						UART_MODE_RS232, .uart_rs485_mode = UART_RS485_HD,
				.rs485_de_pin = IOPORT_PORT_09_PIN_14, };

/** UART interface configuration */
const uart_cfg_t g_uart0_cfg = { .channel = 3, .baud_rate = 9600, .data_bits =
		UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits =
		UART_STOP_BITS_1, .ctsrts_en = false, .p_callback = user_uart_callback,
		.p_context = &g_uart0, .p_extend = &g_uart0_cfg_extend,
#define SYNERGY_NOT_DEFINED (1)                        
#if (SYNERGY_NOT_DEFINED == g_transfer0)
		.p_transfer_tx = NULL,
#else
		.p_transfer_tx = &g_transfer0,
#endif            
#if (SYNERGY_NOT_DEFINED == SYNERGY_NOT_DEFINED)
		.p_transfer_rx = NULL,
#else
		.p_transfer_rx = &SYNERGY_NOT_DEFINED,
#endif   
#undef SYNERGY_NOT_DEFINED            
		.rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl =
				(BSP_IRQ_DISABLED), };

/* Instance structure to use this module. */
const uart_instance_t g_uart0 = { .p_ctrl = &g_uart0_ctrl,
		.p_cfg = &g_uart0_cfg, .p_api = &g_uart_on_sci };
TX_MUTEX g_mbport_mutex0;
TX_SEMAPHORE g_mbport_sem;
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void mbportssp_create(void) {
	/* Increment count so we will know the number of ISDE created threads. */
	g_ssp_common_thread_count++;

	/* Initialize each kernel object. */
	UINT err_g_mbport_mutex0;
	err_g_mbport_mutex0 = tx_mutex_create(&g_mbport_mutex0,
			(CHAR *) "MODBUS port locking", TX_INHERIT);
	if (TX_SUCCESS != err_g_mbport_mutex0) {
		tx_startup_err_callback(&g_mbport_mutex0, 0);
	}
	UINT err_g_mbport_sem;
	err_g_mbport_sem = tx_semaphore_create(&g_mbport_sem,
			(CHAR *) "MODBUS port timer/serial semaphore", 0);
	if (TX_SUCCESS != err_g_mbport_sem) {
		tx_startup_err_callback(&g_mbport_sem, 0);
	}

	UINT err;
	err = tx_thread_create(&mbportssp, (CHAR *) "MODBUS port", mbportssp_func,
			(ULONG) NULL, &mbportssp_stack, 1024, 1, 1, 1, TX_DONT_START);
	if (TX_SUCCESS != err) {
		tx_startup_err_callback(&mbportssp, 0);
	}
}

static void mbportssp_func(ULONG thread_input) {
	/* Not currently using thread_input. */
	SSP_PARAMETER_NOT_USED(thread_input);

	/* Initialize common components */
	tx_startup_common_init();

	/* Initialize each module instance. */

	/* Enter user code for this thread. */
	mbportssp_entry();
}
