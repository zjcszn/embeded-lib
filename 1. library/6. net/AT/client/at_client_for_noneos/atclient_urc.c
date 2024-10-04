#include "atclient_urc.h"
#include "atclient.h"
#include "bsp_wifi.h"
#include "log.h"
#include "string.h"
#include "stdlib.h"
#include "app_wifi.h"
#include "core_json.h"
#include "menu.h"

static uint8_t mqtt_recv_buf[256];
static uint8_t mqtt_sub_buf[256];

static void urc_ready_cb(void *args);
static void urc_wifi_disconnect_cb(void *args);
static void urc_wifi_connect_cb(void *args);
static void urc_mqtt_disconnect_cb(void *args);
static void urc_mqtt_connect_cb(void *args);
static void urc_mqtt_recv_cb(void *args);

volatile char g_ota_url[256];
volatile uint32_t g_ota_fwsize;
volatile char g_ota_ver[12];
volatile uint8_t g_ota_ready;


const at_urc_t urc_prefix[] = {
    {"ready", sizeof("ready") - 1, urc_ready_cb},
    {"WIFI DISCONNECT", sizeof("WIFI DISCONNECT") - 1, urc_wifi_disconnect_cb},
    {"WIFI CONNECTED", sizeof("WIFI CONNECTED") - 1, urc_wifi_connect_cb},
    {"+MQTTDISCONNECTED", sizeof("+MQTTDISCONNECTED") - 1, urc_mqtt_disconnect_cb},
    {"+MQTTCONNECTED", sizeof("+MQTTCONNECTED") - 1, urc_mqtt_connect_cb},
    {"+MQTTSUBRECV:", sizeof("+MQTTSUBRECV:") - 1, urc_mqtt_recv_cb},
    {0},
};


static void urc_ready_cb(void *args) {
    g_wifi_ready = 1;
    g_wifi_init_ok = 0;
    g_wifi_init_state = 0;
    g_wifi_connect = 0;
    g_mqtt_connect = 0;
    g_mqtt_ok = 0;
    LOG("wifi ready\r\n");
}

static void urc_wifi_disconnect_cb(void *args) {
    if (g_menu_state == 2) {
        g_refresh_flag = 1;
    }
    g_wifi_connect = 0;
    g_mqtt_connect = 0;
    LOG("wifi disconnect\r\n");
}

static void urc_wifi_connect_cb(void *args) {
    if (g_menu_state == 2) {
        g_refresh_flag = 1;
    }
    g_wifi_connect = 1;
    LOG("wifi connected\r\n");
}

static void urc_mqtt_disconnect_cb(void *args) {
    if (g_menu_state == 2) {
        g_refresh_flag = 1;
    }
    g_mqtt_connect = 0;
    g_mqtt_ok = 0;
    LOG("mqtt disconnected\r\n");
}

static void urc_mqtt_connect_cb(void *args) {
    if (g_menu_state == 2) {
        g_refresh_flag = 1;
    }
    g_mqtt_connect = 1;
    LOG("mqtt connect\r\n");
}

static void urc_mqtt_recv_cb(void *args) {
    at_rx_handle_t *rx = args;
    char *p_topic, *p_payload, *value;
    int msg_type;
    size_t payload_len, value_len;
    JSONStatus_t ret;

    LOG("mqtt receive\r\n");

    p_topic = strtok((char *)&(rx->rxbuf[14]), ",");
    if ((p_topic == NULL) || (strncmp((const char*)p_topic, MQTT_SUB_TOPIC, sizeof(MQTT_SUB_TOPIC) - 1) != 0)) {
        return;
    }

    p_payload = strtok(NULL, ",");
    if (p_payload == NULL) {
        return;
    }

    payload_len = strtoul(p_payload, NULL, 10);

    p_payload += strlen(p_payload) + 1;
    p_payload[payload_len] = '\0';

    ret = JSON_Validate(p_payload, payload_len);

    if(ret == JSONSuccess) {
        ret = JSON_Search(p_payload, payload_len, "type", sizeof("type") - 1, &value, &value_len);
    }

    if(ret == JSONSuccess) {
        msg_type = strtoul(value, NULL, 10);

        switch (msg_type) {

            case 1:
                if (JSON_Search(p_payload, payload_len, "url", sizeof("url") - 1, &value, &value_len) != JSONSuccess) {
                    return;
                }
                memcpy((void *)g_ota_url, value, value_len);
                g_ota_url[value_len] = '\0';
                LOG("ota url: %s\r\n", g_ota_url);

                if (JSON_Search(p_payload, payload_len, "size", sizeof("size") - 1, &value, &value_len) != JSONSuccess) {
                    return;
                }
                g_ota_fwsize = strtoul(value, NULL, 10);

                if (JSON_Search(p_payload, payload_len, "version", sizeof("version") - 1, &value, &value_len) != JSONSuccess) {
                    return;
                }           
                memcpy((void *)g_ota_ver, value, value_len);
                g_ota_ver[value_len] = '\0'; 
                LOG("ota version: %s\r\n", g_ota_ver);

                g_ota_ready = 1;
                break;
            
            default:
                break;
        }
    }
}
