#ifndef _DEBUG_H
#define _DEBUG_H

#include "stdio.h"
#include "type.h"
#include "timer.h"

extern int dbg_level;

#define SYS_MONITOR    			1		//MONITOR开关
#define RELEASE_VERSION			0		//置1后将关闭所有打印信息


#if RELEASE_VERSION		
#undef DEBUG
#endif

#ifdef DEBUG
#define ERR_PRINT_TIME	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)
#define DBG_PRINT_TIME	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)
#define p_info(...) do{if(!dbg_level)break;printf("[I: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_err(...) do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_dbg_track do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s,%d",  __FUNCTION__, __LINE__, ); printf("\r\n");}while(0)
#define p_dbg(...) do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_dbg_enter do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("enter %s\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_dbg_exit do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("exit %s\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_dbg_status do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("status %d\n", status); printf("\r\n");}while(0)
#define p_err_miss do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s miss\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_err_mem do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s mem err\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_err_fun do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s err in %d\n", __FUNCTION__, __LINE__); printf("\r\n");}while(0)
#define dump_hex(tag, buff, size) do{	\
	int dump_hex_i;\
	if(!dbg_level)break;	\
	printf("%s[", tag);\
	for (dump_hex_i = 0; dump_hex_i < size; dump_hex_i++)\
	{\
		printf("%02x ", ((char*)buff)[dump_hex_i]);\
	}\
	printf("]\r\n");\
}while(0)
#else
#define p_info(...) 	do{printf("[I: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000); printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_err(...) 	do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000); printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_err_miss do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s miss\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_err_mem do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s mem err\n", __FUNCTION__); printf("\r\n");}while(0)
#define p_err_fun do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s err in %d\n", __FUNCTION__, __LINE__); printf("\r\n");}while(0)

#define p_dbg_track
#define p_dbg(...)
#define p_dbg_exit
#define p_dbg_enter
#define p_dbg_status

#define dump_hex(tag, buff, size)

#endif


#define p_hex(X, Y) dump_hex("", (unsigned char*)X, Y)

#define assert(x)                                                               \
do{                                                                              		 \
if (!(x))                                                                   	\
{                                                                           			\
p_err( "%s:%d assert " #x "failed\r\n", __FILE__, __LINE__);	\
while(1);										\
}                                                                           			\
}while(0)

typedef enum
{
	PRINT_TYPE_D, PRINT_TYPE_H, PRINT_TYPE_P_D, PRINT_TYPE_P_H,
} STAT_PRINT_TYPE;

struct dev_monitor_item
{
	const char *name;
	uint32_t value;
	//  STAT_PRINT_TYPE type;
};

#if SYS_MONITOR

#define INC_MONITER_ERR_VALUE(x)    (x->err_cnt)++
#define ADD_MONITER_READ_VALUE(x, v)    (x->read_total_size) += (v)
#define ADD_MONITER_WRITE_VALUE(x, v)    (x->write_total_size) += (v)

#define INC_MONITOR_VALUE(X) X.value++
#define ADD_MONITOR_VALUE(X, V) X.value += V
#define SET_MONITER_VALUE(X, V)  X.value = V

#define DECLARE_MONITOR_ITEM(TAG, NAME)	 struct dev_monitor_item NAME  \
__attribute__((section("monitor_item_table"))) = {TAG, 0}

#define EXTERN_MONITOR_ITEM(NAME)	 extern struct dev_monitor_item NAME;


#define INC_MONITOR_ITEM_VALUE(X)  	X.value++
#define DEC_MONITOR_ITEM_VALUE(X)  	X.value--
#define SET_MONITOR_ITEM_VALUE(X, V)  	X.value = V
#define ADD_MONITOR_ITEM_VALUE(X, V) 	X.value += V
#define SUB_MONITOR_ITEM_VALUE(X, V) 	X.value -= V
#define GET_MONITOR_ITEM_VALUE(X) 	X.value

#else
#define DECLARE_MONITOR_ITEM(TAG, NAME)
#define INC_MONITOR_ITEM_VALUE(X)
#define DEC_MONITOR_ITEM_VALUE(X)
#define SET_MONITOR_ITEM_VALUE(X, V)
#define ADD_MONITOR_ITEM_VALUE(X, V)
#define SUB_MONITOR_ITEM_VALUE(X, V)

#define INC_MONITER_ERR_VALUE(x)
#define ADD_MONITER_READ_VALUE(x, v)
#define ADD_MONITER_WRITE_VALUE(x, v)

#define INC_MONITOR_VALUE(X)
#define ADD_MONITOR_VALUE(X, V)
#define SET_MONITER_VALUE(X, V)
#define GET_MONITOR_ITEM_VALUE(X)
#define EXTERN_MONITOR_ITEM(NAME)

#endif

void send_test_pkg(int cmd, const void *data, int size);
void dev_monitor_task(void *arg);
void switch_dbg(void);
void init_monitor(void);

#endif
