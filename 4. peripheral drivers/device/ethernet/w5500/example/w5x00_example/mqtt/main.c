#include <stdio.h>
#include <stdlib.h>
#include "HAL_Config.h"
#include "HALInit.h"
#include "wizchip_conf.h"
#include "inttypes.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "W5x00RelFunctions.h"
#include "serialCommand.h"
#include "Internet\MQTT\MQTTClient.h"
#include "Internet\DNS\dns.h"

wiz_NetInfo gWIZNETINFO = { .mac = {0x00,0x08,0xdc,0x78,0x91,0x71},
							.ip = {192,168,0,15},
							.sn = {255, 255, 255, 0},
							.gw = {192, 168, 0, 1},
							.dns = {168, 126, 63, 1},
							.dhcp = NETINFO_STATIC};

#define ETH_MAX_BUF_SIZE	2048
#define PUBLISH_MYSELF 1

unsigned char ethBuf0[ETH_MAX_BUF_SIZE];
unsigned char ethBuf1[ETH_MAX_BUF_SIZE];
unsigned char ethBuf2[ETH_MAX_BUF_SIZE];
unsigned char ethBuf3[ETH_MAX_BUF_SIZE];

uint8_t bLoopback = 1;
uint8_t bRandomPacket = 0;
uint8_t bAnyPacket = 0;
uint16_t pack_size = 0;

const uint8_t URL[] = "test.mosquitto.org";
uint8_t dns_server_ip[4] = {168,126,63,1};
uint8_t dnsclient_ip[4] = {0,};
unsigned int targetPort = 1883; // mqtt server port
unsigned char targetIP[4] = {0x00, 0x00, 0x00, 0x00}; // mqtt server IP

uint8_t IP_TYPE;

const uint8_t MQTT_TOPIC[]="W5x00";
unsigned char tempBuffer[1024*2] = {};

void print_network_information(void);
void delay(unsigned int count);
void MQTT_run(void);
void messageArrived(MessageData* md);

struct opts_struct
{
	char* clientid;
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* username;
	char* password;
	char* host;
	int port;
	int showtopics;
} opts ={ (char*)"stdout-subscriber", 0, (char*)"\n", QOS0, NULL, NULL, targetIP, 1883, 0 };


int main(void)
{
	volatile int i;
	volatile int j,k;


	RCCInitialize();
	gpioInitialize();
	usartInitialize();
	timerInitialize();
	printf("System start.\r\n");




#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
	// SPI method callback registration
	reg_wizchip_spi_cbfunc(spiReadByte, spiWriteByte);
	// CS function register
	reg_wizchip_cs_cbfunc(csEnable,csDisable);

#else
	// Indirect bus method callback registration
	reg_wizchip_bus_cbfunc(busReadByte, busWriteByte);
#endif

#if _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_
	FSMCInitialize();
#else
	spiInitailize();
#endif
	resetAssert();
	delay(10);
	resetDeassert();
	delay(10);
	W5x00Initialze();
#if _WIZCHIP_ != W5200
	printf("\r\nCHIP Version: %02x\r\n", getVER());
#endif
	wizchip_setnetinfo(&gWIZNETINFO);

	print_network_information();
	DNS_init(0,ethBuf0);
	IP_TYPE = 0x1c;
	while (DNS_run(dns_server_ip, URL, dnsclient_ip) != 1);
	MQTT_run();
}

void delay(unsigned int count)
{
	int temp;
	temp = count + TIM2_gettimer();
	while(temp > TIM2_gettimer()){}
}

void print_network_information(void)
{
	memset(&gWIZNETINFO,0,sizeof(gWIZNETINFO));

	wizchip_getnetinfo(&gWIZNETINFO);
	printf("MAC Address : %02x:%02x:%02x:%02x:%02x:%02x\n\r",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
	printf("IP  Address : %d.%d.%d.%d\n\r",gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
	printf("Subnet Mask : %d.%d.%d.%d\n\r",gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
	printf("Gateway     : %d.%d.%d.%d\n\r",gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
	printf("DNS Server  : %d.%d.%d.%d\n\r",gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
}
void MQTT_run(void){

	int rc = 0;
	uint8_t buf[100];
	uint8_t pubbuf[100];
	Network n;
	MQTTClient c;
	MQTTMessage m;
	uint32_t ck_timer;

	NewNetwork(&n, 0);
	ConnectNetwork(&n, dnsclient_ip, targetPort);
	MQTTClientInit(&c, &n, 1000, buf, sizeof(buf), tempBuffer, sizeof(tempBuffer));

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 60;
	data.cleansession = 1;

	rc = MQTTConnect(&c, &data);
	printf("Connected %d\r\n", rc);
	opts.showtopics = 1;

	printf("Subscribing to %s\r\n", MQTT_TOPIC);
	rc = MQTTSubscribe(&c, MQTT_TOPIC, opts.qos, messageArrived);
	printf("Subscribed %d\r\n", rc);

#if PUBLISH_MYSELF == 1
	m.qos = QOS0;
	m.retained = 0;
	m.dup = 0;

	ck_timer = TIM2_gettimer();
#endif

	while(1)
	{
		MQTTYield(&c, data.keepAliveInterval);

#if PUBLISH_MYSELF == 1
		if(ck_timer + 10000 < TIM2_gettimer())
		{
			ck_timer = TIM2_gettimer();

			//printf("Publishing to %s\r\n", MQTT_TOPIC);

			sprintf(pubbuf, "Hello, W5x00!" );
			m.payload = pubbuf;
			m.payloadlen = strlen(pubbuf);

			rc = MQTTPublish(&c, MQTT_TOPIC, &m);
			printf("Published %d\r\n", rc);
		}
#endif
	}
}
// @brief messageArrived callback function
void messageArrived(MessageData* md)
{
	unsigned char testbuffer[100];
	uint32_t rd_size;
	uint32_t rd_pt;
	uint32_t rdmore_size;

	MQTTMessage* message = md->message;
	MQTTString* topic = md->topicName;

	rdmore_size = (int)topic->lenstring.len;
	rd_pt = 0;

	while(rdmore_size != 0)
	{
		if(rdmore_size > sizeof(testbuffer) - 1)
		{
			rd_size = sizeof(testbuffer) - 1;
		}
		else
		{
			rd_size = rdmore_size;
		}

		memset(testbuffer, 0, rd_size + 1);

		memcpy(testbuffer, ((char*)topic->lenstring.data) + rd_pt, rd_size);
		printf("SubScribe Topic : %s\r\n", testbuffer);

		rd_pt += rd_size;
		rdmore_size -= rd_size;
	}

	if (opts.nodelimiter)
	{
		printf("Message : %.*s\r\n", (int)message->payloadlen, (char*)message->payload);
	}
	else
	{
		printf("Message : %.*s%s\r\n", (int)message->payloadlen, (char*)message->payload, opts.delimiter);
	}
}
