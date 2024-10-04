/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-22
 * 
 * @copyright (c) 2019 Letter
 * 
 */
#include <stdbool.h>
#include "shell_port.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "shell.h"
#include "dev_uart.h"
#include "elog.h"


Shell shell;
char  shellBuffer[512];

static TaskHandle_t      shellTaskHandle;
static SemaphoreHandle_t shellMutex;

/**
 * @brief 用户shell写
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际写入的数据长度
 */
unsigned long userShellWrite(char *data, unsigned long len)
{
    return uart_write(data, len);
}

/**
 * @brief 用户shell读
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际读取到
 */
unsigned long userShellRead(char *data, unsigned long len)
{
    return uart_read(data, len);
}

/**
 * @brief 用户shell上锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellLock(Shell *shell)
{
    xSemaphoreTakeRecursive(shellMutex, portMAX_DELAY);
    return 0;
}

/**
 * @brief 用户shell解锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellUnlock(Shell *shell)
{ 
    xSemaphoreGiveRecursive(shellMutex);
    return 0;
}

void userShellRecvNotice(void)
{
    vTaskNotifyGiveFromISR(shellTaskHandle, NULL);
}

/**
 * @brief 用户shell初始化
 * 
 */
void bsp_shell_init(void)
{
    shellMutex = xSemaphoreCreateMutex();
    shell.write = userShellWrite;
    shell.read = userShellRead;
    shell.lock = userShellLock;
    shell.unlock = userShellUnlock;

    uart_set_rx_notice((uart_cbf_t)userShellRecvNotice);
    shellInit(&shell, shellBuffer, 512);
    if (xTaskCreate(shellTask, "shell", 4096, &shell, 5, &shellTaskHandle) != pdPASS)
    {
        //log_e("shell task creat failed");
    }
}

