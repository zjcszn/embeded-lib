/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
**********************************************************************/
#ifndef _FM175XX_REG_H
#define _FM175XX_REG_H

#define  JREG_PAGE0           0x00    //Page register in page 0
#define  JREG_COMMAND         0x01    //Contains Command bits, PowerDown bit and bit to switch receiver off.
#define  JREG_COMMIEN         0x02    //Contains Communication interrupt enable bits andbit for Interrupt inversion.
#define  JREG_DIVIEN          0x03    //Contains RfOn, RfOff, CRC and Mode Interrupt enable and bit to switch Interrupt pin to PushPull mode.
#define  JREG_COMMIRQ         0x04    //Contains Communication interrupt request bits.
#define  JREG_DIVIRQ          0x05    //Contains RfOn, RfOff, CRC and Mode Interrupt request.
#define  JREG_ERROR        0x06    //Contains Protocol, Parity, CRC, Collision, Buffer overflow, Temperature and RF error flags.
#define  JREG_STATUS1         0x07    //Contains status information about Lo- and HiAlert, RF-field on, Timer, Interrupt request and CRC status.
#define  JREG_STATUS2         0x08    //Contains information about internal states (Modemstate),Mifare states and possibility to switch Temperature sensor off.
#define  JREG_FIFODATA        0x09    //Gives access to FIFO. Writing to register increments theFIFO level (register 0x0A), reading decrements it.
#define  JREG_FIFOLEVEL       0x0A    //Contains the actual level of the FIFO.     
#define  JREG_WATERLEVEL      0x0B    //Contains the Waterlevel value for the FIFO 
#define  JREG_CONTROL         0x0C    //Contains information about last received bits, Initiator mode bit, bit to copy NFCID to FIFO and to Start and stopthe Timer unit.
#define  JREG_BITFRAMING      0x0D    //Contains information of last bits to send, to align received bits in FIFO and activate sending in Transceive*/
#define  JREG_COLL            0x0E    //Contains all necessary bits for Collission handling 
#define  JREG_RFU0F           0x0F    //Currently not used.                                 
                               
                               
#define  JREG_PAGE1           0x10    //Page register in page 1
#define  JREG_MODE            0x11    //Contains bits for auto wait on Rf, to detect SYNC byte in NFC mode and MSB first for CRC calculation
#define  JREG_TXMODE          0x12    //Contains Transmit Framing, Speed, CRC enable, bit for inverse mode and TXMix bit.                            
#define  JREG_RXMODE          0x13    //Contains Transmit Framing, Speed, CRC enable, bit for multiple receive and to filter errors.                 
#define  JREG_TXCONTROL       0x14    //Contains bits to activate and configure Tx1 and Tx2 and bit to activate 100% modulation.                      
#define  JREG_TXAUTO          0x15    //Contains bits to automatically switch on/off the Rf and to do the collission avoidance and the initial rf-on.
#define  JREG_TXSEL           0x16    //Contains SigoutSel, DriverSel and LoadModSel bits.
#define  JREG_RXSEL           0x17    //Contains UartSel and RxWait bits.                 
#define  JREG_RXTRESHOLD      0x18    //Contains MinLevel and CollLevel for detection.    
#define  JREG_DEMOD           0x19    //Contains bits for time constants, hysteresis and IQ demodulator settings. 
#define  JREG_FELICANFC       0x1A    //Contains bits for minimum FeliCa length received and for FeliCa syncronisation length.
#define  JREG_FELICANFC2      0x1B    //Contains bits for maximum FeliCa length received.      
#define  JREG_MIFARE          0x1C    //Contains Miller settings, TxWait settings and MIFARE halted mode bit.
#define  JREG_MANUALRCV       0x1D    //Currently not used.                          
#define  JREG_RFU1E           0x1E    //Currently not used.                          
#define  JREG_SERIALSPEED     0x1F    //Contains speed settings for serila interface.
                               
                               
#define  JREG_PAGE2           0x20    //Page register in page 2 
#define  JREG_CRCRESULT1      0x21    //Contains MSByte of CRC Result.                
#define  JREG_CRCRESULT2      0x22    //Contains LSByte of CRC Result.                
#define  JREG_GSNLOADMOD      0x23    //Contains the conductance and the modulation settings for the N-MOS transistor only for load modulation (See difference to JREG_GSN!). 
#define  JREG_MODWIDTH        0x24    //Contains modulation width setting.                    
#define  JREG_TXBITPHASE      0x25    //Contains TxBitphase settings and receive clock change.
#define  JREG_RFCFG           0x26    //Contains sensitivity of Rf Level detector, the receiver gain factor and the RfLevelAmp.
#define  JREG_GSN             0x27    //Contains the conductance and the modulation settings for the N-MOS transistor during active modulation (no load modulation setting!).
#define  JREG_CWGSP           0x28    //Contains the conductance for the P-Mos transistor.    
#define  JREG_MODGSP          0x29    //Contains the modulation index for the PMos transistor.
#define  JREG_TMODE           0x2A    //Contains all settings for the timer and the highest 4 bits of the prescaler.
#define  JREG_TPRESCALER      0x2B    //Contais the lowest byte of the prescaler.   
#define  JREG_TRELOADHI       0x2C    //Contains the high byte of the reload value. 
#define  JREG_TRELOADLO       0x2D    //Contains the low byte of the reload value.  
#define  JREG_TCOUNTERVALHI   0x2E    //Contains the high byte of the counter value.
#define  JREG_TCOUNTERVALLO   0x2F    //Contains the low byte of the counter value. 
                               
                               
#define  JREG_PAGE3           0x30    //Page register in page 3
#define  JREG_TESTSEL1        0x31    //Test register                              
#define  JREG_TESTSEL2        0x32    //Test register                              
#define  JREG_TESTPINEN       0x33    //Test register                              
#define  JREG_TESTPINVALUE    0x34    //Test register                              
#define  JREG_TESTBUS         0x35    //Test register                              
#define  JREG_AUTOTEST        0x36    //Test register                              
#define  JREG_VERSION         0x37    //Contains the product number and the version .
#define  JREG_ANALOGTEST      0x38    //Test register                              
#define  JREG_TESTDAC1        0x39    //Test register                              
#define  JREG_TESTDAC2        0x3A    //Test register                              
#define  JREG_TESTADC         0x3B    //Test register                              
#define  JREG_ANALOGUETEST1   0x3C    //Test register                              
#define  JREG_ANALOGUETEST0   0x3D    //Test register                              
#define  JREG_ANALOGUETPD_A   0x3E    //Test register                              
#define  JREG_ANALOGUETPD_B   0x3F    //Test register                              


#define  CMD_MASK          0xF0

#define     CMD_IDLE          0x00
#define     CMD_CONFIGURE     0x01 
#define     CMD_GEN_RAND_ID   0x02
#define     CMD_CALC_CRC      0x03 
#define     CMD_TRANSMIT      0x04 
#define     CMD_NOCMDCHANGE   0x07 
#define     CMD_RECEIVE       0x08 
#define     CMD_TRANSCEIVE    0x0C 
#define     CMD_AUTOCOLL      0x0D
#define     CMD_AUTHENT       0x0E 
#define     CMD_SOFT_RESET    0x0F 


/* /////////////////////////////////////////////////////////////////////////////
 * Bit Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*name FM175XX Bit definitions of Page 0 */
/* Command Register							(01) */
#define     JBIT_RCVOFF             0x20   /*Switches the receiver on/off. */
#define     JBIT_POWERDOWN          0x10   /*Switches FM175XX to Power Down mode. */

/* CommIEn Register							(02) */
#define     JBIT_IRQINV             0x80   /*Inverts the output of IRQ Pin. */

/* DivIEn Register							(03) */
#define     JBIT_IRQPUSHPULL        0x80   /*Sets the IRQ pin to Push Pull mode. */

/* CommIEn and CommIrq Register         (02, 04) */
#define     JBIT_TXI                0x40   /*Bit position for Transmit Interrupt Enable/Request.*/
#define     JBIT_RXI                0x20   /*Bit position for Receive Interrupt Enable/Request. */
#define     JBIT_IDLEI              0x10   /*Bit position for Idle Interrupt Enable/Request.    */
#define     JBIT_HIALERTI           0x08   /*Bit position for HiAlert Interrupt Enable/Request. */
#define     JBIT_LOALERTI           0x04   /*Bit position for LoAlert Interrupt Enable/Request. */
#define     JBIT_ERRI               0x02   /*Bit position for Error Interrupt Enable/Request.   */
#define     JBIT_TIMERI             0x01   /*Bit position for Timer Interrupt Enable/Request.   */

/* DivIEn and DivIrq Register           (03, 05) */
#define     JBIT_SIGINACT           0x10   /*Bit position for SiginAct Interrupt Enable/Request. */
#define     JBIT_MODEI              0x08   /*Bit position for Mode Interrupt Enable/Request. */
#define     JBIT_CRCI               0x04   /*Bit position for CRC Interrupt Enable/Request. */
#define     JBIT_RFONI              0x02   /*Bit position for RF On Interrupt Enable/Request. */
#define     JBIT_RFOFFI             0x01   /*Bit position for RF Off Interrupt Enable/Request. */

/* CommIrq and DivIrq Register          (04, 05) */
#define     JBIT_SET                0x80   /*Bit position to set/clear dedicated IRQ bits. */

/* Error Register 							(06) */
#define     JBIT_WRERR              0x40   /*Bit position for Write Access Error. */
#define     JBIT_TEMPERR            0x40   /*Bit position for Temerature Error. */
#define     JBIT_RFERR              0x20   /*Bit position for RF Error. */
#define     JBIT_BUFFEROVFL         0x10   /*Bit position for Buffer Overflow Error. */
#define     JBIT_COLLERR            0x08   /*Bit position for Collision Error. */
#define     JBIT_CRCERR             0x04   /*Bit position for CRC Error. */
#define     JBIT_PARITYERR          0x02   /*Bit position for Parity Error. */
#define     JBIT_PROTERR            0x01   /*Bit position for Protocol Error. */

/* Status 1 Register 						(07) */
#define     JBIT_CRCOK              0x40   /*Bit position for status CRC OK. */
#define     JBIT_CRCREADY           0x20   /*Bit position for status CRC Ready. */
#define     JBIT_IRQ                0x10   /*Bit position for status IRQ is active. */
#define     JBIT_TRUNNUNG           0x08   /*Bit position for status Timer is running. */
#define     JBIT_RFON               0x04   /*Bit position for status RF is on/off. */
#define     JBIT_HIALERT            0x02   /*Bit position for status HiAlert. */
#define     JBIT_LOALERT            0x01   /*Bit position for status LoAlert. */

/* Status 2 Register				    	(08) */
#define     JBIT_TEMPSENSOFF        0x80   /*Bit position to switch Temperture sensors on/off. */
#define     JBIT_I2CFORCEHS         0x40   /*Bit position to forece High speed mode for I2C Interface. */
#define     JBIT_MFSELECTED         0x10   /*Bit position for card status Mifare selected. */
#define     JBIT_CRYPTO1ON          0x08   /*Bit position for reader status Crypto is on. */

/* FIFOLevel Register				    	(0A) */
#define     JBIT_FLUSHFIFO          0x80   /*Clears FIFO buffer if set to 1 */

/* Control Register					    	(0C) */
#define     JBIT_TSTOPNOW           0x80   /*Stops timer if set to 1. */
#define     JBIT_TSTARTNOW          0x40   /*Starts timer if set to 1. */
#define     JBIT_WRNFCIDTOFIFO      0x20   /*Copies internal stored NFCID3 to actual position of FIFO. */
#define     JBIT_INITIATOR          0x10   /*Sets Initiator mode. */

/* BitFraming Register					    (0D) */
#define     JBIT_STARTSEND          0x80   /*Starts transmission in transceive command if set to 1. */

/* BitFraming Register					    (0E) */
#define     JBIT_VALUESAFTERCOLL    0x80   /*Activates mode to keep data after collision. */

/*name FM175XX Bit definitions of Page 1
 *  Below there are useful bit definition of the FM175XX register set of Page 1.*/
/* Mode Register							(11) */
#define     JBIT_MSBFIRST           0x80   /*Sets CRC coprocessor with MSB first. */
#define     JBIT_DETECTSYNC         0x40   /*Activate automatic sync detection for NFC 106kbps. */
#define     JBIT_TXWAITRF           0x20   /*Tx waits until Rf is enabled until transmit is startet, else 
                                                transmit is started immideately. */
#define     JBIT_RXWAITRF           0x10   /*Rx waits until Rf is enabled until receive is startet, else 
                                                receive is started immideately. */
#define     JBIT_POLSIGIN           0x08   /*Inverts polarity of SiginActIrq, if bit is set to 1 IRQ occures
                                                when Sigin line is 0. */
#define     JBIT_MODEDETOFF         0x04   /*Deactivates the ModeDetector during AutoAnticoll command. The settings
                                                of the register are valid only. */

/* TxMode Register							(12) */
#define     JBIT_TXCRCEN            0x80  /*enables the CRC generation during data transmissio. */
#define     JBIT_INVMOD             0x08   /*Activates inverted transmission mode. */
#define     JBIT_TXMIX              0x04   /*Activates TXMix functionality. */

/* RxMode Register							(13) */
#define     JBIT_RXCRCEN            0x80   /*enables the CRC generation during reception.. */
#define     JBIT_RXNOERR            0x08   /*If 1, receiver does not receive less than 4 bits. */
#define     JBIT_RXMULTIPLE         0x04   /*Activates reception mode for multiple responses. */

/* Definitions for Tx and Rx		    (12, 13) */
#define     JBIT_106KBPS            0x00   /*Activates speed of 106kbps. */
#define     JBIT_212KBPS            0x10   /*Activates speed of 212kbps. */
#define     JBIT_424KBPS            0x20   /*Activates speed of 424kbps. */
#define     JBIT_848KBPS            0x30   /*Activates speed of 848kbps. */
#define     JBIT_1_6MBPS            0x40   /*Activates speed of 1.6Mbps. */
#define     JBIT_3_2MBPS            0x50   /*Activates speed of 3_3Mbps. */

#define     JBIT_MIFARE             0x00   /*Activates Mifare communication mode. */
#define     JBIT_NFC                0x01   /*Activates NFC/Active communication mode. */
#define     JBIT_FELICA             0x02   /*Activates FeliCa communication mode. */

#define     JBIT_CRCEN              0x80   /*Activates transmit or receive CRC. */

/* TxControl Register						(14) */
#define     JBIT_INVTX2ON           0x80   /*Inverts the Tx2 output if drivers are switched on. */
#define     JBIT_INVTX1ON           0x40   /*Inverts the Tx1 output if drivers are switched on. */
#define     JBIT_INVTX2OFF          0x20   /*Inverts the Tx2 output if drivers are switched off. */
#define     JBIT_INVTX1OFF          0x10   /*Inverts the Tx1 output if drivers are switched off. */
#define     JBIT_TX2CW              0x08   /*Does not modulate the Tx2 output, only constant wave. */
#define     JBIT_CHECKRF            0x04   /*Does not activate the driver if an external RF is detected.
                                                Only valid in combination with JBIT_TX2RFEN and JBIT_TX1RFEN. */
#define     JBIT_TX2RFEN            0x02   /*Switches the driver for Tx2 pin on. */
#define     JBIT_TX1RFEN            0x01   /*Switches the driver for Tx1 pin on. */

/* TxAuto Register							(15) */
#define     JBIT_AUTORFOFF          0x80   /*Switches the RF automatically off after transmission is finished. */
#define     JBIT_FORCE100ASK        0x40   /*Activates 100%ASK mode independent of driver settings. */
#define     JBIT_AUTOWAKEUP         0x20   /*Activates automatic wakeup of the FM175XX if set to 1. */
#define     JBIT_CAON               0x08   /*Activates the automatic time jitter generation by switching 
                                                on the Rf field as defined in ECMA-340. */
#define     JBIT_INITIALRFON        0x04   /*Activate the initial RF on procedure as defined iun ECMA-340. */
#define     JBIT_TX2RFAUTOEN        0x02   /*Switches on the driver two automatically according to the 
                                                other settings. */
#define     JBIT_TX1RFAUTOEN        0x01   /*Switches on the driver one automatically according to the 
                                                other settings. */

/* Demod Register 							(19) */
#define     JBIT_FIXIQ              0x20   /*If set to 1 and the lower bit of AddIQ is set to 0, the receiving is fixed to I channel.
                                                If set to 1 and the lower bit of AddIQ is set to 1, the receiving is fixed to Q channel. */

/* Felica/NFC 2 Register 				    (1B) */
#define     JBIT_WAITFORSELECTED    0x80   /*If this bit is set to one, only passive communication modes are possible.
                                                In any other case the AutoColl Statemachine does not exit. */
#define     JBIT_FASTTIMESLOT       0x40   /*If this bit is set to one, the response time to the polling command is half as normal. */

/* Mifare Register 							(1C) */
#define     JBIT_MFHALTED           0x04   /*Configures the internal state machine only to answer to
                                                Wakeup commands according to ISO 14443-3. */

/* RFU 0x1D Register 					    (1D) */
#define     JBIT_PARITYDISABLE      0x10   /*Disables the parity generation and sending independent from the mode. */
#define     JBIT_LARGEBWPLL         0x08   /* */
#define     JBIT_MANUALHPCF         0x04   /* */
/*@}*/

/* FM175XX Bit definitions of Page 2
 *  Below there are useful bit definition of the FM175XX register set.
 */
/* TxBitPhase Register 					    (25) */
#define     JBIT_RCVCLKCHANGE       0x80   /*If 1 the receive clock may change between Rf and oscilator. */

/* RfCFG Register 							(26) */
#define     JBIT_RFLEVELAMP         0x80   /*Activates the RF Level detector amplifier. */

/* TMode Register 							(2A) */
#define     JBIT_TAUTO              0x80   /*Sets the Timer start/stop conditions to Auto mode. */
#define     JBIT_TAUTORESTART       0x10   /*Restarts the timer automatically after finished
                                                counting down to 0. */
/*@}*/

/* FM175XX Bit definitions of Page 3
 *  Below there are useful bit definition of the FM175XX register set.
 */
/* AutoTest Register 					    (36) */
#define     JBIT_AMPRCV             0x40   /* */
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bitmask Definitions
 * ////////////////////////////////////////////////////////////////////////// */


/* Command register                 (0x01)*/
#define     JMASK_COMMAND           0x0F   /*Bitmask for Command bits in Register JREG_COMMAND. */
#define     JMASK_COMMAND_INV       0xF0   /*Inverted bitmask of JMASK_COMMAND. */

/* Waterlevel register              (0x0B)*/
#define     JMASK_WATERLEVEL        0x3F   /*Bitmask for Waterlevel bits in register JREG_WATERLEVEL. */

/* Control register                 (0x0C)*/
#define     JMASK_RXBITS            0x07   /*Bitmask for RxLast bits in register JREG_CONTROL. */

/* Mode register                    (0x11)*/
#define     JMASK_CRCPRESET         0x03   /*Bitmask for CRCPreset bits in register JREG_MODE. */

/* TxMode register                  (0x12, 0x13)*/
#define     JMASK_SPEED             0x70   /*Bitmask for Tx/RxSpeed bits in register JREG_TXMODE and JREG_RXMODE. */
#define     JMASK_FRAMING           0x03   /*Bitmask for Tx/RxFraming bits in register JREG_TXMODE and JREG_RXMODE. */

/* TxSel register                   (0x16)*/
#define     JMASK_LOADMODSEL        0xC0   /*Bitmask for LoadModSel bits in register JREG_TXSEL. */
#define     JMASK_DRIVERSEL         0x30   /*Bitmask for DriverSel bits in register JREG_TXSEL. */
#define     JMASK_SIGOUTSEL         0x0F   /*Bitmask for SigoutSel bits in register JREG_TXSEL. */

/* RxSel register                   (0x17)*/
#define     JMASK_UARTSEL           0xC0   /*Bitmask for UartSel bits in register JREG_RXSEL. */
#define     JMASK_RXWAIT            0x3F   /*Bitmask for RxWait bits in register JREG_RXSEL. */

/* RxThreshold register             (0x18)*/
#define     JMASK_MINLEVEL          0xF0   /*Bitmask for MinLevel bits in register JREG_RXTHRESHOLD. */
#define     JMASK_COLLEVEL          0x07   /*Bitmask for CollLevel bits in register JREG_RXTHRESHOLD. */

/* Demod register                   (0x19)*/
#define     JMASK_ADDIQ             0xC0   /*Bitmask for ADDIQ bits in register JREG_DEMOD. */
#define     JMASK_TAURCV            0x0C   /*Bitmask for TauRcv bits in register JREG_DEMOD. */
#define     JMASK_TAUSYNC           0x03   /*Bitmask for TauSync bits in register JREG_DEMOD. */

/* FeliCa / FeliCa2 register        (0x1A, 0x1B)*/
#define     JMASK_FELICASYNCLEN     0xC0   /*Bitmask for FeliCaSyncLen bits in registers JREG_FELICANFC. */
#define     JMASK_FELICALEN         0x3F   /*Bitmask for FeliCaLenMin and FeliCaLenMax in 
                                                registers JREG_FELICANFC and JREG_FELICANFC2. */
/* Mifare register                  (0x1C)*/
#define     JMASK_SENSMILLER        0xE0   /*Bitmask for SensMiller bits in register JREG_MIFARE. */
#define     JMASK_TAUMILLER         0x18   /*Bitmask for TauMiller bits in register JREG_MIFARE. */
#define     JMASK_TXWAIT            0x03   /*Bitmask for TxWait bits in register JREG_MIFARE. */

/* Manual Rcv register				(0x1D)*/
#define     JMASK_HPCF				0x03   /*Bitmask for HPCF filter adjustments. */

/* TxBitPhase register              (0x25)*/
#define     JMASK_TXBITPHASE        0x7F   /*Bitmask for TxBitPhase bits in register JREG_TXBITPHASE. */

/* RFCfg register                   (0x26)*/
#define     JMASK_RXGAIN            0x70   /*Bitmask for RxGain bits in register JREG_RFCFG. */
#define     JMASK_RFLEVEL           0x0F   /*Bitmask for RfLevel bits in register JREG_RFCFG. */

/* GsN register                     (0x27)*/
#define     JMASK_CWGSN             0xF0   /*Bitmask for CWGsN bits in register JREG_GSN. */
#define     JMASK_MODGSN            0x0F   /*Bitmask for ModGsN bits in register JREG_GSN. */

/* CWGsP register                   (0x28)*/
#define     JMASK_CWGSP             0x3F   /*Bitmask for CWGsP bits in register JREG_CWGSP. */

/* ModGsP register                  (0x29)*/
#define     JMASK_MODGSP            0x3F   /*Bitmask for ModGsP bits in register JREG_MODGSP. */

/* TMode register                   (0x2A)*/
#define     JMASK_TGATED            0x60   /*Bitmask for TGated bits in register JREG_TMODE. */
#define     JMASK_TPRESCALER_HI     0x0F   /*Bitmask for TPrescalerHi bits in register JREG_TMODE. */








//============================================================================
#define		JREG_EXT_REG_ENTRANCE		0x0F	//ext register entrance
//============================================================================
#define		JBIT_EXT_REG_WR_ADDR		0X40	//wrire address cycle
#define		JBIT_EXT_REG_RD_ADDR		0X80	//read address cycle
#define		JBIT_EXT_REG_WR_DATA		0XC0	//write data cycle
#define		JBIT_EXT_REG_RD_DATA		0X00	//read data cycle

//============================================================================
#define		JREG_LVD_CTRL				0x1D	//Low Votage Detect register
//============================================================================

//============================================================================
#define		JREG_LPCD_CTRL1				0x01	//Lpcd Ctrl register1
//============================================================================
#define		JBIT_LPCD_EN				0x01	//enble LPCD
#define		JBIT_LPCD_RSTN				0X02	//lpcd reset
#define		JBIT_LPCD_CALIBRATE_EN		0x04	//into lpcd calibrate mode
#define		JBIT_LPCD_SENSE_1			0x08	//Compare times 1 or 3
#define		JBIT_LPCD_IE				0x10	//Enable LPCD IE
#define		JBIT_BIT_CTRL_SET			0x20	//Lpcd register Bit ctrl set bit
#define		JBIT_BIT_CTRL_CLR			0x00	//Lpcd register Bit ctrl clear bit
//============================================================================

//============================================================================
#define		JREG_LPCD_CTRL2				0x02	//Lpcd Ctrl register2
//============================================================================

//============================================================================
#define		JREG_LPCD_CTRL3				0x03	//Lpcd Ctrl register3
//============================================================================

//============================================================================

//============================================================================
#define		JREG_LPCD_CTRL4				0x04	//Lpcd Ctrl register4
//============================================================================

//============================================================================

//============================================================================
#define		JREG_LPCD_BIAS_CURRENT		0x05	//Lpcd bias current register
//============================================================================

//============================================================================
#define		JREG_LPCD_ADC_REFERECE		0x06	//Lpcd adc reference register 
//============================================================================

//============================================================================
#define		JREG_LPCD_T1CFG				0x07	//T1Cfg[3:0] register 
//============================================================================

//============================================================================
#define		JREG_LPCD_T2CFG				0x08	//T2Cfg[4:0] register 
//============================================================================

//============================================================================
#define		JREG_LPCD_T3CFG				0x09	//T2Cfg[4:0] register 
//============================================================================

//============================================================================
#define		JREG_LPCD_VMIDBD_CFG		0x0A	//VmidBdCfg[4:0] register 
//============================================================================

//============================================================================
#define		JREG_LPCD_AUTO_WUP_CFG			0x0B	//Auto_Wup_Cfg register 
//============================================================================

//============================================================================
#define		JREG_LPCD_ADC_RESULT_L			0x0C	//ADCResult[5:0] Register 
//============================================================================

//============================================================================
#define		JREG_LPCD_ADC_RESULT_H			0x0D	//ADCResult[7:6] Register 
//============================================================================

//============================================================================
#define		JREG_LPCD_THRESHOLD_MIN_L		0x0E	//LpcdThreshold_L[5:0] Register 
//============================================================================

//============================================================================
#define		JREG_LPCD_THRESHOLD_MIN_H		0x0F	//LpcdThreshold_L[7:6] Register 
//============================================================================

//============================================================================
#define		JREG_LPCD_THRESHOLD_MAX_L		0x10	//LpcdThreshold_H[5:0] Register 
//============================================================================

//============================================================================
#define		JREG_LPCD_THRESHOLD_MAX_H		0x11	//LpcdThreshold_H[7:6] Register 
//============================================================================

//============================================================================
#define		JREG_LPCD_IRQ				0x12	//LpcdStatus Register 
//============================================================================
#define		JBIT_CARD_IN_IRQ			0x01	//irq of card in
#define		JBIT_LPCD23_IRQ				0x02	//irq of LPCD 23 end
#define		JBIT_CALIBRATE_IRQ				0x04	//irq of calib end
#define		JBIT_LP10K_TESTOK_IRQ		0x08	//irq of lp osc 10K ok
#define		JBIT_AUTO_WUP_IRQ			0x10	//irq of Auto wake up 
//============================================================================

//============================================================================
#define		JREG_LPCD_RFT1				0x13	//Aux1 select Register 
//============================================================================
//#define		BFL_JBIT_AUX1_CLOSE				0x00	//close aux1 out
//#define		BFL_JBIT_AUX1_VDEM_LPCD			0x01	//vdem of lpcd
//#define		BFL_JBIT_AUX1_VP_LPCD			0x02	//voltage of charecap
//============================================================================

//============================================================================
#define		JREG_LPCD_RFT2				0x14	//Aux2 select Register 
//============================================================================
//#define		BFL_JBIT_AUX2_CLOSE				0x00	//close aux1 out
//#define		BFL_JBIT_AUX2_VDEM_LPCD			0x01	//vdem of lpcd
//#define		BFL_JBIT_AUX2_VP_LPCD			0x02	//voltage of charecap
//============================================================================

//============================================================================
#define		JREG_LPCD_RFT3				0x15	//LPCD test1 Register 
//============================================================================
//#define		BFL_JBIT_LP_OSC10K_EN			0x01	//enable lp osc10k
//#define		BFL_JBIT_LP_OSC_CALIBRA_EN		0x02	//enable lp osc10k calibra mode
//#define		BFL_JBIT_LP_CURR_TEST			0x04	//enable lp t1 current test
//#define		BFL_JBIT_LPCD_TEST2_LPCD_OUT	0x08	//lpcd_test2[3]:LPCD_OUT 
//============================================================================

//============================================================================
#define		JREG_LPCD_RFT4				0x16	//LPCD test2 Register 
//============================================================================
//#define		BFL_JBIT_T1_OUT_EN				0x01	//D5:T1_OUT
//#define		BFL_JBIT_OSCCLK_OUT_EN			0x02	//D4:OSC_CLK_OUT
//#define		BFL_JBIT_OSCEN_OUT_EN			0x04	//D3:OSC_EN
//#define		BFL_JBIT_LP_CLK_LPCD_OUT_EN		0x08	//D2:LP_CLK or LPCD_OUT
//#define		BFL_JBIT_T3_OUT_EN				0x10	//D1:T3_OUT
//============================================================================		

//============================================================================
//#define		BFL_JREG_LP_CLK_CNT1			0x17	//lp_clk_cnt[5:0] Register 
//============================================================================

//============================================================================
//#define		BFL_JREG_LP_CLK_CNT2			0x18	//lp_clk_cnt[7:6] Register 
//============================================================================

//============================================================================
//#define		BFL_JREG_VERSIONREG2			0x19	//VersionReg2[1:0] Register 
//============================================================================

//============================================================================
//#define		BFL_JREG_IRQ_BAK				0x1A	//Irq bak Register 
//============================================================================
//#define		BFL_JBIT_IRQ_INV_BAK			0x01	//Irq Inv backup
//#define		BFL_JBIT_IRQ_PUSHPULL_BAK		0x02	//Irq pushpull backup
//============================================================================


//============================================================================
#define		JREG_LPCD_RFT5				0x1B	//LPCD TEST3 Register 
//============================================================================
//#define		BFL_JBIT_LPCD_TESTEN			0x01	//lPCD test mode
//#define		BFL_JBIT_AWUP_TSEL				0x02	//Auto wakeup test mode
//#define		BFL_JBIT_RNG_MODE_SEL			0x04	//RNG  mode sel
//#define		BFL_JBIT_USE_RET				0x08	//use retention mode
//============================================================================

//============================================================================
#define		JREG_LPCD_MISC				      0x1C	//LPCD Misc Register 
//============================================================================
#define		BFL_JBIT_CALIBRATE_VMID_EN			0x01	//lPCD test mode
#define		BFL_JBIT_AMP_EN_SEL		        0x04	//LPCD amp en select

//============================================================================

#endif 


