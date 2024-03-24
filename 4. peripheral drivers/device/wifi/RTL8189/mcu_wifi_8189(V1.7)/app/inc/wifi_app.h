#ifndef _WIFI_APP_H_
#define _WIFI_APP_H_

typedef enum
{
	KEY_NONE = 0, KEY_WEP, KEY_WPA, KEY_WPA2, KEY_MAX_VALUE
} SECURITY_TYPE;

extern struct net_device *wifi_dev;

	
int is_wifi_connected(void);
int wifi_disconnect(void);
int wifi_connect( char *essid, char *key);
int wifi_get_mac(unsigned char *mac_addr);
int init_wifi(void);
int wifi_set_channel(int channel);
int is_hw_ok(void);
int create_mac(unsigned char *mac);
int wifi_power_cfg(uint8_t power_save_level);
int wifi_set_promisc(bool enable);
int wifi_set_mode(int type);
int wifi_ap_cfg(char *name, char *key, int key_type, int channel, int max_client);
#endif
