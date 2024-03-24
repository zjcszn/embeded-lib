//
// Created by zjcszn on 2024/3/6.
//

#define LOG_TAG "MQTT"
#define LOG_LVL ELOG_LVL_DEBUG

#include "w5x00_mqtt.h"
#include "elog.h"

#include "w5500_port.h"
#include "wizchip_conf.h"
#include "dns.h"
#include "dhcp.h"
#include "MQTTClient.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

typedef struct {
    volatile uint8_t link_ok;
    volatile uint8_t addr_ok;
    uint8_t dhcp_retry;
} w5x00_status_t;

static MQTTClient mqttc;
static Network network;
static uint8_t dns_buf[MAX_DNS_BUF_SIZE];
static uint8_t dhcp_buf[768];
static uint8_t mqtt_buf[2048];
static w5x00_status_t w5x00_stat;
static uint8_t dns_ip[4] = {119, 29, 29, 29};

void w5x00_dhcp_process(struct tmrTimerControl *timer) {
    uint8_t ret;
    (void)timer;
    
    if (wizphy_getphylink() != PHY_LINK_ON) {
        if (w5x00_stat.link_ok == 1) {
            wizchip_sw_reset();
            w5x00_stat.link_ok = 0;
            w5x00_stat.addr_ok = 0;
        }
        return;
    } else {
        if (w5x00_stat.link_ok == 0) {
            DHCP_init(DHCP_SOCKET, dhcp_buf);
            w5x00_stat.link_ok = 1;
            w5x00_stat.dhcp_retry = 0;
        }
    }
    
    ret = DHCP_run();
    
    if (ret == DHCP_IP_LEASED) {
        if (w5x00_stat.addr_ok == 0) {
            w5x00_stat.addr_ok = 1;
            log_d("dhcp state: ip leased");
            w5500_network_info_show();
        }
    } else if (ret == DHCP_IP_CHANGED) {
        log_d("dhcp state: ip updated");
        w5500_network_info_show();
    } else if (ret == DHCP_FAILED) {
        if (++w5x00_stat.dhcp_retry < 3) {
            log_e("dhcp failed, try %dst", w5x00_stat.dhcp_retry);
        } else {
            log_d("use static ip");
            DHCP_stop();
            w5500_network_info_init(0);
            w5500_network_info_show();
            w5x00_stat.addr_ok = 1;
        }
    }
}

void w5x00_url_parse(const char *url, uint8_t url_ip[4]) {
    uint8_t i = 0;
    while (DNS_run(dns_ip, (uint8_t *)url, url_ip) == 0) {
        if (++i > 3) {
            log_e("url parse failed");
            return;
        }
    }
    
    log_d("url: %s", url);
    log_d("parse_ip: %d.%d.%d.%d", url_ip[0], url_ip[1], url_ip[2], url_ip[3]);
}

void w5x00_mqtt_subscribe_cb(MessageData *md) {
    MQTTMessage *message = md->message;
    MQTTString *topic = md->topicName;
    log_d("topic name: %.*s", topic->lenstring.len, topic->lenstring.data);
    log_d("%.*s", (int)message->payloadlen, (char *)message->payload);
}

void w5x00_mqtt_task(void *args) {
    (void)args;
    while (1) {
        MQTTYield(&mqttc, 60);
    }
}

void w5x00_mqtt_test(void) {
    int ret;
    uint8_t parse_ip[4];
    
    w5500_init();
    
    DNS_init(DNS_SOCKET, dns_buf);
    DHCP_init(DHCP_SOCKET, dhcp_buf);
    MQTTClientInit(&mqttc, &network, 1000, mqtt_buf, 2048, mqtt_buf, 2048);
    
    TimerHandle_t dhcp_timer = xTimerCreate("dhcp", pdMS_TO_TICKS(100), pdTRUE, NULL, w5x00_dhcp_process);
    xTimerStart(dhcp_timer, 0);
    
    while (w5x00_stat.addr_ok == 0) {
        vTaskDelay(100);
    };
    
    NewNetwork(&network, 0);
    w5x00_url_parse(MQTT_BROKER_URL, parse_ip);
    
    ConnectNetwork(&network, (char *)parse_ip, MQTT_BROKER_PORT);
    
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = "zjcszn-client";
    data.username.cstring = "zjcszn";
    data.password.cstring = "Farmer007";
    data.keepAliveInterval = 60;
    data.cleansession = 1;
    
    ret = MQTTConnect(&mqttc, &data);
    printf("Connected %d\r\n", ret);
    printf("Subscribing to %s\r\n", "hello/wiznet");
    ret = MQTTSubscribe(&mqttc, "hello/wiznet", QOS0, w5x00_mqtt_subscribe_cb);
    printf("Subscribed %d\r\n", ret);
    
    xTaskCreate(w5x00_mqtt_task, "mqtt", 512, NULL, configMAX_PRIORITIES - 1, NULL);
}

