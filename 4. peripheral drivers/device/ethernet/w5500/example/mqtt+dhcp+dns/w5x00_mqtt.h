//
// Created by zjcszn on 2024/3/6.
//

#ifndef _W5X00_MQTT_H_
#define _W5X00_MQTT_H_

#define MQTT_BROKER_URL     "v93ef544.cn-hangzhou.emqx.cloud"

#define MQTT_BROKER_PORT    15273

#define MQTT_SOCKET         0
#define DNS_SOCKET          1
#define DHCP_SOCKET         2

void w5x00_mqtt_test(void);

#endif //_W5X00_MQTT_H_
