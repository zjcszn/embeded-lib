#ifndef _TEST_H
#define _TEST_H

void test_scan(void);
void test_wifi_connect(void);
void test_start_ap(void);
void test_wifi_disconnect(void);

void test_tcp_link(void);
void test_tcp_unlink(void);
void test_send(char *pstr);
void test_sendto(char *pstr);
void test_close_tcp_server(void);
void test_dns(char *hostname);
void test_tcp_server(void);
void test_auto_get_ip(void);
void test_open_audio(void);
void test_close_audio(void);
void test_wifi_get_stats(void);
void show_tcpip_info(void);
void show_sys_info(void);
void test_udp_link(void);
void test_multicast_join(void);
void test_udp_server(void);
void test_create_ap(void);
void test_stop_ap(void);
void test_get_station_list(void);
void test_power_save_enable(void);
void test_power_save_disable(void);
void test_wifi_join_adhoc(void);
void test_wifi_leave_adhoc(void);
void monitor_switch(void);
void test_full_speed_send(void);
void test_full_speed_send_stop(void);
void switch_loopback_test(void);
int test_connect_to_machtalk_server(void);
void custom_test_demo(void);
#endif
