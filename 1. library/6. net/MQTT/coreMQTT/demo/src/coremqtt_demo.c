/*
 * Lab-Project-coreMQTT-Agent 201215
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 */

/*
 * This file demonstrates numerous tasks all of which use the MQTT agent API
 * to send unique MQTT payloads to unique topics over the same MQTT connection
 * to the same MQTT agent.  Some tasks use QoS0 and others QoS1.
 *
 * Each created task is a unique instance of the task implemented by
 * prvSimpleSubscribePublishTask().  prvSimpleSubscribePublishTask()
 * subscribes to a topic then periodically publishes a message to the same
 * topic to which it has subscribed.  The command context sent to
 * MQTTAgent_Publish() contains a unique number that is sent back to the task
 * as a task notification from the callback function that executes when the
 * PUBLISH operation is acknowledged (or just sent in the case of QoS 0).  The
 * task checks the number it receives from the callback equals the number it
 * previously set in the command context before printing out either a success
 * or failure message.
 */

#include <stdint.h>
#define LOG_TAG "MQTT"
#define LOG_LVL  ELOG_LVL_INFO

/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo Specific configs. */
#include "core_mqtt_agent_config.h"

/* MQTT library includes. */
#include "core_mqtt.h"

/* MQTT agent include. */
#include "core_mqtt_agent.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

#include "bsp_rng.h"
#include "bsp_rtc.h"
#include "elog.h"

//#define DEMO_PUB_TOPIC    "/ext/ntp/iuhsU4N7n8A/EL-L1/request"
//#define DEMO_SUB_TOPIC    "/ext/ntp/iuhsU4N7n8A/EL-L1/response"
//#define DEMO_PUB_MESSAGE  "{\"deviceSendTime\": %u}", rtc_time_get()

#define DEMO_PUB_TOPIC    "/sys/iuhsU4N7n8A/EL-L1/thing/event/property/post"
#define DEMO_SUB_TOPIC    "/sys/iuhsU4N7n8A/EL-L1/thing/event/property/post_reply"
#define DEMO_PUB_MESSAGE  "{\
\"id\": \"2023\",\
\"version\": \"1.0\",\
\"sys\":{\
\"ack\":1\
},\
\"params\": {\
\"Power\": {\
\"value\": \"on\",\
\"time\": %u\
}\
},\
\"method\": \"thing.event.property.post\"\
}", rtc_time_get()

/**
 * @brief This demo uses task notifications to signal tasks from MQTT callback
 * functions.  MS_TO_WAIT_FOR_NOTIFICATION defines the time, in ticks,
 * to wait for such a callback.
 */
#define MS_TO_WAIT_FOR_NOTIFICATION            ( 10000 )

/**
 * @brief Size of statically allocated buffers for holding topic names and
 * payloads.
 */
#define STRING_BUFFER_LENGTH                   ( 256 )

/**
 * @brief Delay for the synchronous publisher task between publishes.
 */
#define DELAY_BETWEEN_PUBLISH_OPERATIONS_MS    ( 2000U )

/**
 * @brief Number of publishes done by each task in this demo.
 */
#define PUBLISH_COUNT                          ( 0xffffffffUL )

/**
 * @brief The maximum amount of time in milliseconds to wait for the commands
 * to be posted to the MQTT agent should the MQTT agent's command queue be full.
 * Tasks wait in the Blocked state, so don't use any CPU time.
 */
#define MAX_COMMAND_SEND_BLOCK_TIME_MS         ( 500 )

/*-----------------------------------------------------------*/

/**
 * @brief Defines the structure to use as the command callback context in this
 * demo.
 */
struct MQTTAgentCommandContext {
  MQTTStatus_t xReturnStatus;
  TaskHandle_t xTaskToNotify;
  uint32_t ulNotificationValue;
  void *pArgs;
};

/*-----------------------------------------------------------*/

/**
 * @brief Passed into MQTTAgent_Subscribe() as the callback to execute when the
 * broker ACKs the SUBSCRIBE message.  Its implementation sends a notification
 * to the task that called MQTTAgent_Subscribe() to let the task know the
 * SUBSCRIBE operation completed.  It also sets the xReturnStatus of the
 * structure passed in as the command's context to the value of the
 * xReturnStatus parameter - which enables the task to check the status of the
 * operation.
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pxCommandContext Context of the initial command.
 * @param[in].xReturnStatus The result of the command.
 */
static void prvSubscribeCommandCallback(MQTTAgentCommandContext_t *pxCommandContext,
                                        MQTTAgentReturnInfo_t *pxReturnInfo);

/**
 * @brief Passed into MQTTAgent_Publish() as the callback to execute when the
 * broker ACKs the PUBLISH message.  Its implementation sends a notification
 * to the task that called MQTTAgent_Publish() to let the task know the
 * PUBLISH operation completed.  It also sets the xReturnStatus of the
 * structure passed in as the command's context to the value of the
 * xReturnStatus parameter - which enables the task to check the status of the
 * operation.
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pxCommandContext Context of the initial command.
 * @param[in].xReturnStatus The result of the command.
 */
static void prvPublishCommandCallback(MQTTAgentCommandContext_t *pxCommandContext,
                                      MQTTAgentReturnInfo_t *pxReturnInfo);

/**
 * @brief Called by the task to wait for a notification from a callback function
 * after the task first executes either MQTTAgent_Publish()* or
 * MQTTAgent_Subscribe().
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pxCommandContext Context of the initial command.
 * @param[out] pulNotifiedValue The task's notification value after it receives
 * a notification from the callback.
 *
 * @return pdTRUE if the task received a notification, otherwise pdFALSE.
 */
static BaseType_t prvWaitForCommandAcknowledgment(uint32_t *pulNotifiedValue);

/**
 * @brief Passed into MQTTAgent_Subscribe() as the callback to execute when
 * there is an incoming publish on the topic being subscribed to.  Its
 * implementation just logs information about the incoming publish including
 * the publish messages source topic and payload.
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pvIncomingPublishCallbackContext Context of the initial command.
 * @param[in] pxPublishInfo Deserialized publish.
 */
static void prvIncomingPublishCallback(void *pvIncomingPublishCallbackContext,
                                       MQTTPublishInfo_t *pxPublishInfo);

/**
 * @brief Subscribe to the topic the demo task will also publish to - that
 * results in all outgoing publishes being published back to the task
 * (effectively echoed back).
 *
 * @param[in] xQoS The quality of service (QoS) to use.  Can be zero or one
 * for all MQTT brokers.  Can also be QoS2 if supported by the broker.  AWS IoT
 * does not support QoS2.
 */
static bool prvSubscribeToTopic(MQTTQoS_t xQoS,
                                char *pcTopicFilter);

/**
 * @brief The function that implements the task demonstrated by this file.
 */
static void prvSimpleSubscribePublishTask(void *pvParameters);

/*-----------------------------------------------------------*/

/**
 * @brief The MQTT agent manages the MQTT contexts.  This set the handle to the
 * context used by this demo.
 */
extern MQTTAgentContext_t xGlobalMqttAgentContext;

/*-----------------------------------------------------------*/

/**
 * @brief The buffer to hold the topic filter. The topic is generated at runtime
 * by adding the task names.
 *
 * @note The topic strings must persist until unsubscribed.
 */
#if NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE > 0
static char pubTopicBuf[NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE][STRING_BUFFER_LENGTH];
static char subTopicBuf[NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE][STRING_BUFFER_LENGTH];
#else
static char topicBuf[1U][STRING_BUFFER_LENGTH];
#endif

/**
 * @brief Pass and fail counts for each created task.
 */
#if NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE > 0
static volatile uint32_t ulQoS0FailCount[(NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE + 1) / 2] = {0UL},
    ulQoS1FailCount[(NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE + 1) / 2] = {0UL};
static volatile uint32_t ulQoS0PassCount[(NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE + 1) / 2] = {0UL},
    ulQoS1PassCount[(NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE + 1) / 2] = {0UL};
#else
static volatile uint32_t ulQoS0FailCount[1] = {0UL}, ulQoS1FailCount[1] = {0UL};
static volatile uint32_t ulQoS0PassCount[1] = {0UL}, ulQoS1PassCount[1] = {0UL};
#endif

/*-----------------------------------------------------------*/

void vStartSimpleSubscribePublishTask(uint32_t ulNumberToCreate,
                                      configSTACK_DEPTH_TYPE uxStackSize,
                                      UBaseType_t uxPriority) {
    char pcTaskNameBuf[15];
    uint32_t ulTaskNumber;
    
    /* Each instance of prvSimpleSubscribePublishTask() generates a unique name
     * and topic filter for itself from the number passed in as the task
     * parameter. */
    /* Create a few instances of vSimpleSubscribePublishTask(). */
    for (ulTaskNumber = 0; ulTaskNumber < ulNumberToCreate; ulTaskNumber++) {
        memset(pcTaskNameBuf, 0x00, sizeof(pcTaskNameBuf));
        snprintf(pcTaskNameBuf, 10, "SubPub%d", (int)ulTaskNumber);
        xTaskCreate(prvSimpleSubscribePublishTask,
                    pcTaskNameBuf,
                    uxStackSize,
                    (void *)ulTaskNumber,
                    uxPriority,
                    NULL);
    }
}

/*-----------------------------------------------------------*/

static void prvPublishCommandCallback(MQTTAgentCommandContext_t *pxCommandContext,
                                      MQTTAgentReturnInfo_t *pxReturnInfo) {
    /* Store the result in the application defined context so the task that
     * initiated the publish can check the operation's status. */
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;
    
    if (pxCommandContext->xTaskToNotify != NULL) {
        /* Send the context's ulNotificationValue as the notification value so
         * the receiving task can check the value it set in the context matches
         * the value it receives in the notification. */
        xTaskNotify(pxCommandContext->xTaskToNotify,
                    pxCommandContext->ulNotificationValue,
                    eSetValueWithOverwrite);
    }
}

/*-----------------------------------------------------------*/

static void prvSubscribeCommandCallback(MQTTAgentCommandContext_t *pxCommandContext,
                                        MQTTAgentReturnInfo_t *pxReturnInfo) {
    bool xSubscriptionAdded = false;
    MQTTAgentSubscribeArgs_t *pxSubscribeArgs = (MQTTAgentSubscribeArgs_t *)pxCommandContext->pArgs;
    
    /* Store the result in the application defined context so the task that
     * initiated the subscribe can check the operation's status.  Also send the
     * status as the notification value.  These things are just done for
     * demonstration purposes. */
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;
    
    /* Check if the subscribe operation is a success. Only one topic is
     * subscribed by this demo. */
    if (pxReturnInfo->returnCode == MQTTSuccess) {
        /* Add subscription so that incoming publishes are routed to the application
         * callback. */
        xSubscriptionAdded = addSubscription((SubscriptionElement_t *)xGlobalMqttAgentContext.pIncomingCallbackContext,
                                             pxSubscribeArgs->pSubscribeInfo->pTopicFilter,
                                             pxSubscribeArgs->pSubscribeInfo->topicFilterLength,
                                             prvIncomingPublishCallback,
                                             NULL);
        
        if (xSubscriptionAdded == false) {
            LogError(("Failed to register an incoming publish callback for topic %.*s.",
                pxSubscribeArgs->pSubscribeInfo->topicFilterLength,
                pxSubscribeArgs->pSubscribeInfo->pTopicFilter));
        }
    }
    
    xTaskNotify(pxCommandContext->xTaskToNotify,
                (uint32_t)(pxReturnInfo->returnCode),
                eSetValueWithOverwrite);
}

/*-----------------------------------------------------------*/

static BaseType_t prvWaitForCommandAcknowledgment(uint32_t *pulNotifiedValue) {
    BaseType_t xReturn;
    
    /* Wait for this task to get notified, passing out the value it gets
     * notified with. */
    xReturn = xTaskNotifyWait(0,
                              0,
                              pulNotifiedValue,
                              pdMS_TO_TICKS(MS_TO_WAIT_FOR_NOTIFICATION));
    return xReturn;
}

/*-----------------------------------------------------------*/

static void prvIncomingPublishCallback(void *pvIncomingPublishCallbackContext,
                                       MQTTPublishInfo_t *pxPublishInfo) {
    static char cTerminatedString[STRING_BUFFER_LENGTH];
    
    (void)pvIncomingPublishCallbackContext;
    
    /* Create a message that contains the incoming MQTT payload to the logger,
     * terminating the string first. */
    if (pxPublishInfo->payloadLength < STRING_BUFFER_LENGTH) {
        memcpy((void *)cTerminatedString, pxPublishInfo->pPayload, pxPublishInfo->payloadLength);
        cTerminatedString[pxPublishInfo->payloadLength] = 0x00;
    } else {
        memcpy((void *)cTerminatedString, pxPublishInfo->pPayload, STRING_BUFFER_LENGTH);
        cTerminatedString[STRING_BUFFER_LENGTH - 1] = 0x00;
    }
    
    LogInfo(("Received incoming publish message %s", cTerminatedString));
}

/*-----------------------------------------------------------*/

static bool prvSubscribeToTopic(MQTTQoS_t xQoS,
                                char *pcTopicFilter) {
    MQTTStatus_t xCommandAdded;
    BaseType_t xCommandAcknowledged = pdFALSE;
    MQTTAgentSubscribeArgs_t xSubscribeArgs;
    MQTTSubscribeInfo_t xSubscribeInfo;
    static int32_t ulNextSubscribeMessageID = 0;
    MQTTAgentCommandContext_t xApplicationDefinedContext = {0UL};
    MQTTAgentCommandInfo_t xCommandParams = {0UL};
    
    /* Create a unique number of the subscribe that is about to be sent.  The number
     * is used as the command context and is sent back to this task as a notification
     * in the callback that executed upon receipt of the subscription acknowledgment.
     * That way this task can match an acknowledgment to a subscription. */
    xTaskNotifyStateClear(NULL);
    taskENTER_CRITICAL();
    {
        ulNextSubscribeMessageID++;
    }
    taskEXIT_CRITICAL();
    
    /* Complete the subscribe information.  The topic string must persist for
     * duration of subscription! */
    xSubscribeInfo.pTopicFilter = pcTopicFilter;
    xSubscribeInfo.topicFilterLength = (uint16_t)strlen(pcTopicFilter);
    xSubscribeInfo.qos = xQoS;
    xSubscribeArgs.pSubscribeInfo = &xSubscribeInfo;
    xSubscribeArgs.numSubscriptions = 1;
    
    /* Complete an application defined context associated with this subscribe message.
     * This gets updated in the callback function so the variable must persist until
     * the callback executes. */
    xApplicationDefinedContext.ulNotificationValue = ulNextSubscribeMessageID;
    xApplicationDefinedContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
    xApplicationDefinedContext.pArgs = (void *)&xSubscribeArgs;
    
    xCommandParams.blockTimeMs = MAX_COMMAND_SEND_BLOCK_TIME_MS;
    xCommandParams.cmdCompleteCallback = prvSubscribeCommandCallback;
    xCommandParams.pCmdCompleteCallbackContext = (void *)&xApplicationDefinedContext;
    
    /* Loop in case the queue used to communicate with the MQTT agent is full and
     * attempts to post to it time out.  The queue will not become full if the
     * priority of the MQTT agent task is higher than the priority of the task
     * calling this function. */
    LogInfo(("Sending subscribe request to agent for topic filter: %s with id %d",
        pcTopicFilter,
        (int)ulNextSubscribeMessageID));
    
    do {
        /* TODO: prvIncomingPublish as publish callback. */
        xCommandAdded = MQTTAgent_Subscribe(&xGlobalMqttAgentContext,
                                            &xSubscribeArgs,
                                            &xCommandParams);
    } while (xCommandAdded != MQTTSuccess);
    
    /* Wait for acks to the subscribe message - this is optional but done here
     * so the code below can check the notification sent by the callback matches
     * the ulNextSubscribeMessageID value set in the context above. */
    xCommandAcknowledged = prvWaitForCommandAcknowledgment(NULL);
    
    /* Check both ways the status was passed back just for demonstration
     * purposes. */
    if ((xCommandAcknowledged != pdTRUE) ||
        (xApplicationDefinedContext.xReturnStatus != MQTTSuccess)) {
        LogInfo(("Error or timed out waiting for ack to subscribe message topic %s",
            pcTopicFilter));
    } else {
        LogInfo(("Received subscribe ack for topic %s containing ID %d",
            pcTopicFilter,
            (int)xApplicationDefinedContext.ulNotificationValue));
    }
    
    return xCommandAcknowledged;
}
/*-----------------------------------------------------------*/

static void prvSimpleSubscribePublishTask(void *pvParameters) {
    uint32_t errcount = 0;
    MQTTPublishInfo_t xPublishInfo = {0UL};
    char payloadBuf[STRING_BUFFER_LENGTH];
    char taskName[STRING_BUFFER_LENGTH];
    MQTTAgentCommandContext_t xCommandContext;
    uint32_t ulNotification = 0U, ulValueToNotify = 0UL;
    MQTTStatus_t xCommandAdded;
    uint32_t ulTaskNumber = (uint32_t)pvParameters;
    MQTTQoS_t xQoS;
    TickType_t xTicksToDelay;
    MQTTAgentCommandInfo_t xCommandParams = {0UL};
    char *pcPubTopicBuffer = pubTopicBuf[ulTaskNumber];
    char *pcSubTopicBuffer = subTopicBuf[ulTaskNumber];
    static volatile uint32_t *ulPassCounts[] = {ulQoS0PassCount, ulQoS1PassCount};
    static volatile uint32_t *ulFailCounts[] = {ulQoS0FailCount, ulQoS1FailCount};
    
    /* Have different tasks use different QoS.  0 and 1.  2 can also be used
     * if supported by the broker. */
    xQoS = (MQTTQoS_t)(ulTaskNumber % 2UL);
    
    /* Create a unique name for this task from the task number that is passed into
     * the task using the task's parameter. */
    snprintf(taskName, STRING_BUFFER_LENGTH, "Publisher%d", (int)ulTaskNumber);
    
    /* Create a topic name for this task to publish to. */
    snprintf(pcSubTopicBuffer, STRING_BUFFER_LENGTH, DEMO_SUB_TOPIC);
    
    /* Subscribe to the same topic to which this task will publish.  That will
     * result in each published message being published from the server back to
     * the target. */
    prvSubscribeToTopic(xQoS, pcSubTopicBuffer);
    
    snprintf(pcPubTopicBuffer, STRING_BUFFER_LENGTH, DEMO_PUB_TOPIC);
    
    /* Configure the publish operation. */
    memset((void *)&xPublishInfo, 0x00, sizeof(xPublishInfo));
    xPublishInfo.qos = xQoS;
    xPublishInfo.pTopicName = pcPubTopicBuffer;
    xPublishInfo.topicNameLength = (uint16_t)strlen(pcPubTopicBuffer);
    xPublishInfo.pPayload = payloadBuf;
    
    /* Store the handler to this task in the command context so the callback
     * that executes when the command is acknowledged can send a notification
     * back to this task. */
    memset((void *)&xCommandContext, 0x00, sizeof(xCommandContext));
    xCommandContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
    
    xCommandParams.blockTimeMs = MAX_COMMAND_SEND_BLOCK_TIME_MS;
    xCommandParams.cmdCompleteCallback = prvPublishCommandCallback;
    xCommandParams.pCmdCompleteCallbackContext = &xCommandContext;
    
    /* For a finite number of publishes... */
    for (ulValueToNotify = 0UL; ulValueToNotify < PUBLISH_COUNT; ulValueToNotify++) {
        /* Create a payload to send with the publish message.  This contains
         * the task name and an incrementing number. */
        snprintf(payloadBuf,
                 STRING_BUFFER_LENGTH,
                 DEMO_PUB_MESSAGE);
        
        xPublishInfo.payloadLength = (uint16_t)strlen(payloadBuf);
        
        /* Also store the incrementing number in the command context so it can
         * be accessed by the callback that executes when the publish operation
         * is acknowledged. */
        xCommandContext.ulNotificationValue = ulValueToNotify;
        
        LogInfo(("Sending publish request to agent with message \"%s\" on topic \"%s\"",
            payloadBuf,
            pcPubTopicBuffer));
        
        /* To ensure ulNotification doesn't accidentally hold the expected value
         * as it is to be checked against the value sent from the callback.. */
        ulNotification = ~ulValueToNotify;
        
        xCommandAdded = MQTTAgent_Publish(&xGlobalMqttAgentContext,
                                          &xPublishInfo,
                                          &xCommandParams);
        configASSERT(xCommandAdded == MQTTSuccess);
        
        /* For QoS 1 and 2, wait for the publish acknowledgment.  For QoS0,
         * wait for the publish to be sent. */
        LogInfo(("Task %s waiting for publish %d to complete.",
            taskName,
            ulValueToNotify));
        prvWaitForCommandAcknowledgment(&ulNotification);
        
        /* The value received by the callback that executed when the publish was
         * acked came from the context passed into MQTTAgent_Publish() above, so
         * should match the value set in the context above. */
        if (ulNotification == ulValueToNotify) {
            errcount = 0;
            (ulPassCounts[xQoS][ulTaskNumber >> 1])++;
            LogInfo(("Rx'ed %s from Tx to %s (P%d:F%d).",
                (xQoS == 0) ? "completion notification for QoS0 publish" : "ack for QoS1 publish",
                pcPubTopicBuffer,
                ulPassCounts[xQoS][ulTaskNumber >> 1],
                ulFailCounts[xQoS][ulTaskNumber >> 1]));
        } else {
            errcount++;
            (ulFailCounts[xQoS][ulTaskNumber >> 1])++;
            LogError(("Timed out Rx'ing %s from Tx to %s (P%d:F%d)",
                (xQoS == 0) ? "completion notification for QoS0 publish" : "ack for QoS1 publish",
                pcPubTopicBuffer,
                ulPassCounts[xQoS][ulTaskNumber >> 1],
                ulFailCounts[xQoS][ulTaskNumber >> 1]));
            
            if (errcount >= MQTT_AGENT_MAX_OUTSTANDING_ACKS - 1) {
                LogError(("Failed to get ACKs, terminate mqtt command loop"));
                errcount = 0;
                MQTTAgent_Terminate(&xGlobalMqttAgentContext, NULL);
            }
        }
        
        /* Add a little randomness into the delay so the tasks don't remain
         * in lockstep. */
        xTicksToDelay = pdMS_TO_TICKS(DELAY_BETWEEN_PUBLISH_OPERATIONS_MS) +
            (bsp_rng_read() % 0xff);
        vTaskDelay(xTicksToDelay);
    }
    
    /* Delete the task if it is complete. */
    LogInfo(("Task %s completed.", taskName));
    vTaskDelete(NULL);
    (void)xCommandAdded;
}

