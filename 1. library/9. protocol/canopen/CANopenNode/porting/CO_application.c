/*
 * CANopen main program file.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        main_generic.c
 * @author      Hamed Jafarzadeh 	2022
 * 				Janez Paternoster	2021
 * @copyright   2021 Janez Paternoster
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

#include "CO_platform.h"
#include "CO_application.h"
#include "CANopen.h"
#include "main.h"
#include <stdio.h>

#include "CO_storageBlank.h"
#include "OD.h"


/* Global variables and objects */
CO_t* CO; 
CANopenNode* canopenNode; 

/* Static variables */
static CO_time_t   last_time, now;
static CO_ReturnError_t err;



/* This function will basically setup the CANopen node */
int
canopen_app_init(CANopenNode* node) {

    if ((node == NULL) || (!node->canSend || !node->canRecv || !node->canInit || !node->canStart ||
        !node->canStop || !node->canIntEnable || !node->canIntDisable || !node->tmrInit || !node->tmrStart ||
        !node->tmrStop)) {

        CO_LOG_Error("Error: CANopenNode object is not properly initialized");
        return 1;
    }

    // Keep a copy global reference of CANopenNode Object
    canopenNode = node;

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    static CO_storage_t storage;
    static CO_storage_entry_t storageEntries[] = {{.addr = &OD_PERSIST_COMM,
                                                   .len = sizeof(OD_PERSIST_COMM),
                                                   .subIndexOD = 2,
                                                   .attr = CO_storage_cmd | CO_storage_restore,
                                                   .addrNV = NULL}};
    uint8_t storageEntriesCount = sizeof(storageEntries) / sizeof(storageEntries[0]);
    uint32_t storageInitError = 0;
#endif

    /* Allocate memory */
    CO_config_t* config_ptr = NULL;
#ifdef CO_MULTIPLE_OD
    /* example usage of CO_MULTIPLE_OD (but still single OD here) */
    CO_config_t co_config = {0};
    OD_INIT_CONFIG(co_config); /* helper macro from OD.h */
    co_config.CNT_LEDS = 1;
    co_config.CNT_LSS_SLV = 1;
    config_ptr = &co_config;
#endif /* CO_MULTIPLE_OD */

    uint32_t heapMemoryUsed;
    CO = CO_new(config_ptr, &heapMemoryUsed);
    if (CO == NULL) {
        CO_LOG_Error("Error: Can't allocate memory");
        return 1;
    } else {
        CO_LOG_Info("Allocated %u bytes for CANopen objects", heapMemoryUsed);
    }

    canopenNode->canOpenStack = CO;

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    err = CO_storageBlank_init(&storage, CO->CANmodule, OD_ENTRY_H1010_storeParameters,
                               OD_ENTRY_H1011_restoreDefaultParameters, storageEntries, storageEntriesCount,
                               &storageInitError);

    if (err != CO_ERROR_NO && err != CO_ERROR_DATA_CORRUPT) {
        CO_LOG_Error("Error: Storage %d", storageInitError);
        return 2;
    }
#endif

    canopen_app_resetCommunication();
    return 0;
}




int
canopen_app_resetCommunication() {
    /* CANopen communication reset - initialize CANopen objects *******************/
    CO_LOG_Info("CANopenNode - Reset communication...");

    /* Wait rt_thread. */
    CO->CANmodule->CANnormal = false;

    /* Initialize CAN */
    canopenNode->canInit(canopenNode->baudrate);
    canopenNode->tmrInit();

    /* Enter CAN configuration. */
    CO_CANsetConfigurationMode((void*)canopenNode);
    CO_CANmodule_disable(CO->CANmodule);

    /* initialize CANopen */
    err = CO_CANinit(CO, canopenNode, canopenNode->baudrate); // Bitrate for STM32 microcontroller is being set in MXCube Settings
    if (err != CO_ERROR_NO) {
        CO_LOG_Error("Error: CAN initialization failed: %d", err);
        return 1;
    }

#if ((CO_CONFIG_LSS)&CO_CONFIG_LSS_SLAVE) != 0
    CO_LSS_address_t lssAddress = {.identity = {.vendorID = OD_PERSIST_COMM.x1018_identity.vendor_ID,
                                                .productCode = OD_PERSIST_COMM.x1018_identity.productCode,
                                                .revisionNumber = OD_PERSIST_COMM.x1018_identity.revisionNumber,
                                                .serialNumber = OD_PERSIST_COMM.x1018_identity.serialNumber}};
    err = CO_LSSinit(CO, &lssAddress, &canopenNode->desiredNodeID, &canopenNode->baudrate);
    if (err != CO_ERROR_NO) {
        CO_LOG_Error("Error: LSS slave initialization failed: %d", err);
        return 2;
    }
#endif // ((CO_CONFIG_LSS)&CO_CONFIG_LSS_SLAVE) != 0

    canopenNode->activeNodeID = canopenNode->desiredNodeID;
    uint32_t errInfo = 0;

    err = CO_CANopenInit(CO,                   /* CANopen object */
                         NULL,                 /* alternate NMT */
                         NULL,                 /* alternate em */
                         OD,                   /* Object dictionary */
                         OD_STATUS_BITS,       /* Optional OD_statusBits */
                         NMT_CONTROL,          /* CO_NMT_control_t */
                         FIRST_HB_TIME,        /* firstHBTime_ms */
                         SDO_SRV_TIMEOUT_TIME, /* SDOserverTimeoutTime_ms */
                         SDO_CLI_TIMEOUT_TIME, /* SDOclientTimeoutTime_ms */
                         SDO_CLI_BLOCK,        /* SDOclientBlockTransfer */
                         canopenNode->activeNodeID, &errInfo);
    if (err != CO_ERROR_NO && err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
        if (err == CO_ERROR_OD_PARAMETERS) {
            CO_LOG_Error("Error: Object Dictionary entry 0x%08X", errInfo);
        } else {
            CO_LOG_Error("Error: CANopen initialization failed: %d", err);
        }
        return 3;
    }

    err = CO_CANopenInitPDO(CO, CO->em, OD, canopenNode->activeNodeID, &errInfo);
    if (err != CO_ERROR_NO && err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
        if (err == CO_ERROR_OD_PARAMETERS) {
            CO_LOG_Error("Error: Object Dictionary entry 0x%08X", errInfo);
        } else {
            CO_LOG_Error("Error: PDO initialization failed: %d", err);
        }
        return 4;
    }

    /* Configure Timer interrupt function for execution every 1 millisecond */
    canopenNode->tmrStart(); //1ms interrupt

    /* Configure CAN transmit and receive interrupt */

    /* Configure CANopen callbacks, etc */
    if (!CO->nodeIdUnconfigured) {

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
        if (storageInitError != 0) {
            CO_errorReport(CO->em, CO_EM_NON_VOLATILE_MEMORY, CO_EMC_HARDWARE, storageInitError);
        }
#endif
    } else {
        CO_LOG_Info("CANopenNode - Node-id not initialized");
    }

    /* start CAN */
    CO_CANsetNormalMode(CO->CANmodule);
    CO_LOG_Info("CANopenNode - Running...");

    last_time = now = CO_Platform_GetMs();
    return 0;
}



void
canopen_app_process() {
    /* loop for normal program execution ******************************************/
    /* get time difference since last function call */
    now = CO_Platform_GetMs();

    if ((now - last_time) > 0) { // Make sure more than 1ms elapsed
        /* CANopen process */
        CO_NMT_reset_cmd_t reset_status;
        uint32_t timeDifference_us = (now - last_time) * 1000;
        last_time = now;
        reset_status = CO_process(CO, false, timeDifference_us, NULL);
        canopenNode->outStatusLEDRed = CO_LED_RED(CO->LEDs, CO_LED_CANopen);
        canopenNode->outStatusLEDGreen = CO_LED_GREEN(CO->LEDs, CO_LED_CANopen);

        if (reset_status == CO_RESET_COMM) {
            /* delete objects from memory */
        	canopenNode->tmrStop();
            CO_CANsetConfigurationMode((void*)canopenNode);
            CO_delete(CO);
            CO_LOG_Info("CANopenNode Reset Communication request");
            canopen_app_init(canopenNode); // Reset Communication routine
        } else if (reset_status == CO_RESET_APP) {
            CO_LOG_Info("CANopenNode Device Reset");
            NVIC_SystemReset(); // Reset the MCU
        }
    }
}

/* Thread function executes in constant intervals, this function can be called from FreeRTOS tasks or Timers ********/
void
canopen_app_interrupt(void) {
    CO_LOCK_OD(CO->CANmodule);
    if (!CO->nodeIdUnconfigured && CO->CANmodule->CANnormal) {
        bool_t syncWas = false;
        /* get time difference since last function call */
        uint32_t timeDifference_us = 1000; // 1ms second

#if (CO_CONFIG_SYNC) & CO_CONFIG_SYNC_ENABLE
        syncWas = CO_process_SYNC(CO, timeDifference_us, NULL);
#endif
#if (CO_CONFIG_PDO) & CO_CONFIG_RPDO_ENABLE
        CO_process_RPDO(CO, syncWas, timeDifference_us, NULL);
#endif
#if (CO_CONFIG_PDO) & CO_CONFIG_TPDO_ENABLE
        CO_process_TPDO(CO, syncWas, timeDifference_us, NULL);
#endif

        /* Further I/O or nonblocking application code may go here. */
    }
    CO_UNLOCK_OD(CO->CANmodule);
}
