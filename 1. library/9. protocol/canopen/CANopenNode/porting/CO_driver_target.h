/*
 * Device and application specific definitions for CANopenNode.
 *
 * @file        CO_driver_target.h
 * @author      Hamed Jafarzadeh 	2022
 * 				Tilen Marjerle		2021
 * 				Janez Paternoster	2020
 * @copyright   2004 - 2020 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CO_DRIVER_TARGET_H
#define CO_DRIVER_TARGET_H

/* This file contains device and application specific definitions.
 * It is included from CO_driver.h, which contains documentation
 * for common definitions below. */

#include "main.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Determining the CANOpen Driver



#undef CO_CONFIG_STORAGE_ENABLE // We don't need Storage option, implement based on your use case and remove this line from here

#ifdef CO_DRIVER_CUSTOM
#include "CO_driver_custom.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* default values for CO_CANopenInit() */
/* NMT control bits, if is a master, CO_NMT_STARTUP_TO_OPERATIONAL can be used to start the node in operational state */
#define NMT_CONTROL                         (CO_NMT_ERR_ON_ERR_REG | CO_ERR_REG_GENERIC_ERR | CO_ERR_REG_COMMUNICATION)
#define FIRST_HB_TIME                       (500U)
#define SDO_SRV_TIMEOUT_TIME                (1000U)
#define SDO_CLI_TIMEOUT_TIME                (500U)
#define SDO_CLI_BLOCK                       (false)
#define OD_STATUS_BITS                      (NULL)


/* Default SDO server ID */
#define SDO_DEFAULT_SERVER_ID               (0x01U)

/* Manufacturer specific error code */
#define CO_EM_MANUFACTURER_CRITICAL         (CO_EM_MANUFACTURER_START + 0x10)



/**
 * \brief           PDO configuration
 *
 * If set to 0, PDO is disabled.
 */
#define CO_CONFIG_PDO      (CO_CONFIG_RPDO_ENABLE | CO_CONFIG_TPDO_ENABLE | CO_CONFIG_RPDO_TIMERS_ENABLE |          \
                            CO_CONFIG_TPDO_TIMERS_ENABLE | CO_CONFIG_PDO_SYNC_ENABLE | CO_CONFIG_PDO_OD_IO_ACCESS | \
                            CO_CONFIG_FLAG_CALLBACK_PRE)   


/**
 * \brief           Emergency producer configuration
 *
 * If set to 0, emergency producer is disabled.
 */
#define CO_CONFIG_EM        (CO_CONFIG_EM_PRODUCER | CO_CONFIG_EM_HISTORY)

/**
 * \brief           LSS slave configuration
 *
 * If set to 0, LSS slave is disabled.
 */
#define CO_CONFIG_LSS       (CO_CONFIG_LSS_SLAVE)

/**
 * \brief           Heartbeat consumer configuration
 *
 * If set to 0, heartbeat consumer is disabled.
 */
#define CO_CONFIG_HB_CONS   (CO_CONFIG_HB_CONS_ENABLE | CO_CONFIG_HB_CONS_CALLBACK_MULTI)


/**
 * \brief           SRDO configuration
 *
 * If set to 0, SRDO is disabled.else if CO_CONFIG_SRDO_ENABLE, SRDO is enabled.
 */
#define CO_CONFIG_SRDO      (0)


/**
 * \brief           CRC16 configuration
 *
 * If set to 0, CRC16 is disabled.else if CO_CONFIG_CRC16_ENABLE, CRC16 is enabled.
 * 
 * is using on sdo block transfer
 */
#define CO_CONFIG_CRC16     (0)



/**
 * @brief           NMT configuration
 * 
 */
#define CO_CONFIG_NMT       (CO_CONFIG_NMT_CALLBACK_CHANGE | CO_CONFIG_FLAG_CALLBACK_PRE)


/**
 * @brief           SDO client configuration
*/
#define CO_CONFIG_SDO_CLI   (CO_CONFIG_SDO_CLI_ENABLE | CO_CONFIG_FLAG_CALLBACK_PRE)



/**
 * @brief           SDO server configuration
*/
#define CO_CONFIG_SDO_SRV   (CO_CONFIG_FLAG_CALLBACK_PRE)



/**
 * @brief           FIFO configuration
*/
#define CO_CONFIG_FIFO      (CO_CONFIG_FIFO_ENABLE | CO_CONFIG_FLAG_CALLBACK_PRE)


/* Basic definitions. If big endian, CO_SWAP_xx macros must swap bytes. */
#define CO_LITTLE_ENDIAN
#define CO_SWAP_16(x) x
#define CO_SWAP_32(x) x
#define CO_SWAP_64(x) x

/* NULL is defined in stddef.h */
/* true and false are defined in stdbool.h */
/* int8_t to uint64_t are defined in stdint.h */
typedef uint_fast8_t    bool_t;
typedef float           float32_t;
typedef double          float64_t;

/**
 * \brief           CAN RX message for platform
 *
 * This is platform specific one
 */
typedef struct {
    uint32_t ident;  /*!< Standard identifier */
    uint8_t dlc;     /*!< Data length */
    uint8_t data[8]; /*!< Received data */
} CO_CANrxMsg_t;

/* Access to received CAN message */
#define CO_CANrxMsg_readIdent(msg) ((uint16_t)(((CO_CANrxMsg_t*)(msg)))->ident)
#define CO_CANrxMsg_readDLC(msg)   ((uint8_t)(((CO_CANrxMsg_t*)(msg)))->dlc)
#define CO_CANrxMsg_readData(msg)  ((uint8_t*)(((CO_CANrxMsg_t*)(msg)))->data)

/* Received message object */
typedef struct {
    uint16_t ident;
    uint16_t mask;
    void* object;
    void (*CANrx_callback)(void* object, void* message);
} CO_CANrx_t;

/* Transmit message object */
typedef struct {
    uint32_t ident;
    uint8_t DLC;
    uint8_t data[8];
    volatile bool_t bufferFull;
    volatile bool_t syncFlag;
} CO_CANtx_t;

/* CAN module object */
typedef struct {
    void* CANptr;
    CO_CANrx_t* rxArray;
    uint16_t rxSize;
    CO_CANtx_t* txArray;
    uint16_t txSize;
    uint16_t CANerrorStatus;
    volatile bool_t CANnormal;
    volatile bool_t useCANrxFilters;
    volatile bool_t bufferInhibitFlag;
    volatile bool_t firstCANtxMessage;
    volatile uint16_t CANtxCount;
    uint32_t errOld;

    /* STM32 specific features */
    uint32_t primask_send; /* Primask register for interrupts for send operation */
    uint32_t primask_emcy; /* Primask register for interrupts for emergency operation */
    uint32_t primask_od;   /* Primask register for interrupts for send operation */

} CO_CANmodule_t;

/* Data storage object for one entry */
typedef struct {
    void* addr;
    size_t len;
    uint8_t subIndexOD;
    uint8_t attr;
    /* Additional variables (target specific) */
    void* addrNV;
} CO_storage_entry_t;

/* (un)lock critical section in CO_CANsend() */
// Why disabling the whole Interrupt

extern uint32_t CO_Platform_Lock(void);
extern void     CO_Platform_Unlock(uint32_t primask);

#define CO_LOCK_CAN_SEND(CAN_MODULE)                            \
    do {                                                        \
        (CAN_MODULE)->primask_send = CO_Platform_Lock();        \
    } while (0)
#define CO_UNLOCK_CAN_SEND(CAN_MODULE)                          \
    do {                                                        \
        CO_Platform_Unlock((CAN_MODULE)->primask_send);         \
    } while (0)

/* (un)lock critical section in CO_errorReport() or CO_errorReset() */
#define CO_LOCK_EMCY(CAN_MODULE)                                \
    do {                                                        \
        (CAN_MODULE)->primask_emcy = CO_Platform_Lock();        \
    } while (0)
#define CO_UNLOCK_EMCY(CAN_MODULE)                              \
    do {                                                        \
        CO_Platform_Unlock((CAN_MODULE)->primask_emcy);         \
    } while (0)

/* (un)lock critical section when accessing Object Dictionary */
#define CO_LOCK_OD(CAN_MODULE)                                  \
    do {                                                        \
        (CAN_MODULE)->primask_od = CO_Platform_Lock();          \
    } while (0)
#define CO_UNLOCK_OD(CAN_MODULE)                                \
    do  {                                                       \
        CO_Platform_Unlock((CAN_MODULE)->primask_od);           \
    } while (0)

/* Synchronization between CAN receive and message processing threads. */
#define CO_MemoryBarrier()
#define CO_FLAG_READ(rxNew) ((rxNew) != NULL)
#define CO_FLAG_SET(rxNew)                                                                                             \
    do {                                                                                                               \
        CO_MemoryBarrier();                                                                                            \
        rxNew = (void*)1L;                                                                                             \
    } while (0)
#define CO_FLAG_CLEAR(rxNew)                                                                                           \
    do {                                                                                                               \
        CO_MemoryBarrier();                                                                                            \
        rxNew = NULL;                                                                                                  \
    } while (0)


void CO_CANinterrupt_RX(uint32_t FifoNum);
void CO_CANinterrupt_TX(uint32_t MailboxNumber);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CO_DRIVER_TARGET_H */
