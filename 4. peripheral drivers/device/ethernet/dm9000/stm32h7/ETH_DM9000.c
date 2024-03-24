/*
*********************************************************************************************************
*
*	模块名称 : CMSIS-Driver
*	文件名称 : ETH_DM9000.c
*	版    本 : V1.0
*	说    明 : CMSIS-Driver的DM9000底层驱动实现【安富莱原创】
*
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2020-11-21   Eric2013 正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include <string.h>
#include "cmsis_compiler.h"

#include "Driver_ETH_MAC.h"
#include "Driver_ETH_PHY.h"
#include "ETH_DM9000.h"
#include "DM9000_BSP.h"


#define ARM_ETH_MAC_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) /* driver version */
#define ARM_ETH_PHY_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) /* driver version */


/* Ethernet MAC driver number (default) */
#ifndef ETH_MAC_NUM
#define ETH_MAC_NUM             1
#endif

/* Ethernet PHY driver number (default) */
#ifndef ETH_PHY_NUM
#define ETH_PHY_NUM             1
#endif

#ifdef __clang__
  #define __rbit(v)             __builtin_arm_rbit(v)
#endif

/* Driver Version */
static const ARM_DRIVER_VERSION MAC_DriverVersion = {
  ARM_ETH_MAC_API_VERSION,
  ARM_ETH_MAC_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_ETH_MAC_CAPABILITIES MAC_DriverCapabilities = {
  0,                                /* checksum_offload_rx_ip4  */
  0,                                /* checksum_offload_rx_ip6  */
  0,                                /* checksum_offload_rx_udp  */
  0,                                /* checksum_offload_rx_tcp  */
  0,                                /* checksum_offload_rx_icmp */
  0,                                /* checksum_offload_tx_ip4  */
  0,                                /* checksum_offload_tx_ip6  */
  0,                                /* checksum_offload_tx_udp  */
  0,                                /* checksum_offload_tx_tcp  */
  0,                                /* checksum_offload_tx_icmp */
  0,                                /* media_interface          */
  0,                                /* mac_address              */
  1,                                /* event_rx_frame           */
  0,                                /* event_tx_frame           */
  0,                                /* event_wakeup             */
  0,                                /* precision_timer          */
  0                                 /* reserved                 */
};

/* Driver control structure */
static ETH_CTRL ETH = { 0, 0, 0, 0, 0 };

/* Local functions */
static uint32_t reg_rd (uint8_t reg);
static void     reg_wr (uint8_t reg, uint32_t val);


#define   NET_BASE_ADDR			0x60002000
#define   NET_REG_ADDR			(*((volatile uint16_t *) NET_BASE_ADDR))
#define   NET_REG_DATA			(*((volatile uint16_t *) (NET_BASE_ADDR + 0x00000004)))  

/**
  Read register value.

  \param[in]   reg  Register to read
*/
static uint32_t reg_rd (uint8_t reg) {
	
	NET_REG_ADDR = reg;
	return (NET_REG_DATA);
}

/**
  Write register value.

  \param[in]   reg  Register to write
  \param[in]   val  Value to write
*/
static void reg_wr (uint8_t reg, uint32_t val) {
	
	NET_REG_ADDR = reg;
	NET_REG_DATA = val;
	
}

/* Ethernet Driver functions */

/**
  \fn          ARM_DRIVER_VERSION GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION MAC_GetVersion (void) {
  return MAC_DriverVersion;
}

/**
  \fn          ARM_ETH_MAC_CAPABILITIES GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_ETH_MAC_CAPABILITIES
*/
static ARM_ETH_MAC_CAPABILITIES MAC_GetCapabilities (void) {
  return MAC_DriverCapabilities;
}

/**
  \fn          int32_t Initialize (ARM_ETH_MAC_SignalEvent_t cb_event)
  \brief       Initialize Ethernet MAC Device.
  \param[in]   cb_event  Pointer to \ref ARM_ETH_MAC_SignalEvent
  \return      \ref execution_status
*/
static int32_t MAC_Initialize (ARM_ETH_MAC_SignalEvent_t cb_event) {
  (void)cb_event;

  if (ETH.Flags & ETH_INIT) { return ARM_DRIVER_OK; }

  /* Determine number of CPU cycles within 1us */
  if (SystemCoreClock < 1000000U) {
    ETH.Us_Cyc = 1U;
  } else {
    ETH.Us_Cyc = (SystemCoreClock + (1000000U-1U)) / 1000000U;
  }

  ETH.Flags = ETH_INIT;
  ETH.cb_event = cb_event;

  etherdev_init();
  
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t Uninitialize (void)
  \brief       De-initialize Ethernet MAC Device.
  \return      \ref execution_status
*/
static int32_t MAC_Uninitialize (void) {

  ETH.Flags = 0U;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t PowerControl (ARM_POWER_STATE state)
  \brief       Control Ethernet MAC Device Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t MAC_PowerControl (ARM_POWER_STATE state) {
  switch ((int32_t)state) {
    case ARM_POWER_OFF:
      ETH.Flags &= ETH_POWER;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_POWER_FULL:
      if ((ETH.Flags & ETH_INIT) == 0) {
        return ARM_DRIVER_ERROR;
      }
      if (ETH.Flags & ETH_POWER) {
        return ARM_DRIVER_OK;
      }

      ETH.Flags |= ETH_POWER;
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t GetMacAddress (ARM_ETH_MAC_ADDR *ptr_addr)
  \brief       Get Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \return      \ref execution_status
*/
static int32_t MAC_GetMacAddress (ARM_ETH_MAC_ADDR *ptr_addr) {
  
  if (ptr_addr == NULL) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((ETH.Flags & ETH_POWER) == 0U) {
    return ARM_DRIVER_ERROR;
  }
  
  ptr_addr->b[0] = reg_rd (DM9000_REG_PAR + 0);
  ptr_addr->b[1] = reg_rd (DM9000_REG_PAR + 1);
  ptr_addr->b[2] = reg_rd (DM9000_REG_PAR + 2);
  ptr_addr->b[3] = reg_rd (DM9000_REG_PAR + 3);
  ptr_addr->b[4] = reg_rd (DM9000_REG_PAR + 4);
  ptr_addr->b[5] = reg_rd (DM9000_REG_PAR + 5);

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SetMacAddress (const ARM_ETH_MAC_ADDR *ptr_addr)
  \brief       Set Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \return      \ref execution_status
*/
static int32_t MAC_SetMacAddress (const ARM_ETH_MAC_ADDR *ptr_addr) {

  if (ptr_addr == NULL) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((ETH.Flags & ETH_POWER) == 0U) {
    return ARM_DRIVER_ERROR;
  }
  
  reg_wr (DM9000_REG_PAR + 0, ptr_addr->b[0]);
  reg_wr (DM9000_REG_PAR + 1, ptr_addr->b[1]);
  reg_wr (DM9000_REG_PAR + 2, ptr_addr->b[2]);
  reg_wr (DM9000_REG_PAR + 3, ptr_addr->b[3]);
  reg_wr (DM9000_REG_PAR + 4, ptr_addr->b[4]);
  reg_wr (DM9000_REG_PAR + 5, ptr_addr->b[5]);

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SetAddressFilter (const ARM_ETH_MAC_ADDR *ptr_addr,
                                               uint32_t          num_addr)
  \brief       Configure Address Filter.
  \param[in]   ptr_addr  Pointer to addresses
  \param[in]   num_addr  Number of addresses to configure
  \return      \ref execution_status
*/
static int32_t MAC_SetAddressFilter (const ARM_ETH_MAC_ADDR *ptr_addr, uint32_t num_addr) {
  if ((ptr_addr == NULL) && (num_addr != 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((ETH.Flags & ETH_POWER) == 0U) {
    return ARM_DRIVER_ERROR;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SendFrame (const uint8_t *frame, uint32_t len, uint32_t flags)
  \brief       Send Ethernet frame.
  \param[in]   frame  Pointer to frame buffer with data to send
  \param[in]   len    Frame buffer length in bytes
  \param[in]   flags  Frame transmit flags (see ARM_ETH_MAC_TX_FRAME_...)
  \return      \ref execution_status
*/
static int32_t MAC_SendFrame (const uint8_t *frame, uint32_t len, uint32_t flags) {
  if ((frame == NULL) || (len == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((ETH.Flags & ETH_POWER) == 0U) {
    return ARM_DRIVER_ERROR;
  }
  
  dm9k_send_packet((uint8_t *)frame, len);

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t ReadFrame (uint8_t *frame, uint32_t len)
  \brief       Read data of received Ethernet frame.
  \param[in]   frame  Pointer to frame buffer for data to read into
  \param[in]   len    Frame buffer length in bytes
  \return      number of data bytes read or execution status
                 - value >= 0: number of data bytes read
                 - value < 0: error occurred, value is execution status as defined with \ref execution_status 
*/
static int32_t MAC_ReadFrame (uint8_t *frame, uint32_t len) {
	int32_t rval = ARM_DRIVER_OK;
	
	uint16_t *src2 = (uint16_t *)frame;
	uint16_t temp;
	uint32_t blkCnt; 
	
	
	blkCnt = len >> 3U;
		
	while (blkCnt > 0U)
	{
		*src2++ = NET_REG_DATA;
		*src2++ = NET_REG_DATA;
		*src2++ = NET_REG_DATA;
		*src2++ = NET_REG_DATA;
		blkCnt--;
	}

	blkCnt = (len/2) % 0x4U;

	while (blkCnt > 0U)
	{
		*src2++ = NET_REG_DATA;
		blkCnt--;
	}
	
	if(len%2)
	{
		frame[len - 1] = (uint8_t)NET_REG_DATA;
	}
	
	temp = NET_REG_DATA;
	temp = NET_REG_DATA;	

    (void)temp;
	return (rval);
}

/**
  \fn          uint32_t GetRxFrameSize (void)
  \brief       Get size of received Ethernet frame.
  \return      number of bytes in received frame
*/
static uint32_t MAC_GetRxFrameSize (void) {
    uint32_t len = 0;

    if ((ETH.Flags & ETH_POWER) == 0U) {
        return (0U);
    }

    len = GetSize();

    return (len);
}

/**
  \fn          int32_t GetRxFrameTime (ARM_ETH_MAC_TIME *time)
  \brief       Get time of received Ethernet frame.
  \param[in]   time  Pointer to time structure for data to read into
  \return      \ref execution_status
*/
static int32_t MAC_GetRxFrameTime (ARM_ETH_MAC_TIME *time) {
  (void)time;
  return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
  \fn          int32_t GetTxFrameTime (ARM_ETH_MAC_TIME *time)
  \brief       Get time of transmitted Ethernet frame.
  \param[in]   time  Pointer to time structure for data to read into
  \return      \ref execution_status
*/
static int32_t MAC_GetTxFrameTime (ARM_ETH_MAC_TIME *time) {
  (void)time;
  return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
  \fn          int32_t ControlTimer (uint32_t control, ARM_ETH_MAC_TIME *time)
  \brief       Control Precision Timer.
  \param[in]   control  Operation
  \param[in]   time     Pointer to time structure
  \return      \ref execution_status
*/
static int32_t MAC_ControlTimer (uint32_t control, ARM_ETH_MAC_TIME *time) {
  (void)control;
  (void)time;
  return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
  \fn          int32_t Control (uint32_t control, uint32_t arg)
  \brief       Control Ethernet Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \return      \ref execution_status
*/
static int32_t MAC_Control (uint32_t control, uint32_t arg) {

  if ((ETH.Flags & ETH_POWER) == 0U) {
    return ARM_DRIVER_ERROR;
  }

  switch (control) {
    case ARM_ETH_MAC_CONFIGURE:
      break;

    case ARM_ETH_MAC_CONTROL_TX:
      break;

    case ARM_ETH_MAC_CONTROL_RX:
      break;

    case ARM_ETH_MAC_FLUSH:
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}


/**
  \fn          int32_t PHY_Read (uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
  \brief       Read Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[out]  data      Pointer where the result is written to
  \return      \ref execution_status
*/
static int32_t PHY_Read (uint8_t phy_addr, uint8_t reg_addr, uint16_t *data) {
    int32_t  status = ARM_DRIVER_OK;

    *data = dm9k_phy_read(reg_addr);
    return (status);
}

/**
  \fn          int32_t PHY_Write (uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
  \brief       Write Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[in]   data      16-bit data to write
  \return      \ref execution_status
*/
static int32_t PHY_Write (uint8_t phy_addr, uint8_t reg_addr, uint16_t data) {
  int32_t  status  = ARM_DRIVER_OK;

	dm9k_phy_write(reg_addr, data);
  return (status);
}

extern __weak void dm9k_re_packet(void);
void dm9k_re_packet(void)
{
	ETH.cb_event(ARM_ETH_MAC_EVENT_RX_FRAME);
}

/* MAC Driver Control Block */
extern
ARM_DRIVER_ETH_MAC ARM_Driver_ETH_MAC_(ETH_MAC_NUM);
ARM_DRIVER_ETH_MAC ARM_Driver_ETH_MAC_(ETH_MAC_NUM) = {
  MAC_GetVersion,
  MAC_GetCapabilities,
  MAC_Initialize,
  MAC_Uninitialize,
  MAC_PowerControl,
  MAC_GetMacAddress,
  MAC_SetMacAddress,
  MAC_SetAddressFilter,
  MAC_SendFrame,
  MAC_ReadFrame,
  MAC_GetRxFrameSize,
  MAC_GetRxFrameTime,
  MAC_GetTxFrameTime,
  MAC_ControlTimer,
  MAC_Control,
  PHY_Read,
  PHY_Write
};


/* Driver Version */
static const ARM_DRIVER_VERSION PHY_DriverVersion = {
  ARM_ETH_PHY_API_VERSION,
  ARM_ETH_PHY_DRV_VERSION
};

/**
  \fn          ARM_DRV_VERSION GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRV_VERSION
*/
static ARM_DRIVER_VERSION PHY_GetVersion (void) {
  return PHY_DriverVersion;
}

/**
  \fn          int32_t Initialize (ARM_ETH_PHY_Read_t  fn_read,
                                   ARM_ETH_PHY_Write_t fn_write)
  \brief       Initialize Ethernet PHY Device.
  \param[in]   fn_read   Pointer to \ref ARM_ETH_MAC_PHY_Read
  \param[in]   fn_write  Pointer to \ref ARM_ETH_MAC_PHY_Write
  \return      \ref execution_status
*/
static int32_t PHY_Initialize (ARM_ETH_PHY_Read_t fn_read, ARM_ETH_PHY_Write_t fn_write) {
  /* PHY_Read and PHY_Write will be re-used, no need to register them again */
  (void)fn_read;
  (void)fn_write;
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t Uninitialize (void)
  \brief       De-initialize Ethernet PHY Device.
  \return      \ref execution_status
*/
static int32_t PHY_Uninitialize (void) {
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t PowerControl (ARM_POWER_STATE state)
  \brief       Control Ethernet PHY Device Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t PHY_PowerControl (ARM_POWER_STATE state) {
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SetInterface (uint32_t interface)
  \brief       Set Ethernet Media Interface.
  \param[in]   interface  Media Interface type
  \return      \ref execution_status
*/
static int32_t PHY_SetInterface (uint32_t interface) {
  /* Not used in this driver */
  (void)interface;
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SetMode (uint32_t mode)
  \brief       Set Ethernet PHY Device Operation mode.
  \param[in]   mode  Operation Mode
  \return      \ref execution_status
*/
#include "PHY_DM916x.h"
static int32_t PHY_SetMode (uint32_t mode) {
  uint16_t val = 0;

  if ((ETH.Flags & ETH_POWER) == 0U) { return ARM_DRIVER_ERROR; }

  val &= ~(BMCR_SPEED_SELECT | BMCR_DUPLEX_MODE |
           BMCR_ANEG_EN    | BMCR_LOOPBACK);

  switch (mode & ARM_ETH_PHY_SPEED_Msk) {
    case ARM_ETH_PHY_SPEED_10M:
      break;
    case ARM_ETH_PHY_SPEED_100M:
      val |= BMCR_SPEED_SELECT;
      break;
    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  switch (mode & ARM_ETH_PHY_DUPLEX_Msk) {
    case ARM_ETH_PHY_DUPLEX_HALF:
      break;
    case ARM_ETH_PHY_DUPLEX_FULL:
      val |= BMCR_DUPLEX_MODE;
      break;
  }

  if (mode & ARM_ETH_PHY_AUTO_NEGOTIATE) {
    val |= BMCR_ANEG_EN;
  }

  if (mode & ARM_ETH_PHY_LOOPBACK) {
    val |= BMCR_LOOPBACK;
  }

  if (mode & ARM_ETH_PHY_ISOLATE) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  
  /* Apply configured mode */
  return PHY_Write (DM9000_PHY, REG_BMCR, val);
}

/**
  \fn          ARM_ETH_LINK_STATE GetLinkState (void)
  \brief       Get Ethernet PHY Device Link state.
  \return      current link status \ref ARM_ETH_LINK_STATE
*/
static ARM_ETH_LINK_STATE PHY_GetLinkState (void) {
  ARM_ETH_LINK_STATE state;
  uint16_t           val = 0U;

  if (ETH.Flags & PHY_POWER) {
	PHY_Read(DM9000_PHY, REG_BMSR, &val);
  }
  state = (val & BMSR_LINK_STAT) ? ARM_ETH_LINK_UP : ARM_ETH_LINK_DOWN;

  return (state);
}

/**
  \fn          ARM_ETH_LINK_INFO GetLinkInfo (void)
  \brief       Get Ethernet PHY Device Link information.
  \return      current link parameters \ref ARM_ETH_LINK_INFO
*/
static ARM_ETH_LINK_INFO PHY_GetLinkInfo (void) {

  ARM_ETH_LINK_INFO info;
  uint16_t          val = 0U;

  if (ETH.Flags & PHY_POWER) {
    /* Get operation mode indication from PHY DSCSR register */
    PHY_Read(DM9000_PHY, REG_DSCSR, &val);
  }

  /* Link must be up to get valid state */
  info.speed  = ((val & DSCSR_100M_FD)|(val & DSCSR_100M_HD)) ? ARM_ETH_SPEED_100M  : ARM_ETH_SPEED_10M;
  info.duplex = ((val & DSCSR_100M_FD)|(val & DSCSR_10M_FD)) ? ARM_ETH_DUPLEX_FULL : ARM_ETH_DUPLEX_HALF;


  return (info);
}

/* PHY Driver Control Block */
extern
ARM_DRIVER_ETH_PHY ARM_Driver_ETH_PHY_(ETH_PHY_NUM);
ARM_DRIVER_ETH_PHY ARM_Driver_ETH_PHY_(ETH_PHY_NUM) = {
  PHY_GetVersion,
  PHY_Initialize,
  PHY_Uninitialize,
  PHY_PowerControl,
  PHY_SetInterface,
  PHY_SetMode,
  PHY_GetLinkState,
  PHY_GetLinkInfo
};
