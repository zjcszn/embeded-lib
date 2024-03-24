/*
 * FreeRTOS V202212.01
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * This demo creates multiple tasks, all of which use the MQTT agent API to
 * communicate with an MQTT broker through the same MQTT connection.
 *
 * This file contains the initial task created after the TCP/IP stack connects
 * to the network.  The task:
 *
 * 1) Connects to the MQTT broker.
 * 2) Creates the other demo tasks, in accordance with the #defines set in
 *    core_mqtt_agent_config.h.  For example, if core_mqtt_agent_config.h contains the following
 *    setting:
 *
 *    #define democonfigNUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE 3
 *
 *    then the initial task will create three instances of the task
 *    implemented in simple_sub_pub_demo.c.  See the comments at the top
 *    of that file for more information.
 *
 * 3) After creating the demo tasks the initial task could create the MQTT
 *    agent task.  However, as it has no other operations to perform, rather
 *    than create the MQTT agent as a separate task the initial task just calls
 *    the agent's implementing function - effectively turning itself into the
 *    MQTT agent.
 */

#define LOG_TAG "MQTT"
#define LOG_LVL ELOG_LVL_INFO

/* Standard includes. */
#include <string.h>
#include <stdint.h>

/* bsp includes */
#include "bsp_rng.h"
#include "bsp_ethif.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "arch/sys_arch.h"
#include "event_groups.h"
#include "queue.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "lwip/sockets.h"

/* Log includes. */
#include "corelog.h"

/* MQTT library includes. */
#include "core_mqtt.h"

/* MQTT agent include. */
#include "core_mqtt_agent.h"

/* MQTT Agent ports. */
#include "freertos_agent_message.h"
#include "freertos_command_pool.h"

/* Exponential backoff retry include. */
#include "backoff_algorithm.h"

/* Subscription manager header include. */
#include "subscription_manager.h"



/* Transport interface include. */
#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
#include "transport_mbedtls.h"
#else
#include "transport_plaintext.h"
#endif

/* This demo uses compile time options to select the demo tasks to created.
 * Ensure the compile time options are defined.  These should be defined in
 * core_mqtt_agent_config.h. */
#if !defined( NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE ) || (NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE < 1)
#error Please set NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE to the number of tasks to create in vStartSimpleSubscribePublishTask().
#endif

#if (NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE > 0) && !defined( SIMPLE_SUB_PUB_TASK_STACK_SIZE )
#error Please define SIMPLE_SUB_PUB_TASK_STACK_SIZE in core_mqtt_agent_config.h to set the stack size (in words, not bytes) for the tasks created by vStartSimpleSubscribePublishTask().
#endif

/* Compile time error for some undefined configs, and provide default values
 * for others. */
#ifndef MQTT_BROKER_ENDPOINT
#error "Please define MQTT_BROKER_ENDPOINT in core_mqtt_agent_config.h."
#endif

#ifndef MQTT_CLIENT_IDENTIFIER

/**
 * @brief The MQTT client identifier used in this example.  Each client identifier
 * must be unique so edit as required to ensure no two clients connecting to the
 * same broker use the same client identifier.  Using a #define is for convenience
 * of demonstration only - production devices should use something unique to the
 * device that can be read from software - such as a production serial number.
 */
#error  "Please define MQTT_CLIENT_IDENTIFIER in core_mqtt_agent_config.h to something unique for this device."
#endif

#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
#ifndef MQTT_ROOT_CA_PEM
#error "Please define Root CA certificate of the MQTT broker(MQTT_ROOT_CA_PEM) in core_mqtt_agent_config.h."
#endif

/* If no username is defined, then a client certificate/key is required. */
#ifndef MQTT_CLIENT_USERNAME

/*
 *!!! Please note democonfigCLIENT_PRIVATE_KEY_PEM in used for
 *!!! convenience of demonstration only.  Production devices should
 *!!! store keys securely, such as within a secure element.
 */

#ifndef MQTT_CLIENT_CERTIFICATE_PEM
#error "Please define client certificate(MQTT_CLIENT_CERTIFICATE_PEM) in core_mqtt_agent_config.h."
#endif
#ifndef MQTT_CLIENT_PRIVATE_KEY_PEM
#error "Please define client private key(MQTT_CLIENT_PRIVATE_KEY_PEM) in core_mqtt_agent_config.h."
#endif
#else

/* If a username is defined, a client password also would need to be defined for
 * client authentication. */
#ifndef MQTT_CLIENT_PASSWORD
#error "Please define client password(MQTT_CLIENT_PASSWORD) in core_mqtt_agent_config.h for client authentication based on username/password."
#endif
#endif /* ifndef MQTT_CLIENT_USERNAME */

#ifndef MQTT_BROKER_PORT
#define MQTT_BROKER_PORT    ( 8883 )
#endif
#else /* if defined( MQTT_USE_TLS ) && ( MQTT_USE_TLS == 1 ) */
#ifndef MQTT_BROKER_PORT
#define MQTT_BROKER_PORT    ( 1883 )
#endif
#endif /* if defined( MQTT_USE_TLS ) && ( MQTT_USE_TLS == 1 ) */


/**
 * @brief Dimensions the buffer used to serialize and deserialize MQTT packets.
 * @note Specified in bytes.  Must be large enough to hold the maximum
 * anticipated MQTT payload.
 */
#ifndef MQTT_AGENT_NETWORK_BUFFER_SIZE
#define MQTT_AGENT_NETWORK_BUFFER_SIZE          ( 4096U )
#endif

/**
 * @brief The length of the queue used to hold commands for the agent.
 */
#ifndef MQTT_AGENT_COMMAND_QUEUE_LENGTH
#define MQTT_AGENT_COMMAND_QUEUE_LENGTH         ( 10U )
#endif


/**
 * These configuration settings are required to run the demo.
 */

/**
 * @brief Timeout for receiving CONNACK after sending an MQTT CONNECT packet.
 * Defined in milliseconds.
 */
#define MQTT_CONNACK_RECV_TIMEOUT_MS            ( 1000U )

/**
 * @brief The maximum number of retries for network operation with server.
 */
#define RETRY_MAX_ATTEMPTS                      ( 5U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying failed operation
 *  with server.
 */
#define RETRY_MAX_BACKOFF_DELAY_MS              ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for network operation retry
 * attempts.
 */
#define RETRY_BACKOFF_BASE_MS                   ( 500U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 *  between two Control Packets.
 *
 *  It is the responsibility of the Client to ensure that the interval between
 *  Control Packets being sent does not exceed the this Keep Alive value. In the
 *  absence of sending any other Control Packets, the Client MUST send a
 *  PINGREQ Packet.
 *//*_RB_ Move to be the responsibility of the agent. */
#define KEEP_ALIVE_INTERVAL_SECONDS             ( 60U )

/**
 * @brief Socket send and receive timeouts to use.  Specified in milliseconds.
 */
#define TRANSPORT_SEND_RECV_TIMEOUT_MS          ( 750 )

/**
 * @brief Used to convert times to/from ticks and milliseconds.
 */
#define MILLISECONDS_PER_SECOND                 ( 1000U )
#define MILLISECONDS_PER_TICK                   ( MILLISECONDS_PER_SECOND / configTICK_RATE_HZ )

#define SOCKETS_START_DELETION                  ( 0x01 )
#define SOCKETS_COMPLETE_DELETION               ( 0x02 )


/**
 * Provide default values for undefined configuration settings.
 */
#ifndef MQTT_OS_NAME
#define MQTT_OS_NAME    "FreeRTOS"
#endif

#ifndef MQTT_OS_VERSION
#define MQTT_OS_VERSION    tskKERNEL_VERSION_NUMBER
#endif

#ifndef MQTT_HARDWARE_PLATFORM_NAME
#define MQTT_HARDWARE_PLATFORM_NAME    "WinSim"
#endif

#ifndef MQTT_LIB
#include "core_mqtt.h" /* Include coreMQTT header for MQTT_LIBRARY_VERSION macro. */
#define MQTT_LIB    "core-mqtt@"MQTT_LIBRARY_VERSION
#endif

typedef void (*Network_RxCallback_t)(Socket_t sock);

struct MQTT_WakeupContext {
  Network_RxCallback_t rxCallback;
  EventGroupHandle_t rxEventGroup;
};
typedef struct MQTT_WakeupContext *MQTT_WakeupContext_t;


/*-----------------------------------------------------------*/

/**
 * @brief Each compilation unit that consumes the NetworkContext must define it.
 * It should contain a single pointer to the type of your desired transport.
 * When using multiple transports in the same compilation unit, define this pointer as void *.
 *
 * @note Transport stacks are defined in FreeRTOS-Plus/Source/Application-Protocols/network_transport.
 */

/*-----------------------------------------------------------*/

/**
 * @brief Initializes an MQTT context, including transport interface and
 * network buffer.
 *
 * @return `MQTTSuccess` if the initialization succeeds, else `MQTTBadParameter`.
 */
static MQTTStatus_t prvMQTTInit(void);

/**
 * @brief Sends an MQTT Connect packet over the already connected TCP socket.
 *
 * @param[in] pxMQTTContext MQTT context pointer.
 * @param[in] xCleanSession If a clean session should be established.
 *
 * @return `MQTTSuccess` if connection succeeds, else appropriate error code
 * from MQTT_Connect.
 */
static MQTTStatus_t prvMQTTConnect(bool xCleanSession);

/**
 * @brief Connect a TCP socket to the MQTT broker.
 *
 * @param[in] pxNetworkContext Network context.
 *
 * @return `pdPASS` if connection succeeds, else `pdFAIL`.
 */
static BaseType_t prvSocketConnect(NetworkContext_t *pxNetworkContext);

/**
 * @brief Disconnect a TCP connection.
 *
 * @param[in] pxNetworkContext Network context.
 *
 * @return `pdPASS` if disconnect succeeds, else `pdFAIL`.
 */
static BaseType_t prvSocketDisconnect(NetworkContext_t *pxNetworkContext);

/**
 * @brief Callback executed when there is activity on the TCP socket that is
 * connected to the MQTT broker.  If there are no messages in the MQTT agent's
 * command queue then the callback send a message to ensure the MQTT agent
 * task unblocks and can therefore process whatever is necessary on the socket
 * (if anything) as quickly as possible.
 *
 * @param[in] pxSocket Socket with data, unused.
 */
static void prvMQTTClientSocketWakeupCallback(Socket_t pxSocket);

/**
 * @brief Fan out the incoming publishes to the callbacks registered by different
 * tasks. If there are no callbacks registered for the incoming publish, it will be
 * passed to the unsolicited publish handler.
 *
 * @param[in] pMqttAgentContext Agent context.
 * @param[in] packetId Packet ID of publish.
 * @param[in] pxPublishInfo Info of incoming publish.
 */
static void prvIncomingPublishCallback(MQTTAgentContext_t *pMqttAgentContext,
                                       uint16_t packetId,
                                       MQTTPublishInfo_t *pxPublishInfo);

/**
 * @brief Function to attempt to resubscribe to the topics already present in the
 * subscription list.
 *
 * This function will be invoked when this demo requests the broker to
 * reestablish the session and the broker cannot do so. This function will
 * enqueue commands to the MQTT Agent queue and will be processed once the
 * command loop starts.
 *
 * @return `MQTTSuccess` if adding subscribes to the command queue succeeds, else
 * appropriate error code from MQTTAgent_Subscribe.
 * */
static MQTTStatus_t prvHandleResubscribe(void);

/**
 * @brief Passed into MQTTAgent_Subscribe() as the callback to execute when the
 * broker ACKs the SUBSCRIBE message. This callback implementation is used for
 * handling the completion of resubscribes. Any topic filter failed to resubscribe
 * will be removed from the subscription list.
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pxCommandContext Context of the initial command.
 * @param[in] pxReturnInfo The result of the command.
 */
static void prvSubscriptionCommandCallback(void *pxCommandContext,
                                           MQTTAgentReturnInfo_t *pxReturnInfo);

/**
 * @brief Task used to run the MQTT agent.  In this example the first task that
 * is created is responsible for creating all the other demo tasks.  Then,
 * rather than create prvMQTTAgentTask() as a separate task, it simply calls
 * prvMQTTAgentTask() to become the agent task itself.
 *
 * This task calls MQTTAgent_CommandLoop() in a loop, until MQTTAgent_Terminate()
 * is called. If an error occurs in the command loop, then it will reconnect the
 * TCP and MQTT connections.
 *
 * @param[in] pvParameters Parameters as passed at the time of task creation. Not
 * used in this example.
 */
static void prvMQTTAgentTask(void *pvParameters);

/**
 * @brief The main task used in the MQTT demo.
 *
 * This task creates the network connection and all other demo tasks. Then,
 * rather than create prvMQTTAgentTask() as a separate task, it simply calls
 * prvMQTTAgentTask() to become the agent task itself.
 *
 * @param[in] pvParameters Parameters as passed at the time of task creation. Not
 * used in this example.
 */
static void prvConnectAndCreateAgentTasks(void *pvParameters);

/**
 * @brief The timer query function provided to the MQTT context.
 *
 * @return Time in milliseconds.
 */
static uint32_t prvGetTimeMs(void);

/**
 * @brief Connects a TCP socket to the MQTT broker, then creates and MQTT
 * connection to the same.
 */
static void prvConnectToMQTTBroker(void);

/*
 * Functions that start the tasks demonstrated by this project.
 */

extern void vStartSimpleSubscribePublishTask(uint32_t ulTaskNumber,
                                             configSTACK_DEPTH_TYPE uxStackSize,
                                             UBaseType_t uxPriority);

/*-----------------------------------------------------------*/

/**
 * @brief The network context used by the MQTT library transport interface.
 * See https://www.freertos.org/network-interface.html
 */
static NetworkContext_t xNetworkContext;

#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
/**
 * @brief The parameters for the network context using a TLS channel.
 */
static TlsTransportParams_t xTlsTransportParams;
#else
/**
 * @brief The parameters for the network context using a non-encrypted channel.
 */
static PlaintextTransportParams_t xPlaintextTransportParams;
#endif

/**
 * @brief Global entry time into the application to use as a reference timestamp
 * in the #prvGetTimeMs function. #prvGetTimeMs will always return the difference
 * between the current time and the global entry time. This will reduce the chances
 * of overflow for the 32 bit unsigned integer used for holding the timestamp.
 */
static uint32_t ulGlobalEntryTimeMs;

MQTTAgentContext_t xGlobalMqttAgentContext;

static uint8_t xNetworkBuffer[MQTT_AGENT_NETWORK_BUFFER_SIZE];

static MQTTAgentMessageContext_t xCommandQueue;

/**
 * @brief The global array of subscription elements.
 *
 * @note No thread safety is required to this array, since the updates the array
 * elements are done only from one task at a time. The subscription manager
 * implementation expects that the array of the subscription elements used for
 * storing subscriptions to be initialized to 0. As this is a global array, it
 * will be initialized to 0 by default.
 */
SubscriptionElement_t xGlobalSubscriptionList[SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS];

/*-----------------------------------------------------------*/

/*
 * @brief Create the task that demonstrates the MQTT Connection sharing demo.
 */
void vStartMQTTAgent(void) {
    /* prvConnectAndCreateDemoTasks() connects to the MQTT broker, creates the
     * tasks that will interact with the broker via the MQTT agent, then turns
     * itself into the MQTT agent task. */
    xTaskCreate(prvConnectAndCreateAgentTasks, /* Function that implements the task. */
                "ConnectManager",             /* Text name for the task - only used for debugging. */
                MQTT_DEMO_THREAD_STACK_SIZE,  /* Size of stack (in words, not bytes) to allocate for the task. */
                NULL,                         /* Optional - task parameter - not used in this case. */
                tskIDLE_PRIORITY + 1,         /* Task priority, must be between 0 and configMAX_PRIORITIES - 1. */
                NULL);                        /* Optional - used to pass out a handle to the created task. */
}

static void vTaskRxSelect(void *param) {
    LWIP_NETCONN_THREAD_SEM_ALLOC();
    
    NetworkContext_t *pxNetworkContext = (NetworkContext_t *)param;
    MQTT_WakeupContext_t mqtt_wakeup = (MQTT_WakeupContext_t)pxNetworkContext->pUserContext;
    
    int s = pxNetworkContext->tcpSocket;
    struct timeval timeout;
    
    fd_set userfds;
    fd_set readfds;
    FD_ZERO(&userfds);
    FD_ZERO(&readfds);
    
    timeout.tv_sec = MQTT_RECV_WAKEUP_WAIT_SEC;
    timeout.tv_usec = 0;
    FD_SET(s, &userfds);
    
    while (1) {
        readfds = userfds;
        if (lwip_select(s + 1, &readfds, NULL, NULL, &timeout) == -1) {
            break;
        }
        
        if (FD_ISSET(s, &readfds)) {
            mqtt_wakeup->rxCallback(s);
        }
        
        if (xEventGroupWaitBits(mqtt_wakeup->rxEventGroup, SOCKETS_START_DELETION, pdTRUE, pdTRUE, 0) ==
            SOCKETS_START_DELETION) {
            /* Inform the main task that the event has been received. */
            (void)xEventGroupSetBits(mqtt_wakeup->rxEventGroup, SOCKETS_COMPLETE_DELETION);
            break;
        }
    }
    
    LWIP_NETCONN_THREAD_SEM_FREE();
    vTaskDelete(NULL);
}

static void prvRxSelectSet(NetworkContext_t *pxNetworkContext,
                           void(*rx_callback)(int)) {
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    uint16_t xStackDepth = MQTT_RECV_WAKEUP_THREAD_STACK_SIZE;
    
    configASSERT(rx_callback != NULL);
    
    MQTT_WakeupContext_t mqtt_wakeup = pvPortMalloc(sizeof(struct MQTT_WakeupContext));
    configASSERT(mqtt_wakeup != NULL);
    
    mqtt_wakeup->rxCallback = rx_callback;
    mqtt_wakeup->rxEventGroup = xEventGroupCreate();
    configASSERT(mqtt_wakeup->rxEventGroup != NULL);
    
    pxNetworkContext->pUserContext = mqtt_wakeup;
    
    xReturned = xTaskCreate(vTaskRxSelect,  /* pvTaskCode */
                            "mqtt wakeup",  /* pcName */
                            xStackDepth,    /* usStackDepth */
                            pxNetworkContext,   /* pvParameters */
                            1,  /* uxPriority */
                            &xHandle);  /* pxCreatedTask */
    
    configASSERT(xReturned == pdPASS);
    configASSERT(xHandle != NULL);
    
    (void)xReturned;
}

static void prvRxSelectClear(NetworkContext_t *pxNetworkContext) {
    /* Inform the vTaskRxSelect to delete itself. */
    MQTT_WakeupContext_t mqtt_wakeup = (MQTT_WakeupContext_t)pxNetworkContext->pUserContext;
    xEventGroupSetBits(mqtt_wakeup->rxEventGroup, SOCKETS_START_DELETION);
    
    /* Wait for the task to delete itself. */
    while (xEventGroupWaitBits(mqtt_wakeup->rxEventGroup,
                               SOCKETS_COMPLETE_DELETION,
                               pdTRUE,
                               pdTRUE,
                               pdMS_TO_TICKS(MQTT_RECV_WAKEUP_WAIT_SEC * 1000)) !=
        SOCKETS_COMPLETE_DELETION) {
        /* Continue waiting for the task to delete itself. */
    }
    
    /* Reset the handle of the task to NULL. */
    
    /* Delete the event group. */
    vEventGroupDelete(mqtt_wakeup->rxEventGroup);
    
    /* Remove the reference to the callback. */
    vPortFree(mqtt_wakeup);
    pxNetworkContext->pUserContext = NULL;
}

/*-----------------------------------------------------------*/

static MQTTStatus_t prvMQTTInit(void) {
    TransportInterface_t xTransport;
    MQTTStatus_t xReturn;
    MQTTFixedBuffer_t xFixedBuffer = {.pBuffer = xNetworkBuffer, .size = MQTT_AGENT_NETWORK_BUFFER_SIZE};

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    static uint8_t staticQueueStorageArea[MQTT_AGENT_COMMAND_QUEUE_LENGTH * sizeof(MQTTAgentCommand_t *)];
    static StaticQueue_t staticQueueStructure;
#endif
    
    MQTTAgentMessageInterface_t messageInterface =
        {
            .pMsgCtx        = NULL,
            .send           = Agent_MessageSend,
            .recv           = Agent_MessageReceive,
            .getCommand     = Agent_GetCommand,
            .releaseCommand = Agent_ReleaseCommand
        };
    
    LogDebug(("Creating command queue."));

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    xCommandQueue.queue = xQueueCreateStatic(MQTT_AGENT_COMMAND_QUEUE_LENGTH,
                                             sizeof(MQTTAgentCommand_t *),
                                             staticQueueStorageArea,
                                             &staticQueueStructure);
#else
    xCommandQueue.queue = xQueueCreate(MQTT_AGENT_COMMAND_QUEUE_LENGTH,
                                       sizeof(MQTTAgentCommand_t *));
#endif
    
    configASSERT(xCommandQueue.queue);
    messageInterface.pMsgCtx = &xCommandQueue;
    
    /* Initialize the task pool. */
    Agent_InitializePool();
    
    /* Fill in Transport Interface send and receive function pointers. */
    xTransport.pNetworkContext = &xNetworkContext;
#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
    xTransport.send = Network_TLS_Send;
    xTransport.recv = Network_TLS_Recv;
    xTransport.writev = NULL;
#else
    xTransport.send = Network_Plaintext_Send;
    xTransport.recv = Network_Plaintext_Recv;
    xTransport.writev = NULL;
#endif
    
    /* Initialize MQTT library. */
    xReturn = MQTTAgent_Init(&xGlobalMqttAgentContext,
                             &messageInterface,
                             &xFixedBuffer,
                             &xTransport,
                             prvGetTimeMs,
                             prvIncomingPublishCallback,
        /* Context to pass into the callback. Passing the pointer to subscription array. */
                             xGlobalSubscriptionList);
    
    return xReturn;
}

/*-----------------------------------------------------------*/

static MQTTStatus_t prvMQTTConnect(bool xCleanSession) {
    MQTTStatus_t xResult;
    MQTTConnectInfo_t xConnectInfo;
    bool xSessionPresent = false;
    
    /* Many fields are not used in this demo so start with everything at 0. */
    memset(&xConnectInfo, 0x00, sizeof(xConnectInfo));
    
    /* Start with a clean session i.e. direct the MQTT broker to discard any
     * previous session data. Also, establishing a connection with clean session
     * will ensure that the broker does not store any data when this client
     * gets disconnected. */
    xConnectInfo.cleanSession = xCleanSession;
    
    /* The client identifier is used to uniquely identify this MQTT client to
     * the MQTT broker. In a production device the identifier can be something
     * unique, such as a device serial number. */
    xConnectInfo.pClientIdentifier = MQTT_CLIENT_IDENTIFIER;
    xConnectInfo.clientIdentifierLength = (uint16_t)strlen(MQTT_CLIENT_IDENTIFIER);
    
    /* Set MQTT keep-alive period. It is the responsibility of the application
     * to ensure that the interval between Control Packets being sent does not
     * exceed the Keep Alive value. In the absence of sending any other Control
     * Packets, the Client MUST send a PINGREQ Packet.  This responsibility will
     * be moved inside the agent. */
    xConnectInfo.keepAliveSeconds = KEEP_ALIVE_INTERVAL_SECONDS;

#if defined( MQTT_CLIENT_USERNAME )
    /* If not connecting to AWS IoT Core, send the username without modification. */
    xConnectInfo.pUserName = MQTT_CLIENT_USERNAME;
    xConnectInfo.userNameLength = (uint16_t)strlen(MQTT_CLIENT_USERNAME);
    xConnectInfo.pPassword = MQTT_CLIENT_PASSWORD;
    xConnectInfo.passwordLength = (uint16_t)strlen(MQTT_CLIENT_PASSWORD);
#endif /* defined( democonfigCLIENT_USERNAME ) */
    
    /* Send MQTT CONNECT packet to broker. MQTT's Last Will and Testament feature
     * is not used in this demo, so it is passed as NULL. */
    xResult = MQTT_Connect(&(xGlobalMqttAgentContext.mqttContext),
                           &xConnectInfo,
                           NULL,
                           MQTT_CONNACK_RECV_TIMEOUT_MS,
                           &xSessionPresent);
    
    LogInfo(("Session present: %d\n", xSessionPresent));
    
    /* Resume a session if desired. */
    if ((xResult == MQTTSuccess) && (xCleanSession == false)) {
        xResult = MQTTAgent_ResumeSession(&xGlobalMqttAgentContext, xSessionPresent);
        
        /* Resubscribe to all the subscribed topics. */
        if ((xResult == MQTTSuccess) && (xSessionPresent == false)) {
            xResult = prvHandleResubscribe();
        }
    }
    
    return xResult;
}

/*-----------------------------------------------------------*/

static MQTTStatus_t prvHandleResubscribe(void) {
    MQTTStatus_t xResult = MQTTBadParameter;
    uint32_t ulIndex = 0U;
    uint16_t usNumSubscriptions = 0U;
    
    /* These variables need to stay in scope until command completes. */
    static MQTTAgentSubscribeArgs_t xSubArgs = {0};
    static MQTTSubscribeInfo_t xSubInfo[SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS] = {0};
    static MQTTAgentCommandInfo_t xCommandParams = {0};
    
    /* Loop through each subscription in the subscription list and add a subscribe
     * command to the command queue. */
    for (ulIndex = 0U; ulIndex < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS; ulIndex++) {
        /* Check if there is a subscription in the subscription list. This demo
         * doesn't check for duplicate subscriptions. */
        if (xGlobalSubscriptionList[ulIndex].usFilterStringLength != 0) {
            xSubInfo[usNumSubscriptions].pTopicFilter = xGlobalSubscriptionList[ulIndex].pcSubscriptionFilterString;
            xSubInfo[usNumSubscriptions].topicFilterLength = xGlobalSubscriptionList[ulIndex].usFilterStringLength;
            
            /* QoS1 is used for all the subscriptions in this demo. */
            xSubInfo[usNumSubscriptions].qos = MQTTQoS1;
            
            LogInfo(("Resubscribe to the topic %.*s will be attempted.",
                xSubInfo[usNumSubscriptions].topicFilterLength,
                xSubInfo[usNumSubscriptions].pTopicFilter));
            
            usNumSubscriptions++;
        }
    }
    
    if (usNumSubscriptions > 0U) {
        xSubArgs.pSubscribeInfo = xSubInfo;
        xSubArgs.numSubscriptions = usNumSubscriptions;
        
        /* The block time can be 0 as the command loop is not running at this point. */
        xCommandParams.blockTimeMs = 0U;
        xCommandParams.cmdCompleteCallback = (MQTTAgentCommandCallback_t)prvSubscriptionCommandCallback;
        xCommandParams.pCmdCompleteCallbackContext = (void *)&xSubArgs;
        
        /* Enqueue subscribe to the command queue. These commands will be processed only
         * when command loop starts. */
        xResult = MQTTAgent_Subscribe(&xGlobalMqttAgentContext, &xSubArgs, &xCommandParams);
    } else {
        /* Mark the resubscribe as success if there is nothing to be subscribed. */
        xResult = MQTTSuccess;
    }
    
    if (xResult != MQTTSuccess) {
        LogError(("Failed to enqueue the MQTT subscribe command. xResult=%s.",
            MQTT_Status_strerror(xResult)));
    }
    
    return xResult;
}

/*-----------------------------------------------------------*/

static void prvSubscriptionCommandCallback(void *pxCommandContext,
                                           MQTTAgentReturnInfo_t *pxReturnInfo) {
    size_t lIndex = 0;
    MQTTAgentSubscribeArgs_t *pxSubscribeArgs = (MQTTAgentSubscribeArgs_t *)pxCommandContext;
    
    /* If the return code is success, no further action is required as all the topic filters
     * are already part of the subscription list. */
    if (pxReturnInfo->returnCode != MQTTSuccess) {
        /* Check through each of the suback codes and determine if there are any failures. */
        for (lIndex = 0; lIndex < pxSubscribeArgs->numSubscriptions; lIndex++) {
            /* This demo doesn't attempt to resubscribe in the event that a SUBACK failed. */
            if (pxReturnInfo->pSubackCodes[lIndex] == MQTTSubAckFailure) {
                LogError(("Failed to resubscribe to topic %.*s.",
                    pxSubscribeArgs->pSubscribeInfo[lIndex].topicFilterLength,
                    pxSubscribeArgs->pSubscribeInfo[lIndex].pTopicFilter));
                /* Remove subscription callback for unsubscribe. */
                removeSubscription(xGlobalSubscriptionList,
                                   pxSubscribeArgs->pSubscribeInfo[lIndex].pTopicFilter,
                                   pxSubscribeArgs->pSubscribeInfo[lIndex].topicFilterLength);
            }
        }
        
        /* Hit an assert as some of the tasks won't be able to proceed correctly without
         * the subscriptions. This logic will be updated with exponential backoff and retry.  */
        configASSERT(pdTRUE);
    }
}

/*-----------------------------------------------------------*/

static BaseType_t prvSocketConnect(NetworkContext_t *pxNetworkContext) {
    BaseType_t xConnected = pdPASS;
    BackoffAlgorithmStatus_t xBackoffAlgStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t xReconnectParams = {0};
    uint16_t usNextRetryBackOff = 0U;

#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
    TlsTransportStatus_t xNetworkStatus = TLS_TRANSPORT_CONNECT_FAILURE;
    NetworkCredentials_t xNetworkCredentials = {0};

#if defined( MQTT_ALPN_PROTOCOL_NAME )
    xNetworkCredentials.pAlpnProtos = MQTT_ALPN_PROTOCOL_NAME;
#else
    xNetworkCredentials.pAlpnProtos = NULL;
#endif
    
    /* Set the credentials for establishing a TLS connection. */
    xNetworkCredentials.pRootCa = (const unsigned char *)MQTT_ROOT_CA_PEM;
    xNetworkCredentials.rootCaSize = sizeof(MQTT_ROOT_CA_PEM);
#ifdef MQTT_CLIENT_CERTIFICATE_PEM
    xNetworkCredentials.pClientCert = ( const unsigned char * ) CLIENT_CERTIFICATE_PEM;
    xNetworkCredentials.clientCertSize = sizeof( CLIENT_CERTIFICATE_PEM );
    xNetworkCredentials.pPrivateKey = ( const unsigned char * ) CLIENT_PRIVATE_KEY_PEM;
    xNetworkCredentials.privateKeySize = sizeof( CLIENT_PRIVATE_KEY_PEM );
#endif
    xNetworkCredentials.disableSni = MQTT_DISABLE_SNI;
#else /* if defined( MQTT_USE_TLS ) && ( MQTT_USE_TLS == 1 ) */
    PlaintextTransportStatus_t xNetworkStatus = PLAINTEXT_TRANSPORT_CONNECT_FAILURE;
#endif /* if defined( MQTT_USE_TLS ) && ( MQTT_USE_TLS == 1 ) */
    
    /* We will use a retry mechanism with an exponential backoff mechanism and
     * jitter.  That is done to prevent a fleet of IoT devices all trying to
     * reconnect at exactly the same time should they become disconnected at
     * the same time. We initialize reconnect attempts and interval here. */
    BackoffAlgorithm_InitializeParams(&xReconnectParams,
                                      RETRY_BACKOFF_BASE_MS,
                                      RETRY_MAX_BACKOFF_DELAY_MS,
                                      RETRY_MAX_ATTEMPTS);
    
    /* Attempt to connect to MQTT broker. If connection fails, retry after a
     * timeout. Timeout value will exponentially increase until the maximum
     * number of attempts are reached.
     */
    do {
        /* Establish a TCP connection with the MQTT broker. This example connects to
         * the MQTT broker as specified in MQTT_BROKER_ENDPOINT and
         * MQTT_BROKER_PORT at the top of this file. */
#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
        LogInfo(("Creating a TLS connection to %s:%d.",
            MQTT_BROKER_ENDPOINT,
            MQTT_BROKER_PORT));
        xNetworkStatus = Network_TLS_Connect(pxNetworkContext,
                                             MQTT_BROKER_ENDPOINT,
                                             MQTT_BROKER_PORT,
                                             &xNetworkCredentials,
                                             TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                             TRANSPORT_SEND_RECV_TIMEOUT_MS);
        xConnected = (xNetworkStatus == TLS_TRANSPORT_SUCCESS) ? pdPASS : pdFAIL;
#else /* if defined( MQTT_USE_TLS ) && ( MQTT_USE_TLS == 1 ) */
        LogInfo(("Creating a TCP connection to %s:%d.",
            MQTT_BROKER_ENDPOINT,
            MQTT_BROKER_PORT));
        xNetworkStatus = Network_Plaintext_Connect(pxNetworkContext,
                                                   MQTT_BROKER_ENDPOINT,
                                                   MQTT_BROKER_PORT,
                                                   TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                                   TRANSPORT_SEND_RECV_TIMEOUT_MS);
        xConnected = (xNetworkStatus == PLAINTEXT_TRANSPORT_SUCCESS) ? pdPASS : pdFAIL;
#endif /* if defined( democonfigUSE_TLS ) && ( democonfigUSE_TLS == 1 ) */
        
        if (!xConnected) {
            /* Get back-off value (in milliseconds) for the next connection retry. */
            xBackoffAlgStatus = BackoffAlgorithm_GetNextBackoff(&xReconnectParams, bsp_rng_read(), &usNextRetryBackOff);
            
            if (xBackoffAlgStatus == BackoffAlgorithmSuccess) {
                LogWarn(("Connection to the broker failed. "
                         "Retrying connection in %hu ms.",
                    usNextRetryBackOff));
                vTaskDelay(pdMS_TO_TICKS(usNextRetryBackOff));
            }
        }
        
        if (xBackoffAlgStatus == BackoffAlgorithmRetriesExhausted) {
            LogError(("Connection to the broker failed, all attempts exhausted."));
        }
    } while ((xConnected != pdPASS) && (xBackoffAlgStatus == BackoffAlgorithmSuccess));
    
    
    /* Set the socket wakeup callback and ensure the read block time. */
    if (xConnected) {
        prvRxSelectSet(pxNetworkContext, prvMQTTClientSocketWakeupCallback);
    }
    
    return xConnected;
}

/*-----------------------------------------------------------*/

static BaseType_t prvSocketDisconnect(NetworkContext_t *pxNetworkContext) {
    BaseType_t xDisconnected = pdFAIL;
    
    /* Set the wakeup callback to NULL since the socket will disconnect. */
    prvRxSelectClear(pxNetworkContext);

#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
    LogInfo(("Disconnecting TLS connection.\n"));
    Network_TLS_Disconnect(pxNetworkContext);
    xDisconnected = pdPASS;
#else
    LogInfo(("Disconnecting TCP connection.\n"));
    PlaintextTransportStatus_t xNetworkStatus = PLAINTEXT_TRANSPORT_CONNECT_FAILURE;
    xNetworkStatus = Network_Plaintext_Disconnect(pxNetworkContext);
    xDisconnected = (xNetworkStatus == PLAINTEXT_TRANSPORT_SUCCESS) ? pdPASS : pdFAIL;
#endif
    return xDisconnected;
}

/*-----------------------------------------------------------*/

static void prvMQTTClientSocketWakeupCallback(Socket_t pxSocket) {
    int ionread;
    MQTTAgentCommandInfo_t xCommandParams = {0};
    
    /* Just to avoid compiler warnings.  The socket is not used but the function
     * prototype cannot be changed because this is a callback function. */
    (void)pxSocket;
    
    /* A socket used by the MQTT task may need attention.  Send an event
     * to the MQTT task to make sure the task is not blocked on xCommandQueue. */
    if ((uxQueueMessagesWaiting(xCommandQueue.queue) == 0U) && (!ioctl(pxSocket, FIONREAD, &ionread)) &&
        (ionread > 0)) {
        /* Don't block as this is called from the context of the IP task. */
        xCommandParams.blockTimeMs = 0U;
        MQTTAgent_ProcessLoop(&xGlobalMqttAgentContext, &xCommandParams);
    }
}

/*-----------------------------------------------------------*/

static void prvIncomingPublishCallback(MQTTAgentContext_t *pMqttAgentContext,
                                       uint16_t packetId,
                                       MQTTPublishInfo_t *pxPublishInfo) {
    bool xPublishHandled = false;
    char cOriginalChar, *pcLocation;
    
    (void)packetId;
    
    /* Fan out the incoming publishes to the callbacks registered using
     * subscription manager. */
    xPublishHandled = handleIncomingPublishes((SubscriptionElement_t *)pMqttAgentContext->pIncomingCallbackContext,
                                              pxPublishInfo);
    
    /* If there are no callbacks to handle the incoming publishes,
     * handle it as an unsolicited publish. */
    if (xPublishHandled != true) {
        /* Ensure the topic string is terminated for printing.  This will over-
         * write the message ID, which is restored afterwards. */
        pcLocation = (char *)&(pxPublishInfo->pTopicName[pxPublishInfo->topicNameLength]);
        cOriginalChar = *pcLocation;
        *pcLocation = 0x00;
        LogWarn(("WARN:  Received an unsolicited publish from topic %s", pxPublishInfo->pTopicName));
        *pcLocation = cOriginalChar;
    }
}

/*-----------------------------------------------------------*/

static void prvMQTTAgentTask(void *pvParameters) {
    BaseType_t xNetworkResult = pdFAIL;
    MQTTStatus_t xMQTTStatus = MQTTSuccess, xConnectStatus = MQTTSuccess;
    MQTTContext_t *pMqttContext = &(xGlobalMqttAgentContext.mqttContext);
    
    (void)pvParameters;
    
    do {
        /* MQTTAgent_CommandLoop() is effectively the agent implementation.  It
         * will manage the MQTT protocol until such time that an error occurs,
         * which could be a disconnect.  If an error occurs the MQTT context on
         * which the error happened is returned so there can be an attempt to
         * clean up and reconnect however the application writer prefers. */
        xMQTTStatus = MQTTAgent_CommandLoop(&xGlobalMqttAgentContext);
        
        /* Success is returned for disconnect or termination. The socket should
         * be disconnected. */
        if ((xMQTTStatus == MQTTSuccess) && (xGlobalMqttAgentContext.mqttContext.connectStatus == MQTTNotConnected)) {
            /* MQTT Disconnect. Disconnect the socket. */
            xNetworkResult = prvSocketDisconnect(&xNetworkContext);
            configASSERT(xNetworkResult == pdPASS);
        } else if (xMQTTStatus == MQTTSuccess) {
            /* MQTTAgent_Terminate() was called, but MQTT was not disconnected. */
            xMQTTStatus = MQTT_Disconnect(&(xGlobalMqttAgentContext.mqttContext));
            configASSERT(xMQTTStatus == MQTTSuccess);
            xNetworkResult = prvSocketDisconnect(&xNetworkContext);
            configASSERT(xNetworkResult == pdPASS);
        }
            /* Error. */
        else {
            /* Reconnect TCP. */
            xNetworkResult = prvSocketDisconnect(&xNetworkContext);
            configASSERT(xNetworkResult == pdPASS);
            xNetworkResult = prvSocketConnect(&xNetworkContext);
            configASSERT(xNetworkResult == pdPASS);
            pMqttContext->connectStatus = MQTTNotConnected;
            /* MQTT Connect with a persistent session. */
            xConnectStatus = prvMQTTConnect(false);
            configASSERT(xConnectStatus == MQTTSuccess);
        }
    } while (xMQTTStatus != MQTTSuccess);
}

/*-----------------------------------------------------------*/

static void prvConnectToMQTTBroker(void) {
    BaseType_t xNetworkStatus = pdFAIL;
    MQTTStatus_t xMQTTStatus;
    
    /* Connect a TCP socket to the broker. */
    xNetworkStatus = prvSocketConnect(&xNetworkContext);
    configASSERT(xNetworkStatus == pdPASS);
    
    /* Initialize the MQTT context with the buffer and transport interface. */
    xMQTTStatus = prvMQTTInit();
    configASSERT(xMQTTStatus == MQTTSuccess);
    
    /* Form an MQTT connection without a persistent session. */
    xMQTTStatus = prvMQTTConnect(true);
    configASSERT(xMQTTStatus == MQTTSuccess);
}
/*-----------------------------------------------------------*/

static void prvConnectAndCreateAgentTasks(void *pvParameters) {
    (void)pvParameters;
    
    LWIP_NETCONN_THREAD_SEM_ALLOC();
    
    /* Miscellaneous initialization. */
    ulGlobalEntryTimeMs = prvGetTimeMs();
    
    /* Set the pParams member of the network context with desired transport. */
#if defined( MQTT_USE_TLS ) && (MQTT_USE_TLS == 1)
    xNetworkContext.pParams = &xTlsTransportParams;
#else
    xNetworkContext.pParams = &xPlaintextTransportParams;
#endif
    
    /* Wait for Networking */
    while (bsp_netif_is_linkup() == pdFALSE) {
        LogInfo(("wait for ethernet link up..."));
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
    
    /* Create the TCP connection to the broker, then the MQTT connection to the
     * same. */
    prvConnectToMQTTBroker();
    
    /* Selectively create demo tasks as per the compile time constant settings. */

#if (NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE > 0)
    {
        vStartSimpleSubscribePublishTask(NUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE,
                                         SIMPLE_SUB_PUB_TASK_STACK_SIZE,
                                         tskIDLE_PRIORITY);
    }
#endif
    
    
    /* This task has nothing left to do, so rather than create the MQTT
     * agent as a separate thread, it simply calls the function that implements
     * the agent - in effect turning itself into the agent. */
    prvMQTTAgentTask(NULL);
    
    /* Should not get here.  Force an assert if the task returns from
     * prvMQTTAgentTask(). */
    LWIP_NETCONN_THREAD_SEM_FREE();
    vTaskDelete(NULL);
    
    configASSERT(pvParameters == (void *)~1);
}

/*-----------------------------------------------------------*/

static uint32_t prvGetTimeMs(void) {
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;
    
    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();
    
    /* Convert the ticks to milliseconds. */
    ulTimeMs = (uint32_t)xTickCount * MILLISECONDS_PER_TICK;
    
    /* Reduce ulGlobalEntryTimeMs from obtained time so as to always return the
     * elapsed time in the application. */
    ulTimeMs = (uint32_t)(ulTimeMs - ulGlobalEntryTimeMs);
    
    return ulTimeMs;
}

/*-----------------------------------------------------------*/
