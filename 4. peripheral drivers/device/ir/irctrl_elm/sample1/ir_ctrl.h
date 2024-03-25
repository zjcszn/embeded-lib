/*----------------------------------------------------------------------------/
/  IR_CTRL - IR remote control module  R0.02                  (C)ChaN, 2015
/-----------------------------------------------------------------------------/
/  Common include file for IR_CTRL module and application
/----------------------------------------------------------------------------*/

#include <stdint.h>

/* Put hardware dependent include files here */
#include <avr/io.h>
#include <avr/interrupt.h>


/* Enable/Disable transmission/reception functions <1/0> */
#define IR_USE_XMIT	1		/* 1:Enable transmission feature */
#define IR_USE_RCVR	1		/* 1*Enable receiving feature */
#define IR_MAX_RCVR	160		/* Maximum number of bits to receive (255 bits max.) */
#define IR_USE_NEC	1		/* 1:Enable NEC format */
#define IR_USE_AEHA	1		/* 1:Enable AEHA format */
#define IR_USE_SONY	1		/* 1:Enable SONY format */



/* Structure of IR function work area */
typedef struct {
	uint8_t stat;	/* Communication state */
	uint8_t fmt;	/* Frame format */
	uint8_t len;	/* Number of bits received */
	uint8_t bc;		/* Bit counter */
#if IR_USE_XMIT
	const uint8_t* txptr;	/* Pointer to transmission data */
#endif
#if IR_USE_RCVR
	uint8_t rxdata[(IR_MAX_RCVR + 7) / 8];	/* Receive data buffer */
#endif
} IR_STRUCT;

/* The work area for IR_CTRL is defined in ir_ctrl.c */
extern
volatile IR_STRUCT IrCtrl;

/* IR control state (state) */
#define IR_IDLE		0	/* In idle state, ready to receive/transmit */
#define IR_RECVING	1	/* An IR frame is being received */
#define IR_RECVED	2	/* An IR frame has been received and data is valid */
#define IR_XMIT		3	/* IR transmission is initiated */
#define IR_XMITING	4	/* IR transmission is in progress */

/* Format ID (fmt) */
#define NEC		0x01	/* NEC format */
#define AEHA	0x02	/* AEHA format */
#define SONY	0x03	/* SONY format */
#define REPT	0x10	/* Repeat frame flag */

/* Prototypes */
void IR_initialize (void);
int IR_xmit (uint8_t, const void*, uint8_t);

