/*----------------------------------------------------------------------------/
/  IR_CTRL - IR remote control module  R0.02                  (C)ChaN, 2015
/-----------------------------------------------------------------------------/
/ The IR_CTRL is a generic Transmisson/Reception control module for IR remote
/ control systems. This is a free software and is opened for education,
/ research and development under license policy of following trems.
/
/  Copyright (C) 2015, ChaN, all right reserved.
/
/ * The IR_CTRL module is a free software and there is no warranty.
/ * You can use, modify and/or redistribute it for personal, non-profit or
/   commercial use without restriction under your responsibility.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
/ Aug 30,'08 R0.01  First release.
/ Jul 18,'15 R0.02  Added timing compensation, Improved RAM consumption.
/----------------------------------------------------------------------------*/

#include "ir_ctrl.h"


/*----------------------------------------------------------------------------/
/ Platform dependent definitions
/ (Macros not needed in this configuration should be left blanked)
/----------------------------------------------------------------------------*/

/* Definitions of hardware control macros */
#define	INIT_TMR()	{TCCR1B = 0b10000010;}					/* Always: Initialize Timers, Ports */

#define	INIT_XMIT()	{										/* TX: Initialize IR transmitter */ \
		OCR0B = 10; \
		TCCR0A = _BV(WGM01)|_BV(WGM00); \
		TCCR0B = _BV(WGM02)|0b010;}
#define IRTX_38K()	{OCR0A = 32; TCNT0 = 0;}				/* TX && NEC/AEHA: Set IR sub-carrier to 38kHz */
#define IRTX_40K()	{OCR0A = 30; TCNT0 = 0;}				/* TX && SONY: Set IR sub-carrier to 40kHz */
#define IRTX_ON()	TCCR0A |= _BV(COM0B1)				 	/* TX: IR sub-carrier on */
#define IRTX_OFF()	TCCR0A &= ~_BV(COM0B1)					/* TX: IR sub-carrier off */
#define IRTX_TST()	(TCCR0A & _BV(COM0B1))					/* TX: Check status of IR sub-carrier (true:on) */

#define CAPT_STA()	TCCR1B &= ~_BV(ICES1)					/* RX: Set capture on IR start (left blanked if it can capture on both edge) */
#define CAPT_STP()	TCCR1B |= _BV(ICES1)					/* RX: Set capture on IR stop (left blanked if it can capture on both edge) */
#define CAPT_TST()	(~TCCR1B & _BV(ICES1))					/* RX: Check which edge triggered the capture event (true:IR start) */
#define	CAPT_VAL()	ICR1									/* RX: Get the captured timer value */
#define CAPT_ENA()	{TIFR1 = _BV(ICF1); TIMSK1 = _BV(ICIE1);}	/* RX: Enable capture interrupt */
#define CAPT_DIS()	TIMSK1 &= ~_BV(ICIE1)					/* RX && TX: Disable capture interrupt */

#define COMP_ENA(n)	{OCR1A = TCNT1+(n); TIFR1 = _BV(OCF1A); TIMSK1 |= _BV(OCIE1A);}	/* Always: Enable compare interrupt n count after now */
#define COMP_DIS()	TIMSK1 &= ~_BV(OCIE1A)					/* Always: Disable compare interrupt */
#define COMP_NXT(n)	OCR1A += (n)							/* TX: Increase compare register by n count */

/* Counter clock rate and register width */
#define T_CLK			800									/* Always: Timer tick period [ns] */
#define	irtmr_t			uint16_t							/* Always: Integer type of timer counter */

/* Definitions of interrupt service functions */
#define	ISR_COMPARE()	ISR(TIMER1_COMPA_vect)				/* Always: Timer compare match ISR */
#define	ISR_CAPTURE()	ISR(TIMER1_CAPT_vect)				/* RX: Timer input capture ISR */
#define CALLBACK_TX()										/* TX: Callback function on end of transmission */
#define CALLBACK_RX()										/* RX: Callback function on frame arrival */


/*---------------------------------------------------------------------------*/


/* IR control timings */
#define	T_NEC	(562000/T_CLK)		/* Base time for NEC format (T=562us) */
#define	T_AEHA	(425000/T_CLK)		/* Base time for AEHA format (T=425us) */
#define T_SONY	(600000/T_CLK)		/* Base time for SONY format (T=600us) */
#define T_TRAIL	(6000000/T_CLK)		/* Trailer detection time (6ms) */

#define L_WINDOW(pw,t)	(pw >= (uint16_t)(0.75 * t) && pw <= (uint16_t)(1.33 * t))


/* Using only a format */
#if IR_USE_NEC && !IR_USE_AEHA && !IR_USE_SONY
#define USE_NEC_ONLY	1
#else
#define USE_NEC_ONLY	0
#endif
#if !IR_USE_NEC && IR_USE_AEHA && !IR_USE_SONY
#define USE_AEHA_ONLY	1
#else
#define USE_AEHA_ONLY	0
#endif
#if !IR_USE_NEC && !IR_USE_AEHA && IR_USE_SONY
#define USE_SONY_ONLY	1
#else
#define USE_SONY_ONLY	0
#endif


/* Working area for IR communication  */

volatile IR_STRUCT IrCtrl;


/*----------------------------------------------------*/
/* Timer capture ISR                                  */
/* (IR receiving interrupt on either edge of input)   */
/*----------------------------------------------------*/

#if IR_USE_RCVR
ISR_CAPTURE()
{
	static irtmr_t pw1, pw2, pth;	/* Pulse width values */
	static uint8_t bm;				/* Bit mask */
	irtmr_t pw, ct = CAPT_VAL();
	uint8_t i, f, d;


	if (!(CAPT_TST())) {
		/* Captured on stop of IR sub-carrier */
		CAPT_STA();			/* Next capture event is start of IR */
		COMP_ENA(T_TRAIL);	/* Enable copmpare interrupt (trailer timer) */
		pw1 = ct - pw1;		/* pw1: this mark length */
		pw2 = ct;			/* pw2: current ct */
		if (IR_USE_SONY && IrCtrl.fmt == SONY && IrCtrl.stat == IR_RECVING && pw1 <= (uint16_t)(T_SONY * 2.5)) {
			i = IrCtrl.bc;
			if (i >= IR_MAX_RCVR) return;
			i /= 8;
			d = IrCtrl.rxdata[i] & ~bm;
			if (pw1 >= pth) d |= bm;
			IrCtrl.rxdata[i] = d;
			if ((bm <<= 1) == 0) bm = 1;
			IrCtrl.bc++;
		}
		return;
	}

	/* Captured on start of IR sub-carrier */
	CAPT_STP();				/* Next capture event is stop of IR */
	COMP_DIS();				/* Disable compare interrupt (trailer timer) */
	pw = pw1; pw1 = ct; ct -= pw2;			/* pw: previous mark length, ct: this space length */
	if (IrCtrl.stat >= IR_RECVED) return;	/* Reject if not ready to receive */

	/* Detection of leader pattern */
	f = 0;
	if (IR_USE_NEC && L_WINDOW(pw, T_NEC * 16)) {		/* Test for NEC leader pattern */
 		pth = pw / 8;
 		if (L_WINDOW(ct, T_NEC * 8)) f = NEC;
		if (L_WINDOW(ct, T_NEC * 4)) f = NEC | REPT;
	}
	if (IR_USE_AEHA && L_WINDOW(pw, T_AEHA * 8)) {		/* Test for AEHA leader pattern */
 		pth = pw / 4;
 		if (L_WINDOW(ct, T_AEHA * 4)) f = AEHA;
		if (L_WINDOW(ct, T_AEHA * 8)) f = AEHA | REPT;
	}
	if (IR_USE_SONY && L_WINDOW(pw, T_SONY * 4)) {		/* Test for SONY leader pattern */
 		pth = pw * 3 / 8;
 		if (L_WINDOW(ct, T_SONY * 1)) f = SONY;
	}
	if (f) {	/* A leader pattern is detected */
		IrCtrl.fmt = f;
		IrCtrl.bc = 0; bm = 1;
		IrCtrl.stat = IR_RECVING;
		return;
	}

	if (IrCtrl.stat == IR_RECVING) {
		f = IrCtrl.fmt;
		if (IR_USE_SONY && f == SONY && ct <= (uint16_t)(T_SONY * 1.5))		/* Is SONY data mark? */
			return;
		i = IrCtrl.bc;
		if (i >= IR_MAX_RCVR) return;
		i /= 8;
		d = IrCtrl.rxdata[i] & ~bm;
		if (IR_USE_NEC && f == NEC && pw <= (uint16_t)(T_NEC * 1.5) && ct <= (uint16_t)(T_NEC * 4.0)) {	/* Is NEC data mark? */
			if (ct >= pth) d |= bm;
			IrCtrl.rxdata[i] = d;
			if ((bm <<= 1) == 0) bm = 1;
			IrCtrl.bc++;
			return;
		}
		if (IR_USE_AEHA && f == AEHA && pw <= (uint16_t)(T_AEHA * 1.5) && ct <= (uint16_t)(T_AEHA * 4.0)) {	/* Is AEHA data mark? */
			if (ct >= pth) d |= bm;
			IrCtrl.rxdata[i] = d;
			if ((bm <<= 1) == 0) bm = 1;
			IrCtrl.bc++;
			return;
		}
	}

	IrCtrl.stat = IR_IDLE;	/* When an invalid mark width is detected, abort and return idle state */
	IrCtrl.fmt = 0;
}
#endif /* IR_USE_RCVR */


/*----------------------------------------------------*/
/* Timer compare-match ISR                            */
/* (Transmission timing and Trailer detection)        */
/*----------------------------------------------------*/

ISR_COMPARE()
{
	uint8_t st = IrCtrl.stat;
#if IR_USE_XMIT
	uint8_t i, d, fmt = IrCtrl.fmt;
	uint16_t w = 0;

	if (st == IR_XMITING) {
		if (IRTX_TST()) {			/* Is sub-carrier on transmission? */
			IRTX_OFF();				/* End of IR sub-carrier */
			i = IrCtrl.bc;
			if (i < IrCtrl.len) {	/* Is there a bit to be sent? */
				if (USE_SONY_ONLY || (IR_USE_SONY && (fmt & SONY))) {
					w = T_SONY;
				} else {
					d = IrCtrl.txptr[i / 8] & 1 << (i % 8);
					if (USE_AEHA_ONLY || (IR_USE_AEHA && (fmt & AEHA)))
						w = d ? T_AEHA * 3 : T_AEHA;
					if (USE_NEC_ONLY || (IR_USE_NEC && (fmt & NEC)))
						w = d ? T_NEC * 3 : T_NEC;
				}
				COMP_NXT(w);		/* Set space width */
				return;
			}
		} else {
			IRTX_ON();				/* Start of IR sub-carrier */
			i = ++IrCtrl.bc;
			if (USE_SONY_ONLY || (IR_USE_SONY && (fmt & SONY))) {
				w = (IrCtrl.txptr[i / 8] & 1 << (i % 8)) ? T_SONY * 2 : T_SONY;
			} else {
				if (USE_AEHA_ONLY || (IR_USE_AEHA && (fmt & AEHA)))
					w = T_AEHA;
				if (USE_NEC_ONLY || (IR_USE_NEC && (fmt & NEC)))
					w = T_NEC;
			}
			COMP_NXT(w);			/* Set mark width */
			return;
		}
	}

	if (st == IR_XMIT) {	/* End of AGC burst? */
		IRTX_OFF();			/* Stop carrier */
#if USE_NEC_ONLY
		w = IrCtrl.len ? T_NEC * 8 : T_NEC * 4;		/* Data frame or Repeat frame? */
#elif USE_AEHA_ONLY
		w = IrCtrl.len ? T_AEHA * 4 : T_AEHA * 8;	/* Data frame or Repeat frame? */
#elif USE_SONY_ONLY
		w = T_SONY;
#else
		switch (fmt) {		/* Set next transition time */
#if IR_USE_SONY
		case SONY:
			w = T_SONY;
			break;
#endif
#if IR_USE_AEHA
		case AEHA:
			w = IrCtrl.len ? T_AEHA * 4 : T_AEHA * 8;	/* Data frame or Repeat frame? */
			break;
#endif
#if IR_USE_NEC
		case NEC:
			w = IrCtrl.len ? T_NEC * 8 : T_NEC * 4;		/* Data frame or Repeat frame? */
			break;
#endif
		}
#endif
		COMP_NXT(w);
		IrCtrl.stat = IR_XMITING;
		IrCtrl.bc = 0xFF;
		return;
	}
#endif /* IR_USE_XMIT */

	COMP_DIS();					/* Disable compare interrupt */

#if IR_USE_RCVR
	CAPT_STA();					/* Re-enable receiving */
	CAPT_ENA();
	if (st == IR_RECVING) {		/* Trailer detected */
		IrCtrl.len = IrCtrl.bc;
		IrCtrl.stat = IR_RECVED;
		CALLBACK_RX();
		return;
	}
#endif

	IrCtrl.stat = IR_IDLE;
	IrCtrl.fmt = 0;
	CALLBACK_TX();
}




/*---------------------------*/
/* Data Transmission Request */
/*---------------------------*/

#if IR_USE_XMIT
int IR_xmit (
	uint8_t fmt,		/* Frame format: NEC, AEHA or SONY */
	const void* data,	/* Pointer to the data to be sent */
	uint8_t len			/* Data length [bit]. 0 for a repeat frame */
)
{
	irtmr_t lw;


	if (IrCtrl.stat != IR_IDLE) return 0;			/* Abort when collision detected */

	switch (fmt) {
#if IR_USE_NEC
	case NEC:
		lw = T_NEC * 16;	/* AGC burst length */
		IRTX_38K();
		break;
#endif
#if IR_USE_AEHA
	case AEHA:
		lw = T_AEHA * 8;	/* AGC burst length */
		IRTX_38K();
		break;
#endif
#if IR_USE_SONY
	case SONY:
		lw = T_SONY * 4;	/* AGC burst length */
		IRTX_40K();
		break;
#endif
	default:
		return 0;
	}

	CAPT_DIS();
	COMP_DIS();
	IrCtrl.fmt = fmt;
	IrCtrl.len = (USE_SONY_ONLY || (IR_USE_SONY && (fmt == SONY))) ? len - 1 : len;
	IrCtrl.txptr = (const uint8_t*)data;

	/* Start transmission sequense */
	IrCtrl.stat = IR_XMIT;
	IRTX_ON();
	COMP_ENA(lw);

	return 1;
}
#endif /* IR_USE_XMIT */



/*---------------------------*/
/* Initialize IR functions   */
/*---------------------------*/

void IR_initialize (void)
{
	/* Initialize timer and port functions for IR communication */
	INIT_TMR();
	INIT_XMIT();

	IrCtrl.stat = IR_IDLE;
	IrCtrl.fmt = 0;

	/* Enable receiving */
	CAPT_STA();
	CAPT_ENA();
}


