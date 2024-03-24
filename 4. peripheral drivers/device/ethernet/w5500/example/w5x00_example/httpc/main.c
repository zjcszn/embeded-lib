/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "wizchip_conf.h"
#include "webpage.h"
#include "httpClient.h"
#include "Internet/dns/dns.h"
#include "Internet/dhcp/dhcp.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DATA_BUF_SIZE	2048
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
wiz_NetInfo defaultNetInfo = { .mac = {0x00,0x08,0xdc,0xff,0xee,0xdd},
							.ip = {192,168,0,130},
							.sn = {255,255,255,0},
							.gw = {192,168,0,254},
							//.dns = {168, 126, 63, 1},
							.dns = {8, 8, 8, 8},
							.dhcp = NETINFO_STATIC};


//for DHCP, DNS
#define __USE_DHCP__
#define __USE_DNS__
#define _MAIN_DEBUG_
#define SOCK_DHCP               3
#define SOCK_DNS                4
typedef enum
{
  OFF = 0,
  ON  = 1
} OnOff_State_Type;
uint8_t flag_process_dhcp_success = OFF;
uint8_t flag_process_dns_success = OFF;
//uint8_t dns_server[4] = {8, 8, 8, 8};           // Secondary DNS server IP
uint8_t dns_server[4] = {168, 126, 63, 1};           // Secondary DNS server IP



//for http client
// Example domain name
uint8_t Domain_IP[4]  = {0,};                  // Translated IP address by DNS Server
//uint8_t Domain_name[] = "www.kma.go.kr";
//uint8_t URI[] = "/wid/queryDFSRSS.jsp?zone=4113552000";
uint8_t Domain_name[] = "www.google.com";
uint8_t URI[] = "/search?ei=BkGsXL63AZiB-QaJ8KqoAQ&q=W6100&oq=W6100&gs_l=psy-ab.3...0.0..6208...0.0..0.0.0.......0......gws-wiz.eWEWFN8TORw";

uint8_t flag_sent_http_request = DISABLE;

uint8_t g_send_buf[DATA_BUF_SIZE];
uint8_t g_recv_buf[DATA_BUF_SIZE];

uint8_t data_buf [DATA_BUF_SIZE]; // TX Buffer for applications

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void print_network_information(void);
int8_t process_dhcp(void);
int8_t process_dns(void);
int _write(int fd, char *str, int len)
{
	for(int i=0; i<len; i++)
	{
		HAL_UART_Transmit(&huart2, (uint8_t *)&str[i], 1, 0xFFFF);
	}
	return len;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  uint8_t i;
  uint16_t len = 0;
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  printf("\t - W5x00 Project - \r\n");
  resetAssert();
  HAL_Delay(300);
  resetDeassert();
  HAL_Delay(300);
  printf("\t - WizChip Init - \r\n");

  WIZCHIPInitialize();
#if _WIZCHIP_ == W5100
  HAL_Delay(3000);
#endif
#if _WIZCHIP_ == W5100S
  printf("version:%.2x\r\n", getVER());
  printf("TMSR:%.2x\r\n", getTMSR());
  //NETUNLOCK();
#endif



#if _WIZCHIP_ == W5200  ||  _WIZCHIP_ == W5500
  printf("version:%.2x\r\n", getVERSIONR());
#endif

  wizchip_setnetinfo(&defaultNetInfo);
  print_network_information();

  /* Network Configuration - DHCP client */
  // Initialize Network Information: DHCP or Static IP allocation
#ifdef __USE_DHCP__
  if(process_dhcp() == DHCP_IP_LEASED) // DHCP success
  {
    flag_process_dhcp_success = ON;
  }
  else // DHCP failed
  {
    ctlnetwork(CN_SET_NETINFO,&defaultNetInfo); // Set default static IP settings
  }
#else
  ctlnetwork(CN_SET_NETINFO,&defaultNetInfo); // Set default static IP settings
#endif
  printf("Register value after W5x00 initialize!\r\n");
  print_network_information();

  /* DNS client */
#ifdef __USE_DNS__
  if(process_dns()) // DNS success
  {
    flag_process_dns_success = ON;
  }
#endif
  // Debug UART: DNS results print out
#ifdef __USE_DHCP__
  if(flag_process_dhcp_success == ENABLE)
  {
    printf(" # DHCP IP Leased time : %u seconds\r\n", getDHCPLeasetime());
  }
  else
  {
    printf(" # DHCP Failed\r\n");
  }
#endif

#ifdef __USE_DNS__
  if(flag_process_dns_success == ENABLE)
  {
    printf(" # DNS: %s => %d.%d.%d.%d\r\n", Domain_name, Domain_IP[0], Domain_IP[1], Domain_IP[2], Domain_IP[3]);
  }
  else
  {
    printf(" # DNS Failed\r\n");
  }
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
  {
    httpc_init(0, Domain_IP, 80, g_send_buf, g_recv_buf);
    while(1){
		  httpc_connection_handler();
		  if(httpc_isSockOpen)
			{
				httpc_connect();
			}
		  // HTTP client example
		  if(httpc_isConnected)
		  {
		    if(!flag_sent_http_request)
			  {
				  // Send: HTTP request
				  request.method = (uint8_t *)HTTP_GET;
				  request.uri = (uint8_t *)URI;
				  request.host = (uint8_t *)Domain_name;

				  // HTTP client example #1: Function for send HTTP request (header and body fields are integrated)
				  {
					  httpc_send(&request, g_recv_buf, g_send_buf, 0);
				  }

				  // HTTP client example #2: Separate functions for HTTP request - default header + body
				  {
					  //httpc_send_header(&request, g_recv_buf, NULL, len);
					  //httpc_send_body(g_send_buf, len); // Send HTTP requset message body
				  }

				  // HTTP client example #3: Separate functions for HTTP request with custom header fields - default header + custom header + body
				  {
					  //httpc_add_customHeader_field(tmpbuf, "Custom-Auth", "auth_method_string"); // custom header field extended - example #1
					  //httpc_add_customHeader_field(tmpbuf, "Key", "auth_key_string"); // custom header field extended - example #2
					  //httpc_send_header(&request, g_recv_buf, tmpbuf, len);
					  //httpc_send_body(g_send_buf, len);
				  }

				  flag_sent_http_request = ENABLE;
			  }

		    // Recv: HTTP response
		    if(httpc_isReceived > 0)
		    {
				  len = httpc_recv(g_recv_buf, httpc_isReceived);

				  printf(" >> HTTP Response - Received len: %d\r\n", len);
				  printf("======================================================\r\n");
				  for(i = 0; i < len; i++) printf("%c", g_recv_buf[i]);
				  printf("\r\n");
				  printf("======================================================\r\n");
			  }
		  }
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(W5x00_RESET_GPIO_Port, W5x00_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : W5x00_RESET_Pin */
  GPIO_InitStruct.Pin = W5x00_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(W5x00_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void print_network_information(void)
{
	wizchip_getnetinfo(&defaultNetInfo);
	printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n\r",defaultNetInfo.mac[0],defaultNetInfo.mac[1],defaultNetInfo.mac[2],defaultNetInfo.mac[3],defaultNetInfo.mac[4],defaultNetInfo.mac[5]);
	printf("IP address : %d.%d.%d.%d\n\r",defaultNetInfo.ip[0],defaultNetInfo.ip[1],defaultNetInfo.ip[2],defaultNetInfo.ip[3]);
	printf("SM Mask	   : %d.%d.%d.%d\n\r",defaultNetInfo.sn[0],defaultNetInfo.sn[1],defaultNetInfo.sn[2],defaultNetInfo.sn[3]);
	printf("Gate way   : %d.%d.%d.%d\n\r",defaultNetInfo.gw[0],defaultNetInfo.gw[1],defaultNetInfo.gw[2],defaultNetInfo.gw[3]);
	printf("DNS Server : %d.%d.%d.%d\n\r",defaultNetInfo.dns[0],defaultNetInfo.dns[1],defaultNetInfo.dns[2],defaultNetInfo.dns[3]);
}


int8_t process_dhcp(void)
{
  uint8_t ret = 0;
  uint8_t dhcp_retry = 0;

#ifdef _MAIN_DEBUG_
  printf(" - DHCP Client running\r\n");
#endif
  DHCP_init(SOCK_DHCP, data_buf);
  while(1)
  {
    ret = DHCP_run();

    if(ret == DHCP_IP_LEASED)
    {
#ifdef _MAIN_DEBUG_
      printf(" - DHCP Success\r\n");
#endif
        break;
      }
    else if(ret == DHCP_FAILED)
    {
      dhcp_retry++;
#ifdef _MAIN_DEBUG_
      if(dhcp_retry <= 3) printf(" - DHCP Timeout occurred and retry [%d]\r\n", dhcp_retry);
#endif
    }

    if(dhcp_retry > 3)
    {
#ifdef _MAIN_DEBUG_
      printf(" - DHCP Failed\r\n\r\n");
#endif
      DHCP_stop();
      break;
    }

  }

  return ret;
}


int8_t process_dns(void)
{
  int8_t ret = 0;
  uint8_t dns_retry = 0;

#ifdef _MAIN_DEBUG_
  printf(" - DNS Client running\r\n");
#endif
  DNS_init(SOCK_DNS, data_buf);

  while(1)
  {

    if((ret = DNS_run(dns_server, (uint8_t *)Domain_name, Domain_IP)) == 1)
    {
#ifdef _MAIN_DEBUG_
      printf(" - DNS Success\r\n");
#endif
      break;
    }
    else
    {
      dns_retry++;
#ifdef _MAIN_DEBUG_
      if(dns_retry <= 2) printf(" - DNS Timeout occurred and retry [%d]\r\n", dns_retry);
#endif
    }



    if(dns_retry > 2) {
#ifdef _MAIN_DEBUG_
      printf(" - DNS Failed\r\n\r\n");
#endif
  break;
    }

#ifdef __USE_DHCP__
  DHCP_run();
#endif
  }
  return ret;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
