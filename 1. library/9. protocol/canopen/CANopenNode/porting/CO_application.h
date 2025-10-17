/*
 * CO_application.h
 *
 *  Created on: Aug 7, 2022
 *      Author: hamed
 */

#ifndef CANOPENSTM32_CO_APPLICATION_H_
#define CANOPENSTM32_CO_APPLICATION_H_

#include "CANopen.h"

/* CANHandle : Pass in the CAN Handle to this function and it wil be used for all CAN Communications. It can be FDCan or CAN
 * and CANOpen Driver will take of care of handling that
 * HWInitFunction : Pass in the function that initialize the CAN peripheral, usually MX_CAN_Init
 * timerHandle : Pass in the timer that is going to be used for generating 1ms interrupt for tmrThread function,
 * please note that CANOpen Library will override HAL_TIM_PeriodElapsedCallback function, if you also need this function
 * in your codes, please take required steps

 */

#ifdef __cplusplus
extern "C" {
#endif

#include "OD.h"



#define    CAN_ERROR_NONE          0x00000000
#define    CAN_ERROR_BUSOFF        0x00000001
#define    CAN_ERROR_WARNING       0x00000002
#define    CAN_ERROR_PASSIVE       0x00000004
#define    CAN_ERROR_RX_OVERFLOW   0x00000008
#define    CAN_ERROR_TX_OVERFLOW   0x00000010



typedef struct {
    /*This is the Node ID that you ask the CANOpen stack to assign to your device, although it might not always
	 * be the final NodeID, after calling canopen_app_init() you should check ActiveNodeID of CANopenNode structure for assigned Node ID.
	 */
    uint8_t desiredNodeID; 

    /* Assigned Node ID */
    uint8_t activeNodeID; 

    /* This is the baudrate you've set in your CubeMX Configuration */
    uint16_t baudrate;     

    /*Pass in the timer that is going to be used for generating 1ms interrupt for tmrThread function,
	 * please note that CANOpen Library will override HAL_TIM_PeriodElapsedCallback function, if you also need this function in your codes, please take required steps
	 */
    /* Pass in the function that initialize the timer peripheral, usually MX_TIM_Init */
    void (*tmrInit)(void);
    /* Pass in the function that start the timer peripheral, usually TIM_Start*/
    void (*tmrStart)(void); 
    /* Pass in the function that stop the timer peripheral, usually TIM_Stop*/
    void (*tmrStop)(void); 

    /* Pass in the function that initialize the CAN peripheral, usually MX_CAN_Init */
    void (*canInit)(int baudrate);
    /* Pass in the function that start the CAN peripheral, usually CAN_Start, it should return 0 if success */
    int  (*canStart)(void);
    /* Pass in the function that stop the CAN peripheral, usually CAN_Stop, it should return 0 if success */
    int  (*canStop)(void);
    /* Pass in the function that send a message over CAN, usually CAN_TransmitMessage, it should return 0 if success */
    int  (*canSend)(uint32_t ident, uint8_t* data, uint8_t dlc, uint8_t rtr);
    /* Pass in the function that receive a message over CAN, usually CAN_ReceiveMessage, it should return 0 if success */
    int  (*canRecv)(uint8_t fifo, uint32_t* ident, uint8_t* data, uint8_t* dlc, uint8_t* rtr);
    /* Pass in the function that enable the CAN interrupt, usually CAN_EnableIT */
    int  (*canIntEnable)(void);
    /* Pass in the function that disable the CAN interrupt, usually CAN_DisableIT */
    int  (*canIntDisable)(void);       

    /* Pass in the function that read the CAN error counter, usually CAN_GetErrorCounter, it should return the error counter value
    *  @return : Error Value, can be a combination of CAN_ERROR_BUSOFF, CAN_ERROR_WARNING, CAN_ERROR_PASSIVE, CAN_ERROR_RX_OVERFLOW, CAN_ERROR_TX_OVERFLOW
    */
    uint32_t (*canErrRead)(void);


    /* Pass in the CAN Handle to this function and it wil be used for all CAN Communications. It can be FDCan or CAN
	 * and CANOpen Driver will take of care of handling that*/
    void* CANHandle;


    uint8_t outStatusLEDGreen; // This will be updated by the stack - Use them for the LED management
    uint8_t outStatusLEDRed;   // This will be updated by the stack - Use them for the LED management

    CO_t* canOpenStack;

} CANopenNode;


// In order to use CANOpen, you'll have it have a canopenNode structure somewhere in your codes, it is usually residing in CO_app_STM32.c
extern CANopenNode* canopenNode;


/* This function will initialize the required CANOpen Stack objects, allocate the memory and prepare stack for communication reset*/
int canopen_app_init(CANopenNode* handle);
/* This function will reset the CAN communication periperhal and also the CANOpen stack variables */
int canopen_app_resetCommunication();
/* This function will check the input buffers and any outstanding tasks that are not time critical, this function should be called regurarly from your code (i.e from your while(1))*/
void canopen_app_process();
/* Thread function executes in constant intervals, this function can be called from FreeRTOS tasks or Timers ********/
void canopen_app_interrupt(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CANOPENSTM32_CO_APP_STM32_H_ */
