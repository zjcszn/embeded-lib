#include "nt35510.h"
#include "stm32f4xx_hal.h"

/* Private variable definition --------------------------------------------------*/


static NT35510_ParamsTypedef          nt35510_params;


/* Private macro definition --------------------------------------------------*/

#if       NT35510_USE_LOG
#include  <stdio.h>
#define   NT35510_LOG(...)            printf(__VA_ARGS__)
#else
#define   NT35510_LOG(...)            ((void)0U)
#endif

#define NT35510_Delay(ms)             HAL_Delay(ms);

#undef  ABS
#define ABS(exp)                      ((exp) >= 0 ? (exp) : (-(exp)))
#undef  MIN
#define MIN(a,b)                      ((a) < (b) ? (a) : (b))


/* NT35510 Command List ------------------------------------------------------*/

#define NT35510_CMD_NOP               0x0000  /* NOP */
#define NT35510_CMD_SWRESET           0x0100  /* SW reset */
#define NT35510_CMD_RDDID             0x0400  /* Read display ID */
#define NT35510_CMD_RDNUMED           0x0500  /* Read number of errors on DSI */
#define NT35510_CMD_RDDPM             0x0A00  /* Read display power mode */
#define NT35510_CMD_RDDMADCTL         0x0B00  /* Read display MADCTL */
#define NT35510_CMD_RDDCOLMOD         0x0C00  /* Read display pixel format */
#define NT35510_CMD_RDDIM             0x0D00  /* Read display image mode */
#define NT35510_CMD_RDDSM             0x0E00  /* Read display signal mode */
#define NT35510_CMD_RDDSDR            0x0F00  /* Read display self-diagnostics result */
#define NT35510_CMD_SLPIN             0x1000  /* Sleep in */
#define NT35510_CMD_SLPOUT            0x1100  /* Sleep out */
#define NT35510_CMD_PTLON             0x1200  /* Partial mode on  */
#define NT35510_CMD_NORON             0x1300  /* Normal display mode on */
#define NT35510_CMD_INVOFF            0x2000  /* Display inversion off */
#define NT35510_CMD_INVON             0x2100  /* Display inversion on */
#define NT35510_CMD_ALLPOFF           0x2200  /* All pixel off */
#define NT35510_CMD_ALLPON            0x2300  /* All pixel on */
#define NT35510_CMD_GAMSET            0x2600  /* Gamma set */
#define NT35510_CMD_DISPOFF           0x2800  /* Display off */
#define NT35510_CMD_DISPON            0x2900  /* Display on */
#define NT35510_CMD_CASET             0x2A00  /* Column address set */
#define NT35510_CMD_RASET             0x2B00  /* Row address set */
#define NT35510_CMD_RAMWR             0x2C00  /* Memory write */
#define NT35510_CMD_RAMRD             0x2E00  /* Memory read  */
#define NT35510_CMD_PLTAR             0x3000  /* Partial area */
#define NT35510_CMD_TOPC              0x3200  /* Turn On Peripheral Command */
#define NT35510_CMD_TEOFF             0x3400  /* Tearing effect line off */
#define NT35510_CMD_TEEON             0x3500  /* Tearing effect line on */
#define NT35510_CMD_MADCTL            0x3600  /* Memory data access control */
#define NT35510_CMD_IDMOFF            0x3800  /* Idle mode off */
#define NT35510_CMD_IDMON             0x3900  /* Idle mode on */
#define NT35510_CMD_COLMOD            0x3A00  /* Interface pixel format */
#define NT35510_CMD_RAMWRC            0x3C00  /* Memory write continue */
#define NT35510_CMD_RAMRDC            0x3E00  /* Memory read continue */
#define NT35510_CMD_STESL             0x4400  /* Set tearing effect scan line */
#define NT35510_CMD_GSL               0x4500  /* Get scan line */
#define NT35510_CMD_DSTBON            0x4F00  /* Deep standby mode on */
#define NT35510_CMD_WRPFD             0x5000  /* Write profile value for display */
#define NT35510_CMD_WRDISBV           0x5100  /* Write display brightness */
#define NT35510_CMD_RDDISBV           0x5200  /* Read display brightness */
#define NT35510_CMD_WRCTRLD           0x5300  /* Write CTRL display */
#define NT35510_CMD_RDCTRLD           0x5400  /* Read CTRL display value */
#define NT35510_CMD_WRCABC            0x5500  /* Write content adaptative brightness control */
#define NT35510_CMD_RDCABC            0x5600  /* Read content adaptive brightness control */
#define NT35510_CMD_WRHYSTE           0x5700  /* Write hysteresis */
#define NT35510_CMD_WRGAMMSET         0x5800  /* Write gamme setting */
#define NT35510_CMD_RDFSVM            0x5A00  /* Read FS value MSBs */
#define NT35510_CMD_RDFSVL            0x5B00  /* Read FS value LSBs */
#define NT35510_CMD_RDMFFSVM          0x5C00  /* Read median filter FS value MSBs */
#define NT35510_CMD_RDMFFSVL          0x5D00  /* Read median filter FS value LSBs */
#define NT35510_CMD_WRCABCMB          0x5E00  /* Write CABC minimum brightness */
#define NT35510_CMD_RDCABCMB          0x5F00  /* Read CABC minimum brightness */
#define NT35510_CMD_WRLSCC            0x6500  /* Write light sensor compensation coefficient value */
#define NT35510_CMD_RDLSCCM           0x6600  /* Read light sensor compensation coefficient value MSBs */
#define NT35510_CMD_RDLSCCL           0x6700  /* Read light sensor compensation coefficient value LSBs */
#define NT35510_CMD_RDBWLB            0x7000  /* Read black/white low bits */
#define NT35510_CMD_RDBKX             0x7100 /* Read Bkx */
#define NT35510_CMD_RDBKY             0x7200  /* Read Bky */
#define NT35510_CMD_RDWX              0x7300  /* Read Wx */
#define NT35510_CMD_RDWY              0x7400  /* Read Wy */
#define NT35510_CMD_RDRGLB            0x7500  /* Read red/green low bits */
#define NT35510_CMD_RDRX              0x7600  /* Read Rx */
#define NT35510_CMD_RDRY              0x7700  /* Read Ry */
#define NT35510_CMD_RDGX              0x7800  /* Read Gx */
#define NT35510_CMD_RDGY              0x7900  /* Read Gy */
#define NT35510_CMD_RDBALB            0x7A00  /* Read blue/acolor low bits */
#define NT35510_CMD_RDBX              0x7B00  /* Read Bx */
#define NT35510_CMD_RDBY              0x7C00  /* Read By */
#define NT35510_CMD_RDAX              0x7D00  /* Read Ax */
#define NT35510_CMD_RDAY              0x7E00  /* Read Ay */
#define NT35510_CMD_RDDDBS            0xA100  /* Read DDB start */
#define NT35510_CMD_RDDDBC            0xA800  /* Read DDB continue */
#define NT35510_CMD_RDDCS             0xAA00  /* Read first checksum */
#define NT35510_CMD_RDCCS             0xAF00  /* Read continue checksum */
#define NT35510_CMD_RDID1             0xDA00  /* Read ID1 value */
#define NT35510_CMD_RDID2             0xDB00  /* Read ID2 value */
#define NT35510_CMD_RDID3             0xDC00  /* Read ID3 value */


/* IO Operation Function -----------------------------------------------------*/

/**
 * @brief nt35510 write command
 * 
 * @param _cmd command to write
 */
static inline void NT35510_WriteCmd(__IO uint16_t _cmd) {
  NT35510->REG = _cmd;
}

/**
 * @brief nt35510 write data
 * 
 * @param _data data to write
 */
static inline void NT35510_WriteData(__IO uint16_t _data) {
  NT35510->RAM = _data;
}

/**
 * @brief nt35510 read data
 * 
 * @return uint16_t data of read
 */
static inline uint16_t NT35510_ReadData(void) {
  return NT35510->RAM;
}

/**
 * @brief writes to selected register
 * 
 * @param reg address of register
 * @param reg_val value to write to the selected register
 */
static inline void NT35510_WriteReg(__IO uint16_t reg, __IO uint16_t reg_val) {
  NT35510->REG = reg;
  NT35510->RAM = reg_val;
}

/**
 * @brief reads the selected register
 * 
 * @param reg address of register
 * @return uint16_t register value
 */
static inline uint16_t NT35510_ReadReg(__IO uint16_t reg) {
  NT35510->REG = reg;
  return NT35510->RAM;
}

/**
 * @brief write multi data
 * 
 * @param buffer pointer of write buffer
 * @param length length of data
 */
static void NT35510_WriteMultiData(uint16_t *buffer, uint32_t length) {
  for (int i = 0; i < length; i++) {
    NT35510->RAM = *buffer++;
  }
}


/* NT35510 Operation Function -----------------------------------------------------*/

/**
 * @brief NT35510 init function
 * 
 * @param disp_dir NT35510_DISP_PORTRAIT | NT35510_DISP_LANDSCAPE
 */
void NT35510_Init(NT35510_DispDirEnum disp_dir) {
  uint32_t read_id = NT35510_ReadID();
  NT35510_LOG("LCD Controller Manufacture ID: %4x\r\n", read_id); 
  if (read_id != 0x8000) {
    NT35510_LOG("[ERROR]: Manufacture ID Error!\r\n");
    return;
  }
  NT35510_LOG("LCD Controller Model: NT35510\r\n");
  nt35510_params.id = 0x5510;

  NT35510_Delay(10);
  // LV1 Page 1 enable
  NT35510_WriteReg(0xF000, 0x55);
  NT35510_WriteReg(0xF001, 0xAA);
  NT35510_WriteReg(0xF002, 0x52);
  NT35510_WriteReg(0xF003, 0x08);
  NT35510_WriteReg(0xF004, 0x01);

  // AVDD Set AVDD 5.2V
  NT35510_WriteReg(0xB000, 0x0D);
  NT35510_WriteReg(0xB001, 0x0D);
  NT35510_WriteReg(0xB002, 0x0D);

  // AVDD ratio
  NT35510_WriteReg(0xB600, 0x34);
  NT35510_WriteReg(0xB601, 0x34);
  NT35510_WriteReg(0xB602, 0x34);

  //AVEE -5.2V
  NT35510_WriteReg(0xB100, 0x0D);
  NT35510_WriteReg(0xB101, 0x0D);
  NT35510_WriteReg(0xB102, 0x0D);

  /*AVEE ratio old
  NT35510_WriteReg(0xB700, 0x34);   
  NT35510_WriteReg(0xB701, 0x34);
  NT35510_WriteReg(0xB702, 0x34);
  */
  //AVEE ratio
  NT35510_WriteReg(0xB700, 0x35);   
  NT35510_WriteReg(0xB701, 0x35);
  NT35510_WriteReg(0xB702, 0x35);

  //VCL -2.5V
  NT35510_WriteReg(0xB200, 0x00);
  NT35510_WriteReg(0xB201, 0x00);
  NT35510_WriteReg(0xB202, 0x00);

  //VCL ratio
  NT35510_WriteReg(0xB800, 0x24);
  NT35510_WriteReg(0xB801, 0x24);
  NT35510_WriteReg(0xB802, 0x24);

  /*VGH 15V (Free pump) old
  NT35510_WriteReg(0xBF00, 0x01);
  NT35510_WriteReg(0xB300, 0x0F);
  NT35510_WriteReg(0xB301, 0x0F);
  NT35510_WriteReg(0xB302, 0x0F);
  */
  //VGH 15V (Free pump) new
  NT35510_WriteReg(0xBF00, 0x01);
  NT35510_WriteReg(0xB300, 0x08);
  NT35510_WriteReg(0xB301, 0x08);
  NT35510_WriteReg(0xB302, 0x08);

  //VGH ratio
  NT35510_WriteReg(0xB900, 0x34);
  NT35510_WriteReg(0xB901, 0x34);
  NT35510_WriteReg(0xB902, 0x34);

  //VGL_REG -10V
  NT35510_WriteReg(0xB500, 0x08);
  NT35510_WriteReg(0xB501, 0x08);
  NT35510_WriteReg(0xB502, 0x08);
  NT35510_WriteReg(0xC200, 0x03);

  //VGLX ratio
  NT35510_WriteReg(0xBA00, 0x24);
  NT35510_WriteReg(0xBA01, 0x24);
  NT35510_WriteReg(0xBA02, 0x24);

  /*VGMP/VGSP 4.5V/0V
  NT35510_WriteReg(0xBC00, 0x00);
  NT35510_WriteReg(0xBC01, 0x78);
  NT35510_WriteReg(0xBC02, 0x00);
  */
  //VGMP/VGSP 4.7V/0V
  NT35510_WriteReg(0xBC00, 0x00);
  NT35510_WriteReg(0xBC01, 0x88);
  NT35510_WriteReg(0xBC02, 0x00);


  /*VGMN/VGSN -4.5V/0V
  NT35510_WriteReg(0xBD00, 0x00);
  NT35510_WriteReg(0xBD01, 0x78);
  NT35510_WriteReg(0xBD02, 0x00);
  */
  //VGMN/VGSN -4.7V/0V
  NT35510_WriteReg(0xBD00, 0x00);
  NT35510_WriteReg(0xBD01, 0x88);
  NT35510_WriteReg(0xBD02, 0x00);

  /*VCOM old
  NT35510_WriteReg(0xBE00, 0x00);
  NT35510_WriteReg(0xBE01, 0x64);
  */
  //VCOM -2.0375V
  NT35510_WriteReg(0xBE00, 0x00);
  NT35510_WriteReg(0xBE01, 0xA3);

  //Gamma Setting

  #if NT35510_UPDATE_GAMMA

  #if NT35510_GAMMA_CFG_GROUP == 1

  NT35510_WriteReg(0xD100, 0x00);
  NT35510_WriteReg(0xD101, 0x33);
  NT35510_WriteReg(0xD102, 0x00);
  NT35510_WriteReg(0xD103, 0x34);
  NT35510_WriteReg(0xD104, 0x00);
  NT35510_WriteReg(0xD105, 0x3A);
  NT35510_WriteReg(0xD106, 0x00);
  NT35510_WriteReg(0xD107, 0x4A);
  NT35510_WriteReg(0xD108, 0x00);
  NT35510_WriteReg(0xD109, 0x5C);
  NT35510_WriteReg(0xD10A, 0x00);
  NT35510_WriteReg(0xD10B, 0x81);
  NT35510_WriteReg(0xD10C, 0x00);
  NT35510_WriteReg(0xD10D, 0xA6);
  NT35510_WriteReg(0xD10E, 0x00);
  NT35510_WriteReg(0xD10F, 0xE5);
  NT35510_WriteReg(0xD110, 0x01);
  NT35510_WriteReg(0xD111, 0x13);
  NT35510_WriteReg(0xD112, 0x01);
  NT35510_WriteReg(0xD113, 0x54);
  NT35510_WriteReg(0xD114, 0x01);
  NT35510_WriteReg(0xD115, 0x82);
  NT35510_WriteReg(0xD116, 0x01);
  NT35510_WriteReg(0xD117, 0xCA);
  NT35510_WriteReg(0xD118, 0x02);
  NT35510_WriteReg(0xD119, 0x00);
  NT35510_WriteReg(0xD11A, 0x02);
  NT35510_WriteReg(0xD11B, 0x01);
  NT35510_WriteReg(0xD11C, 0x02);
  NT35510_WriteReg(0xD11D, 0x34);
  NT35510_WriteReg(0xD11E, 0x02);
  NT35510_WriteReg(0xD11F, 0x67);
  NT35510_WriteReg(0xD120, 0x02);
  NT35510_WriteReg(0xD121, 0x84);
  NT35510_WriteReg(0xD122, 0x02);
  NT35510_WriteReg(0xD123, 0xA4);
  NT35510_WriteReg(0xD124, 0x02);
  NT35510_WriteReg(0xD125, 0xB7);
  NT35510_WriteReg(0xD126, 0x02);
  NT35510_WriteReg(0xD127, 0xCF);
  NT35510_WriteReg(0xD128, 0x02);
  NT35510_WriteReg(0xD129, 0xDE);
  NT35510_WriteReg(0xD12A, 0x02);
  NT35510_WriteReg(0xD12B, 0xF2);
  NT35510_WriteReg(0xD12C, 0x02);
  NT35510_WriteReg(0xD12D, 0xFE);
  NT35510_WriteReg(0xD12E, 0x03);
  NT35510_WriteReg(0xD12F, 0x10);
  NT35510_WriteReg(0xD130, 0x03);
  NT35510_WriteReg(0xD131, 0x33);
  NT35510_WriteReg(0xD132, 0x03);
  NT35510_WriteReg(0xD133, 0x6D);

  NT35510_WriteReg(0xD200, 0x00);
  NT35510_WriteReg(0xD201, 0x33);
  NT35510_WriteReg(0xD202, 0x00);
  NT35510_WriteReg(0xD203, 0x34);
  NT35510_WriteReg(0xD204, 0x00);
  NT35510_WriteReg(0xD205, 0x3A);
  NT35510_WriteReg(0xD206, 0x00);
  NT35510_WriteReg(0xD207, 0x4A);
  NT35510_WriteReg(0xD208, 0x00);
  NT35510_WriteReg(0xD209, 0x5C);
  NT35510_WriteReg(0xD20A, 0x00);
  NT35510_WriteReg(0xD20B, 0x81);
  NT35510_WriteReg(0xD20C, 0x00);
  NT35510_WriteReg(0xD20D, 0xA6);
  NT35510_WriteReg(0xD20E, 0x00);
  NT35510_WriteReg(0xD20F, 0xE5);
  NT35510_WriteReg(0xD210, 0x01);
  NT35510_WriteReg(0xD211, 0x13);
  NT35510_WriteReg(0xD212, 0x01);
  NT35510_WriteReg(0xD213, 0x54);
  NT35510_WriteReg(0xD214, 0x01);
  NT35510_WriteReg(0xD215, 0x82);
  NT35510_WriteReg(0xD216, 0x01);
  NT35510_WriteReg(0xD217, 0xCA);
  NT35510_WriteReg(0xD218, 0x02);
  NT35510_WriteReg(0xD219, 0x00);
  NT35510_WriteReg(0xD21A, 0x02);
  NT35510_WriteReg(0xD21B, 0x01);
  NT35510_WriteReg(0xD21C, 0x02);
  NT35510_WriteReg(0xD21D, 0x34);
  NT35510_WriteReg(0xD21E, 0x02);
  NT35510_WriteReg(0xD21F, 0x67);
  NT35510_WriteReg(0xD220, 0x02);
  NT35510_WriteReg(0xD221, 0x84);
  NT35510_WriteReg(0xD222, 0x02);
  NT35510_WriteReg(0xD223, 0xA4);
  NT35510_WriteReg(0xD224, 0x02);
  NT35510_WriteReg(0xD225, 0xB7);
  NT35510_WriteReg(0xD226, 0x02);
  NT35510_WriteReg(0xD227, 0xCF);
  NT35510_WriteReg(0xD228, 0x02);
  NT35510_WriteReg(0xD229, 0xDE);
  NT35510_WriteReg(0xD22A, 0x02);
  NT35510_WriteReg(0xD22B, 0xF2);
  NT35510_WriteReg(0xD22C, 0x02);
  NT35510_WriteReg(0xD22D, 0xFE);
  NT35510_WriteReg(0xD22E, 0x03);
  NT35510_WriteReg(0xD22F, 0x10);
  NT35510_WriteReg(0xD230, 0x03);
  NT35510_WriteReg(0xD231, 0x33);
  NT35510_WriteReg(0xD232, 0x03);
  NT35510_WriteReg(0xD233, 0x6D);

  NT35510_WriteReg(0xD300, 0x00);
  NT35510_WriteReg(0xD301, 0x33);
  NT35510_WriteReg(0xD302, 0x00);
  NT35510_WriteReg(0xD303, 0x34);
  NT35510_WriteReg(0xD304, 0x00);
  NT35510_WriteReg(0xD305, 0x3A);
  NT35510_WriteReg(0xD306, 0x00);
  NT35510_WriteReg(0xD307, 0x4A);
  NT35510_WriteReg(0xD308, 0x00);
  NT35510_WriteReg(0xD309, 0x5C);
  NT35510_WriteReg(0xD30A, 0x00);
  NT35510_WriteReg(0xD30B, 0x81);
  NT35510_WriteReg(0xD30C, 0x00);
  NT35510_WriteReg(0xD30D, 0xA6);
  NT35510_WriteReg(0xD30E, 0x00);
  NT35510_WriteReg(0xD30F, 0xE5);
  NT35510_WriteReg(0xD310, 0x01);
  NT35510_WriteReg(0xD311, 0x13);
  NT35510_WriteReg(0xD312, 0x01);
  NT35510_WriteReg(0xD313, 0x54);
  NT35510_WriteReg(0xD314, 0x01);
  NT35510_WriteReg(0xD315, 0x82);
  NT35510_WriteReg(0xD316, 0x01);
  NT35510_WriteReg(0xD317, 0xCA);
  NT35510_WriteReg(0xD318, 0x02);
  NT35510_WriteReg(0xD319, 0x00);
  NT35510_WriteReg(0xD31A, 0x02);
  NT35510_WriteReg(0xD31B, 0x01);
  NT35510_WriteReg(0xD31C, 0x02);
  NT35510_WriteReg(0xD31D, 0x34);
  NT35510_WriteReg(0xD31E, 0x02);
  NT35510_WriteReg(0xD31F, 0x67);
  NT35510_WriteReg(0xD320, 0x02);
  NT35510_WriteReg(0xD321, 0x84);
  NT35510_WriteReg(0xD322, 0x02);
  NT35510_WriteReg(0xD323, 0xA4);
  NT35510_WriteReg(0xD324, 0x02);
  NT35510_WriteReg(0xD325, 0xB7);
  NT35510_WriteReg(0xD326, 0x02);
  NT35510_WriteReg(0xD327, 0xCF);
  NT35510_WriteReg(0xD328, 0x02);
  NT35510_WriteReg(0xD329, 0xDE);
  NT35510_WriteReg(0xD32A, 0x02);
  NT35510_WriteReg(0xD32B, 0xF2);
  NT35510_WriteReg(0xD32C, 0x02);
  NT35510_WriteReg(0xD32D, 0xFE);
  NT35510_WriteReg(0xD32E, 0x03);
  NT35510_WriteReg(0xD32F, 0x10);
  NT35510_WriteReg(0xD330, 0x03);
  NT35510_WriteReg(0xD331, 0x33);
  NT35510_WriteReg(0xD332, 0x03);
  NT35510_WriteReg(0xD333, 0x6D);

  NT35510_WriteReg(0xD400, 0x00);
  NT35510_WriteReg(0xD401, 0x33);
  NT35510_WriteReg(0xD402, 0x00);
  NT35510_WriteReg(0xD403, 0x34);
  NT35510_WriteReg(0xD404, 0x00);
  NT35510_WriteReg(0xD405, 0x3A);
  NT35510_WriteReg(0xD406, 0x00);
  NT35510_WriteReg(0xD407, 0x4A);
  NT35510_WriteReg(0xD408, 0x00);
  NT35510_WriteReg(0xD409, 0x5C);
  NT35510_WriteReg(0xD40A, 0x00);
  NT35510_WriteReg(0xD40B, 0x81);
  NT35510_WriteReg(0xD40C, 0x00);
  NT35510_WriteReg(0xD40D, 0xA6);
  NT35510_WriteReg(0xD40E, 0x00);
  NT35510_WriteReg(0xD40F, 0xE5);
  NT35510_WriteReg(0xD410, 0x01);
  NT35510_WriteReg(0xD411, 0x13);
  NT35510_WriteReg(0xD412, 0x01);
  NT35510_WriteReg(0xD413, 0x54);
  NT35510_WriteReg(0xD414, 0x01);
  NT35510_WriteReg(0xD415, 0x82);
  NT35510_WriteReg(0xD416, 0x01);
  NT35510_WriteReg(0xD417, 0xCA);
  NT35510_WriteReg(0xD418, 0x02);
  NT35510_WriteReg(0xD419, 0x00);
  NT35510_WriteReg(0xD41A, 0x02);
  NT35510_WriteReg(0xD41B, 0x01);
  NT35510_WriteReg(0xD41C, 0x02);
  NT35510_WriteReg(0xD41D, 0x34);
  NT35510_WriteReg(0xD41E, 0x02);
  NT35510_WriteReg(0xD41F, 0x67);
  NT35510_WriteReg(0xD420, 0x02);
  NT35510_WriteReg(0xD421, 0x84);
  NT35510_WriteReg(0xD422, 0x02);
  NT35510_WriteReg(0xD423, 0xA4);
  NT35510_WriteReg(0xD424, 0x02);
  NT35510_WriteReg(0xD425, 0xB7);
  NT35510_WriteReg(0xD426, 0x02);
  NT35510_WriteReg(0xD427, 0xCF);
  NT35510_WriteReg(0xD428, 0x02);
  NT35510_WriteReg(0xD429, 0xDE);
  NT35510_WriteReg(0xD42A, 0x02);
  NT35510_WriteReg(0xD42B, 0xF2);
  NT35510_WriteReg(0xD42C, 0x02);
  NT35510_WriteReg(0xD42D, 0xFE);
  NT35510_WriteReg(0xD42E, 0x03);
  NT35510_WriteReg(0xD42F, 0x10);
  NT35510_WriteReg(0xD430, 0x03);
  NT35510_WriteReg(0xD431, 0x33);
  NT35510_WriteReg(0xD432, 0x03);
  NT35510_WriteReg(0xD433, 0x6D);

  NT35510_WriteReg(0xD500, 0x00);
  NT35510_WriteReg(0xD501, 0x33);
  NT35510_WriteReg(0xD502, 0x00);
  NT35510_WriteReg(0xD503, 0x34);
  NT35510_WriteReg(0xD504, 0x00);
  NT35510_WriteReg(0xD505, 0x3A);
  NT35510_WriteReg(0xD506, 0x00);
  NT35510_WriteReg(0xD507, 0x4A);
  NT35510_WriteReg(0xD508, 0x00);
  NT35510_WriteReg(0xD509, 0x5C);
  NT35510_WriteReg(0xD50A, 0x00);
  NT35510_WriteReg(0xD50B, 0x81);
  NT35510_WriteReg(0xD50C, 0x00);
  NT35510_WriteReg(0xD50D, 0xA6);
  NT35510_WriteReg(0xD50E, 0x00);
  NT35510_WriteReg(0xD50F, 0xE5);
  NT35510_WriteReg(0xD510, 0x01);
  NT35510_WriteReg(0xD511, 0x13);
  NT35510_WriteReg(0xD512, 0x01);
  NT35510_WriteReg(0xD513, 0x54);
  NT35510_WriteReg(0xD514, 0x01);
  NT35510_WriteReg(0xD515, 0x82);
  NT35510_WriteReg(0xD516, 0x01);
  NT35510_WriteReg(0xD517, 0xCA);
  NT35510_WriteReg(0xD518, 0x02);
  NT35510_WriteReg(0xD519, 0x00);
  NT35510_WriteReg(0xD51A, 0x02);
  NT35510_WriteReg(0xD51B, 0x01);
  NT35510_WriteReg(0xD51C, 0x02);
  NT35510_WriteReg(0xD51D, 0x34);
  NT35510_WriteReg(0xD51E, 0x02);
  NT35510_WriteReg(0xD51F, 0x67);
  NT35510_WriteReg(0xD520, 0x02);
  NT35510_WriteReg(0xD521, 0x84);
  NT35510_WriteReg(0xD522, 0x02);
  NT35510_WriteReg(0xD523, 0xA4);
  NT35510_WriteReg(0xD524, 0x02);
  NT35510_WriteReg(0xD525, 0xB7);
  NT35510_WriteReg(0xD526, 0x02);
  NT35510_WriteReg(0xD527, 0xCF);
  NT35510_WriteReg(0xD528, 0x02);
  NT35510_WriteReg(0xD529, 0xDE);
  NT35510_WriteReg(0xD52A, 0x02);
  NT35510_WriteReg(0xD52B, 0xF2);
  NT35510_WriteReg(0xD52C, 0x02);
  NT35510_WriteReg(0xD52D, 0xFE);
  NT35510_WriteReg(0xD52E, 0x03);
  NT35510_WriteReg(0xD52F, 0x10);
  NT35510_WriteReg(0xD530, 0x03);
  NT35510_WriteReg(0xD531, 0x33);
  NT35510_WriteReg(0xD532, 0x03);
  NT35510_WriteReg(0xD533, 0x6D);

  NT35510_WriteReg(0xD600, 0x00);
  NT35510_WriteReg(0xD601, 0x33);
  NT35510_WriteReg(0xD602, 0x00);
  NT35510_WriteReg(0xD603, 0x34);
  NT35510_WriteReg(0xD604, 0x00);
  NT35510_WriteReg(0xD605, 0x3A);
  NT35510_WriteReg(0xD606, 0x00);
  NT35510_WriteReg(0xD607, 0x4A);
  NT35510_WriteReg(0xD608, 0x00);
  NT35510_WriteReg(0xD609, 0x5C);
  NT35510_WriteReg(0xD60A, 0x00);
  NT35510_WriteReg(0xD60B, 0x81);
  NT35510_WriteReg(0xD60C, 0x00);
  NT35510_WriteReg(0xD60D, 0xA6);
  NT35510_WriteReg(0xD60E, 0x00);
  NT35510_WriteReg(0xD60F, 0xE5);
  NT35510_WriteReg(0xD610, 0x01);
  NT35510_WriteReg(0xD611, 0x13);
  NT35510_WriteReg(0xD612, 0x01);
  NT35510_WriteReg(0xD613, 0x54);
  NT35510_WriteReg(0xD614, 0x01);
  NT35510_WriteReg(0xD615, 0x82);
  NT35510_WriteReg(0xD616, 0x01);
  NT35510_WriteReg(0xD617, 0xCA);
  NT35510_WriteReg(0xD618, 0x02);
  NT35510_WriteReg(0xD619, 0x00);
  NT35510_WriteReg(0xD61A, 0x02);
  NT35510_WriteReg(0xD61B, 0x01);
  NT35510_WriteReg(0xD61C, 0x02);
  NT35510_WriteReg(0xD61D, 0x34);
  NT35510_WriteReg(0xD61E, 0x02);
  NT35510_WriteReg(0xD61F, 0x67);
  NT35510_WriteReg(0xD620, 0x02);
  NT35510_WriteReg(0xD621, 0x84);
  NT35510_WriteReg(0xD622, 0x02);
  NT35510_WriteReg(0xD623, 0xA4);
  NT35510_WriteReg(0xD624, 0x02);
  NT35510_WriteReg(0xD625, 0xB7);
  NT35510_WriteReg(0xD626, 0x02);
  NT35510_WriteReg(0xD627, 0xCF);
  NT35510_WriteReg(0xD628, 0x02);
  NT35510_WriteReg(0xD629, 0xDE);
  NT35510_WriteReg(0xD62A, 0x02);
  NT35510_WriteReg(0xD62B, 0xF2);
  NT35510_WriteReg(0xD62C, 0x02);
  NT35510_WriteReg(0xD62D, 0xFE);
  NT35510_WriteReg(0xD62E, 0x03);
  NT35510_WriteReg(0xD62F, 0x10);
  NT35510_WriteReg(0xD630, 0x03);
  NT35510_WriteReg(0xD631, 0x33);
  NT35510_WriteReg(0xD632, 0x03);
  NT35510_WriteReg(0xD633, 0x6D);
  
  #endif

  #if NT35510_GAMMA_CFG_GROUP == 2
  /* Reference: NT35510 Application Notes*/

  NT35510_WriteReg(0xD100, 0x00);
  NT35510_WriteReg(0xD101, 0x05);     
  NT35510_WriteReg(0xD102, 0x00);
  NT35510_WriteReg(0xD103, 0x40);     
  NT35510_WriteReg(0xD104, 0x00);
  NT35510_WriteReg(0xD105, 0x6D);     
  NT35510_WriteReg(0xD106, 0x00);
  NT35510_WriteReg(0xD107, 0x90);     
  NT35510_WriteReg(0xD108, 0x00);
  NT35510_WriteReg(0xD109, 0x99);  
  NT35510_WriteReg(0xD10A, 0x00);
  NT35510_WriteReg(0xD10B, 0xBB);     
  NT35510_WriteReg(0xD10C, 0x00);
  NT35510_WriteReg(0xD10D, 0xDC);     
  NT35510_WriteReg(0xD10E, 0x01);     
  NT35510_WriteReg(0xD10F, 0x04);     
  NT35510_WriteReg(0xD110, 0x01);
  NT35510_WriteReg(0xD111, 0x25);     
  NT35510_WriteReg(0xD112, 0x01);
  NT35510_WriteReg(0xD113, 0x59);     
  NT35510_WriteReg(0xD114, 0x01);
  NT35510_WriteReg(0xD115, 0x82);
  NT35510_WriteReg(0xD116, 0x01);
  NT35510_WriteReg(0xD117, 0xC6);     
  NT35510_WriteReg(0xD118, 0x02);
  NT35510_WriteReg(0xD119, 0x01);     
  NT35510_WriteReg(0xD11A, 0x02);
  NT35510_WriteReg(0xD11B, 0x02);     
  NT35510_WriteReg(0xD11C, 0x02);
  NT35510_WriteReg(0xD11D, 0x39);     
  NT35510_WriteReg(0xD11E, 0x02);
  NT35510_WriteReg(0xD11F, 0x79);     
  NT35510_WriteReg(0xD120, 0x02);
  NT35510_WriteReg(0xD121, 0xA1);
  NT35510_WriteReg(0xD122, 0x02);
  NT35510_WriteReg(0xD123, 0xD9);
  NT35510_WriteReg(0xD124, 0x03);
  NT35510_WriteReg(0xD125, 0x00);
  NT35510_WriteReg(0xD126, 0x03);
  NT35510_WriteReg(0xD127, 0x38);
  NT35510_WriteReg(0xD128, 0x03);
  NT35510_WriteReg(0xD129, 0x67);
  NT35510_WriteReg(0xD12A, 0x03);
  NT35510_WriteReg(0xD12B, 0x8F);
  NT35510_WriteReg(0xD12C, 0x03);
  NT35510_WriteReg(0xD12D, 0xCD);
  NT35510_WriteReg(0xD12E, 0x03);
  NT35510_WriteReg(0xD12F, 0xFD);
  NT35510_WriteReg(0xD130, 0x03);
  NT35510_WriteReg(0xD131, 0xFE);
  NT35510_WriteReg(0xD132, 0x03);
  NT35510_WriteReg(0xD133, 0xFF);

  NT35510_WriteReg(0xD200, 0x00);
  NT35510_WriteReg(0xD201, 0x33);
  NT35510_WriteReg(0xD202, 0x00);
  NT35510_WriteReg(0xD203, 0x34);
  NT35510_WriteReg(0xD204, 0x00);
  NT35510_WriteReg(0xD205, 0x3A);
  NT35510_WriteReg(0xD206, 0x00);
  NT35510_WriteReg(0xD207, 0x90);
  NT35510_WriteReg(0xD208, 0x00);
  NT35510_WriteReg(0xD209, 0x99);
  NT35510_WriteReg(0xD20A, 0x00);
  NT35510_WriteReg(0xD20B, 0xBB);
  NT35510_WriteReg(0xD20C, 0x00);
  NT35510_WriteReg(0xD20D, 0xDC);
  NT35510_WriteReg(0xD20E, 0x01);
  NT35510_WriteReg(0xD20F, 0x04);
  NT35510_WriteReg(0xD210, 0x01);
  NT35510_WriteReg(0xD211, 0x25);
  NT35510_WriteReg(0xD212, 0x01);
  NT35510_WriteReg(0xD213, 0x59);
  NT35510_WriteReg(0xD214, 0x01);
  NT35510_WriteReg(0xD215, 0x82);
  NT35510_WriteReg(0xD216, 0x01);
  NT35510_WriteReg(0xD217, 0xC6);
  NT35510_WriteReg(0xD218, 0x02);
  NT35510_WriteReg(0xD219, 0x01);
  NT35510_WriteReg(0xD21A, 0x02);
  NT35510_WriteReg(0xD21B, 0x02);
  NT35510_WriteReg(0xD21C, 0x02);
  NT35510_WriteReg(0xD21D, 0x39);
  NT35510_WriteReg(0xD21E, 0x02);
  NT35510_WriteReg(0xD21F, 0x79);
  NT35510_WriteReg(0xD220, 0x02);
  NT35510_WriteReg(0xD221, 0xA1);
  NT35510_WriteReg(0xD222, 0x02);
  NT35510_WriteReg(0xD223, 0xD9);
  NT35510_WriteReg(0xD224, 0x03);
  NT35510_WriteReg(0xD225, 0x00);
  NT35510_WriteReg(0xD226, 0x03);
  NT35510_WriteReg(0xD227, 0x38);
  NT35510_WriteReg(0xD228, 0x03);
  NT35510_WriteReg(0xD229, 0x67);
  NT35510_WriteReg(0xD22A, 0x03);
  NT35510_WriteReg(0xD22B, 0x8F);
  NT35510_WriteReg(0xD22C, 0x03);
  NT35510_WriteReg(0xD22D, 0xCD);
  NT35510_WriteReg(0xD22E, 0x03);
  NT35510_WriteReg(0xD22F, 0xFD);
  NT35510_WriteReg(0xD230, 0x03);
  NT35510_WriteReg(0xD231, 0xFE);
  NT35510_WriteReg(0xD232, 0x03);
  NT35510_WriteReg(0xD233, 0xFF);

  NT35510_WriteReg(0xD300, 0x00);
  NT35510_WriteReg(0xD301, 0x33);
  NT35510_WriteReg(0xD302, 0x00);
  NT35510_WriteReg(0xD303, 0x34);
  NT35510_WriteReg(0xD304, 0x00);
  NT35510_WriteReg(0xD305, 0x3A);
  NT35510_WriteReg(0xD306, 0x00);
  NT35510_WriteReg(0xD307, 0x90);
  NT35510_WriteReg(0xD308, 0x00);
  NT35510_WriteReg(0xD309, 0x99);
  NT35510_WriteReg(0xD30A, 0x00);
  NT35510_WriteReg(0xD30B, 0xBB);
  NT35510_WriteReg(0xD30C, 0x00);
  NT35510_WriteReg(0xD30D, 0xDC);
  NT35510_WriteReg(0xD30E, 0x01);
  NT35510_WriteReg(0xD30F, 0x04);
  NT35510_WriteReg(0xD310, 0x01);
  NT35510_WriteReg(0xD311, 0x25);
  NT35510_WriteReg(0xD312, 0x01);
  NT35510_WriteReg(0xD313, 0x59);
  NT35510_WriteReg(0xD314, 0x01);
  NT35510_WriteReg(0xD315, 0x82);
  NT35510_WriteReg(0xD316, 0x01);
  NT35510_WriteReg(0xD317, 0xC6);
  NT35510_WriteReg(0xD318, 0x02);
  NT35510_WriteReg(0xD319, 0x01);
  NT35510_WriteReg(0xD31A, 0x02);
  NT35510_WriteReg(0xD31B, 0x02);
  NT35510_WriteReg(0xD31C, 0x02);
  NT35510_WriteReg(0xD31D, 0x39);
  NT35510_WriteReg(0xD31E, 0x02);
  NT35510_WriteReg(0xD31F, 0x79);
  NT35510_WriteReg(0xD320, 0x02);
  NT35510_WriteReg(0xD321, 0xA1);
  NT35510_WriteReg(0xD322, 0x02);
  NT35510_WriteReg(0xD323, 0xD9);
  NT35510_WriteReg(0xD324, 0x03);
  NT35510_WriteReg(0xD325, 0x00);
  NT35510_WriteReg(0xD326, 0x03);
  NT35510_WriteReg(0xD327, 0x38);
  NT35510_WriteReg(0xD328, 0x03);
  NT35510_WriteReg(0xD329, 0x67);
  NT35510_WriteReg(0xD32A, 0x03);
  NT35510_WriteReg(0xD32B, 0x8F);
  NT35510_WriteReg(0xD32C, 0x03);
  NT35510_WriteReg(0xD32D, 0xCD);
  NT35510_WriteReg(0xD32E, 0x03);
  NT35510_WriteReg(0xD32F, 0xFD);
  NT35510_WriteReg(0xD330, 0x03);
  NT35510_WriteReg(0xD331, 0xFE);
  NT35510_WriteReg(0xD332, 0x03);
  NT35510_WriteReg(0xD333, 0xFF);

  NT35510_WriteReg(0xD400, 0x00);
  NT35510_WriteReg(0xD401, 0x33);
  NT35510_WriteReg(0xD402, 0x00);
  NT35510_WriteReg(0xD403, 0x34);
  NT35510_WriteReg(0xD404, 0x00);
  NT35510_WriteReg(0xD405, 0x3A);
  NT35510_WriteReg(0xD406, 0x00);
  NT35510_WriteReg(0xD407, 0x90);
  NT35510_WriteReg(0xD408, 0x00);
  NT35510_WriteReg(0xD409, 0x99);
  NT35510_WriteReg(0xD40A, 0x00);
  NT35510_WriteReg(0xD40B, 0xBB);
  NT35510_WriteReg(0xD40C, 0x00);
  NT35510_WriteReg(0xD40D, 0xDC);
  NT35510_WriteReg(0xD40E, 0x01);
  NT35510_WriteReg(0xD40F, 0x04);
  NT35510_WriteReg(0xD410, 0x01);
  NT35510_WriteReg(0xD411, 0x25);
  NT35510_WriteReg(0xD412, 0x01);
  NT35510_WriteReg(0xD413, 0x59);
  NT35510_WriteReg(0xD414, 0x01);
  NT35510_WriteReg(0xD415, 0x82);
  NT35510_WriteReg(0xD416, 0x01);
  NT35510_WriteReg(0xD417, 0xC6);
  NT35510_WriteReg(0xD418, 0x02);
  NT35510_WriteReg(0xD419, 0x01);
  NT35510_WriteReg(0xD41A, 0x02);
  NT35510_WriteReg(0xD41B, 0x02);
  NT35510_WriteReg(0xD41C, 0x02);
  NT35510_WriteReg(0xD41D, 0x39);
  NT35510_WriteReg(0xD41E, 0x02);
  NT35510_WriteReg(0xD41F, 0x79);
  NT35510_WriteReg(0xD420, 0x02);
  NT35510_WriteReg(0xD421, 0xA1);
  NT35510_WriteReg(0xD422, 0x02);
  NT35510_WriteReg(0xD423, 0xD9);
  NT35510_WriteReg(0xD424, 0x03);
  NT35510_WriteReg(0xD425, 0x00);
  NT35510_WriteReg(0xD426, 0x03);
  NT35510_WriteReg(0xD427, 0x38);
  NT35510_WriteReg(0xD428, 0x03);
  NT35510_WriteReg(0xD429, 0x67);
  NT35510_WriteReg(0xD42A, 0x03);
  NT35510_WriteReg(0xD42B, 0x8F);
  NT35510_WriteReg(0xD42C, 0x03);
  NT35510_WriteReg(0xD42D, 0xCD);
  NT35510_WriteReg(0xD42E, 0x03);
  NT35510_WriteReg(0xD42F, 0xFD);
  NT35510_WriteReg(0xD430, 0x03);
  NT35510_WriteReg(0xD431, 0xFE);
  NT35510_WriteReg(0xD432, 0x03);
  NT35510_WriteReg(0xD433, 0xFF);

  NT35510_WriteReg(0xD500, 0x00);
  NT35510_WriteReg(0xD501, 0x33);
  NT35510_WriteReg(0xD502, 0x00);
  NT35510_WriteReg(0xD503, 0x34);
  NT35510_WriteReg(0xD504, 0x00);
  NT35510_WriteReg(0xD505, 0x3A);
  NT35510_WriteReg(0xD506, 0x00);
  NT35510_WriteReg(0xD507, 0x90);
  NT35510_WriteReg(0xD508, 0x00);
  NT35510_WriteReg(0xD509, 0x99);
  NT35510_WriteReg(0xD50A, 0x00);
  NT35510_WriteReg(0xD50B, 0xBB);
  NT35510_WriteReg(0xD50C, 0x00);
  NT35510_WriteReg(0xD50D, 0xDC);
  NT35510_WriteReg(0xD50E, 0x01);
  NT35510_WriteReg(0xD50F, 0x04);
  NT35510_WriteReg(0xD510, 0x01);
  NT35510_WriteReg(0xD511, 0x25);
  NT35510_WriteReg(0xD512, 0x01);
  NT35510_WriteReg(0xD513, 0x59);
  NT35510_WriteReg(0xD514, 0x01);
  NT35510_WriteReg(0xD515, 0x82);
  NT35510_WriteReg(0xD516, 0x01);
  NT35510_WriteReg(0xD517, 0xC6);
  NT35510_WriteReg(0xD518, 0x02);
  NT35510_WriteReg(0xD519, 0x01);
  NT35510_WriteReg(0xD51A, 0x02);
  NT35510_WriteReg(0xD51B, 0x02);
  NT35510_WriteReg(0xD51C, 0x02);
  NT35510_WriteReg(0xD51D, 0x39);
  NT35510_WriteReg(0xD51E, 0x02);
  NT35510_WriteReg(0xD51F, 0x79);
  NT35510_WriteReg(0xD520, 0x02);
  NT35510_WriteReg(0xD521, 0xA1);
  NT35510_WriteReg(0xD522, 0x02);
  NT35510_WriteReg(0xD523, 0xD9);
  NT35510_WriteReg(0xD524, 0x03);
  NT35510_WriteReg(0xD525, 0x00);
  NT35510_WriteReg(0xD526, 0x03);
  NT35510_WriteReg(0xD527, 0x38);
  NT35510_WriteReg(0xD528, 0x03);
  NT35510_WriteReg(0xD529, 0x67);
  NT35510_WriteReg(0xD52A, 0x03);
  NT35510_WriteReg(0xD52B, 0x8F);
  NT35510_WriteReg(0xD52C, 0x03);
  NT35510_WriteReg(0xD52D, 0xCD);
  NT35510_WriteReg(0xD52E, 0x03);
  NT35510_WriteReg(0xD52F, 0xFD);
  NT35510_WriteReg(0xD530, 0x03);
  NT35510_WriteReg(0xD531, 0xFE);
  NT35510_WriteReg(0xD532, 0x03);
  NT35510_WriteReg(0xD533, 0xFF);

  NT35510_WriteReg(0xD600, 0x00);
  NT35510_WriteReg(0xD601, 0x33);
  NT35510_WriteReg(0xD602, 0x00);
  NT35510_WriteReg(0xD603, 0x34);
  NT35510_WriteReg(0xD604, 0x00);
  NT35510_WriteReg(0xD605, 0x3A);
  NT35510_WriteReg(0xD606, 0x00);
  NT35510_WriteReg(0xD607, 0x90);
  NT35510_WriteReg(0xD608, 0x00);
  NT35510_WriteReg(0xD609, 0x99);
  NT35510_WriteReg(0xD60A, 0x00);
  NT35510_WriteReg(0xD60B, 0xBB);
  NT35510_WriteReg(0xD60C, 0x00);
  NT35510_WriteReg(0xD60D, 0xDC);
  NT35510_WriteReg(0xD60E, 0x01);
  NT35510_WriteReg(0xD60F, 0x04);
  NT35510_WriteReg(0xD610, 0x01);
  NT35510_WriteReg(0xD611, 0x25);
  NT35510_WriteReg(0xD612, 0x01);
  NT35510_WriteReg(0xD613, 0x59);
  NT35510_WriteReg(0xD614, 0x01);
  NT35510_WriteReg(0xD615, 0x82);
  NT35510_WriteReg(0xD616, 0x01);
  NT35510_WriteReg(0xD617, 0xC6);
  NT35510_WriteReg(0xD618, 0x02);
  NT35510_WriteReg(0xD619, 0x01);
  NT35510_WriteReg(0xD61A, 0x02);
  NT35510_WriteReg(0xD61B, 0x02);
  NT35510_WriteReg(0xD61C, 0x02);
  NT35510_WriteReg(0xD61D, 0x39);
  NT35510_WriteReg(0xD61E, 0x02);
  NT35510_WriteReg(0xD61F, 0x79);
  NT35510_WriteReg(0xD620, 0x02);
  NT35510_WriteReg(0xD621, 0xA1);
  NT35510_WriteReg(0xD622, 0x02);
  NT35510_WriteReg(0xD623, 0xD9);
  NT35510_WriteReg(0xD624, 0x03);
  NT35510_WriteReg(0xD625, 0x00);
  NT35510_WriteReg(0xD626, 0x03);
  NT35510_WriteReg(0xD627, 0x38);
  NT35510_WriteReg(0xD628, 0x03);
  NT35510_WriteReg(0xD629, 0x67);
  NT35510_WriteReg(0xD62A, 0x03);
  NT35510_WriteReg(0xD62B, 0x8F);
  NT35510_WriteReg(0xD62C, 0x03);
  NT35510_WriteReg(0xD62D, 0xCD);
  NT35510_WriteReg(0xD62E, 0x03);
  NT35510_WriteReg(0xD62F, 0xFD);
  NT35510_WriteReg(0xD630, 0x03);
  NT35510_WriteReg(0xD631, 0xFE);
  NT35510_WriteReg(0xD632, 0x03);
  NT35510_WriteReg(0xD633, 0xFF);

  #endif
  #endif

  // LV2 Page 0 enable
  NT35510_WriteReg(0xF000, 0x55);
  NT35510_WriteReg(0xF001, 0xAA);
  NT35510_WriteReg(0xF002, 0x52);
  NT35510_WriteReg(0xF003, 0x08);
  NT35510_WriteReg(0xF004, 0x00);
  // Display control
  NT35510_WriteReg(0xB100, 0xCC);
  NT35510_WriteReg(0xB101, 0x00);
  // Source hold time
  NT35510_WriteReg(0xB600, 0x05);
  // Gate EQ control
  NT35510_WriteReg(0xB700, 0x70);
  NT35510_WriteReg(0xB701, 0x70);
  // Source EQ control (Mode 2)
  NT35510_WriteReg(0xB800, 0x01);
  NT35510_WriteReg(0xB801, 0x03);
  NT35510_WriteReg(0xB802, 0x03);
  NT35510_WriteReg(0xB803, 0x03);
  // Inversion mode (2-dot)
  NT35510_WriteReg(0xBC00, 0x02);
  NT35510_WriteReg(0xBC01, 0x00);
  NT35510_WriteReg(0xBC02, 0x00);
  // Timing control 4H w/ 4-delay
  NT35510_WriteReg(0xC900, 0xD0);
  NT35510_WriteReg(0xC901, 0x02);
  NT35510_WriteReg(0xC902, 0x50);
  NT35510_WriteReg(0xC903, 0x50);
  NT35510_WriteReg(0xC904, 0x50);
  NT35510_WriteReg(0x3500, 0x00);
  NT35510_WriteReg(0x3A00, 0x55);  // 16-bit/pixel
  // Sleep out
  NT35510_WriteCmd(0x1100);
  NT35510_Delay(1);
  // Display on
  NT35510_WriteCmd(0x2900);
  NT35510_SetDispDirection(disp_dir);
  NT35510_SetTextColor(LCD_COLOR_BLACK, LCD_COLOR_WHITE);
  NT35510_BacklightON();
  NT35510_LOG("LCD Controller Initialize Success\r\n");
}

/**
 * @brief Read NT35510 Manufacture ID
 * 
 * @return uint16_t NT35510 Manufacture ID: 0x8000
 */
uint16_t NT35510_ReadID(void) {
  uint16_t read_id;
  read_id  = NT35510_ReadReg(NT35510_CMD_RDID1);
  read_id  = NT35510_ReadReg(NT35510_CMD_RDID2) << 8;
  read_id |= NT35510_ReadReg(NT35510_CMD_RDID3);
  return read_id;
}

/**
 * @brief Set LCD display on
 * 
 */
void NT35510_DisplayON(void) {
  NT35510_WriteCmd(NT35510_CMD_DISPON);
}

/**
 * @brief Set LCD display off
 * 
 */
void NT35510_DisplayOFF(void) {
  NT35510_WriteCmd(NT35510_CMD_DISPOFF);
}

/**
 * @brief Set LCD cursor
 * 
 * @param xs column address
 * @param ys row    address
 */
void NT35510_SetCursor(uint16_t xs, uint16_t ys) {
  NT35510_WriteReg(NT35510_CMD_CASET, xs >> 8);
  NT35510_WriteReg(NT35510_CMD_CASET + 1, xs & 0xFF);
  NT35510_WriteReg(NT35510_CMD_RASET, ys >> 8);
  NT35510_WriteReg(NT35510_CMD_RASET + 1, ys & 0xFF);
}

/**
 * @brief Set LCD end cursor
 * 
 * @param xe column address
 * @param ye row    address
 */
void NT35510_SetCursorEnd(uint16_t xe, uint16_t ye) {
  NT35510_WriteReg(NT35510_CMD_CASET + 2, xe >> 8);
  NT35510_WriteReg(NT35510_CMD_CASET + 3, xe & 0xFF);
  NT35510_WriteReg(NT35510_CMD_RASET + 2, ye >> 8);
  NT35510_WriteReg(NT35510_CMD_RASET + 3, ye & 0xFF);
}

/**
 * @brief Sets a display window
 * 
 * @param x specifies the X bottom left position
 * @param y specifies the Y bottom left position
 * @param width  display window width
 * @param height display window height
 */
void NT35510_SetWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
  NT35510_SetCursor(x, y);
  NT35510_SetCursorEnd(x + width - 1, y + height - 1);
}

/**
 * @brief Draw a point
 * 
 * @param x specifies the X position
 * @param y specifies the Y position
 * @param color the RGB pixel color
 */
void NT35510_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
  /*
  if (x >= nt35510_params.width || y >= nt35510_params.height) return;
  */
  NT35510_SetCursor(x, y);
  NT35510_WriteReg(NT35510_CMD_RAMWR, color);
}

/**
 * @brief Clear the full display by selected color
 * 
 * @param color the RGB pixel color
 */
void NT35510_Clear(uint16_t color) {
  uint32_t totalPixel;
  totalPixel = nt35510_params.width * nt35510_params.height;

  NT35510_SetWindow(0, 0, nt35510_params.width, nt35510_params.height);
  NT35510_WriteCmd(NT35510_CMD_RAMWR);
  for (uint32_t i = 0; i < totalPixel; i++) {
    NT35510_WriteData(color);
  }
}

/**
 * @brief Sets LCD scan direction
 * 
 * @param dir Scan direction
 */
void NT35510_SetScanDirection(NT35510_ScanDirEnum scan_dir) {
  uint16_t reg_val = 0;
  uint16_t tmp;

  if (nt35510_params.direction == NT35510_DISP_LANDSCAPE) {
    switch (scan_dir) {
      case L2R_U2D:   scan_dir = D2U_L2R;    break;
      case L2R_D2U:   scan_dir = D2U_R2L;    break;
      case R2L_U2D:   scan_dir = U2D_L2R;    break;
      case R2L_D2U:   scan_dir = U2D_R2L;    break;   
      case U2D_L2R:   scan_dir = L2R_D2U;    break;
      case U2D_R2L:   scan_dir = L2R_U2D;    break;
      case D2U_L2R:   scan_dir = R2L_D2U;    break;
      case D2U_R2L:   scan_dir = R2L_U2D;    break;   
    }
  }
  switch (scan_dir) {
    case L2R_U2D:
      reg_val |= (0<<7)|(0<<6)|(0<<5); 
      break;
    case L2R_D2U:
      reg_val |= (1<<7)|(0<<6)|(0<<5); 
      break;
    case R2L_U2D:
      reg_val |= (0<<7)|(1<<6)|(0<<5); 
      break;
    case R2L_D2U:
      reg_val |= (1<<7)|(1<<6)|(0<<5); 
      break;   
    case U2D_L2R:
      reg_val |= (0<<7)|(0<<6)|(1<<5); 
      break;
    case U2D_R2L:
      reg_val |= (0<<7)|(1<<6)|(1<<5); 
      break;
    case D2U_L2R:
      reg_val |= (1<<7)|(0<<6)|(1<<5); 
      break;
    case D2U_R2L:
      reg_val |= (1<<7)|(1<<6)|(1<<5); 
      break;   
  }
  NT35510_WriteReg(NT35510_CMD_MADCTL, reg_val);

  if (reg_val & 0x20) {
    if (nt35510_params.width < nt35510_params.height) {
      tmp = nt35510_params.width;
      nt35510_params.width  = nt35510_params.height;
      nt35510_params.height = tmp;
    }
  }
  else {
    if (nt35510_params.width > nt35510_params.height) {
      tmp = nt35510_params.width;
      nt35510_params.width  = nt35510_params.height;
      nt35510_params.height = tmp;
    }
  }
  NT35510_SetWindow(0, 0, nt35510_params.width, nt35510_params.height);
  NT35510_LOG("LCD Display Width  Set: %u\r\n", nt35510_params.width);
  NT35510_LOG("LCD Display Height Set: %u\r\n", nt35510_params.height);
}

/**
 * @brief set lcd display direction
 * 
 * @param disp_dir NT35510_DISP_PORTRAIT | NT35510_DISP_LANDSCAPE
 */
void NT35510_SetDispDirection(NT35510_DispDirEnum disp_dir) {
  if (disp_dir == NT35510_DISP_PORTRAIT) {
    nt35510_params.direction = NT35510_DISP_PORTRAIT;
    nt35510_params.width     = NT35510_WIDTH; 
    nt35510_params.height    = NT35510_HEIGHT; 
  }
  else {
    nt35510_params.direction = NT35510_DISP_LANDSCAPE;
    nt35510_params.width     = NT35510_HEIGHT; 
    nt35510_params.height    = NT35510_WIDTH; 
  }

  NT35510_SetScanDirection(L2R_U2D);
}

/**
 * @brief write a char to lcd display
 * 
 * @param x specifies X postion
 * @param y specifies Y postion
 * @param ch char
 * @param font text font
 */
void NT35510_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font) {
  uint16_t i, j ,b;

  NT35510_SetWindow(x, y, font.width, font.height);
  NT35510_WriteCmd(NT35510_CMD_RAMWR);

  for (i = 0; i < font.height; i++) {
    b = font.data[(ch - 32) * font.height + i];
    for (j = 0; j < font.width; j++) {
      if ((b << j) & 0x8000)  {
        NT35510_WriteData(nt35510_params.textcolor);
      } else {
        NT35510_WriteData(nt35510_params.backcolor);
      }
    }
  }
}

/**
 * @brief write string to lcd display
 * 
 * @param x specifies x postion
 * @param y specifies y postion
 * @param str pointer of string
 * @param font text font
 */
void NT35510_WriteString(uint16_t x, uint16_t y, char *str, FontDef font) {
  if (!str) return;

  uint16_t x_pos = x;
  uint16_t y_pos = y;

  while (*str) {
    NT35510_WriteChar(x_pos, y_pos, *str++, font);
    x_pos += font.width;
    if (x_pos + font.width >= nt35510_params.width) {
      y_pos += font.height;
      x_pos = x;
    }
  }
}

/**
 * @brief set lcd display text color and background color
 * 
 * @param text_color specifies the text color
 * @param back_color specifies the background color
 */
void NT35510_SetTextColor(uint16_t text_color, uint16_t back_color) {
  nt35510_params.textcolor = text_color;
  nt35510_params.backcolor = back_color;
}

/**
 * @brief draw vertical line.
 * 
 * @param x specifies the X position 
 * @param y specifies the Y position
 * @param len specifies the Line length
 * @param color specifies the RGB color
 */
void NT35510_DrawVLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
  len = MIN(len, nt35510_params.height - y);

  for (int i = 0; i < len; i++) {
    NT35510_DrawPoint(x, y + i, color);
  }
}

/**
 * @brief draw horizontal line
 * 
 * @param x specifies the X position 
 * @param y specifies the Y position 
 * @param len specifies the Line length
 * @param color specifies the RGB color
 */
void NT35510_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
  len = MIN(len, nt35510_params.width - x);

  NT35510_SetWindow(x, y, len, 1);
  NT35510_WriteCmd(NT35510_CMD_RAMWR);

  for (int i = 0; i < len; i++) {
    NT35510_WriteData(color);
  }
}

/**
 * @brief draw arbitrary slope line (Bresenham's line algorithm)
 * 
 * @param xs specifies the X start position
 * @param ys specifies the Y start position
 * @param xe specifies the X end position
 * @param ye specifies the Y end position
 * @param color specifies the RGB color
 */
void NT35510_DrawLine(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color) {

  if (xs == xe) return NT35510_DrawVLine(xs, MIN(ys, ye), ABS((int)ys - (int)ye), color);
  if (ys == ye) return NT35510_DrawHLine(MIN(xs, xe), ys, ABS((int)xs - (int)xe), color);

  /* references: http://members.chello.at/easyfilter/bresenham.html */

  int dx  = ABS(xe - xs);
  int dy  = ABS(ye - ys);
  int sx  = xs < xe ? 1 : -1;
  int sy  = ys < ye ? 1 : -1;
  int err = dx - dy;
  int e2; /* error value e_xy */

  while (xs != xe || ys != ye) {
    NT35510_DrawPoint(xs, ys, color);
    e2 = 2 * err;
    if (e2 >= -dy) {err -= dy; xs += sx;}
    if (e2 <=  dx) {err += dx; ys += sy;}
  } 
}

/**
 * @brief draw point of circle
 * 
 * @param x specifies the circle center X position
 * @param y specifies the circle center Y position
 * @param a position offset
 * @param b position offset
 */
static inline void NT35510_DrawCirclePoint(uint16_t x, uint16_t y, uint16_t a, uint16_t b, uint16_t color) {
  NT35510_DrawPoint(x + a, y + b, color);
  NT35510_DrawPoint(x + a, y - b, color);
  NT35510_DrawPoint(x - a, y + b, color);
  NT35510_DrawPoint(x - a, y - b, color);
  NT35510_DrawPoint(x + b, y + a, color);
  NT35510_DrawPoint(x + b, y - a, color);
  NT35510_DrawPoint(x - b, y + a, color);
  NT35510_DrawPoint(x - b, y - a, color);
}

/**
 * @brief draw circle
 * 
 * @param x specifies the circle center X position
 * @param y specifies the circle center Y position
 * @param r specifies the circle radius
 * @param color specifies the RGB color
 */
void NT35510_DrawCircle(uint16_t x, uint16_t y, uint16_t r, uint16_t color) {
  int a = 0,  b = r;
  int d = 3 - 2 * r;

  while (a <= b) {
    NT35510_DrawCirclePoint(x, y, a, b, color);
    if(d < 0) {
      d += 4 * a + 6;
    }
    else {
      d += 4 * (a - b) + 10;
      b--;
    }
    a++;
  }
}

/**
 * @brief draw a solid circles
 * 
 * @param x specifies the circle center X position
 * @param y specifies the circle center Y position
 * @param r specifies the circle radius
 * @param color specifies the RGB color
 */
void NT35510_DrawCircleFill(uint16_t x, uint16_t y, uint16_t r, uint16_t color) {
  int a = 0,  b = r;
  int d = 3 - 2 * r;

  while(a <= b)
  {
    for (int i = a; i <= b; i++) {
      NT35510_DrawCirclePoint(x, y, a, i, color);
    }
    if(d < 0) {
      d += 4 * a + 6;
    }
    else
    {
      d += 4 * (a - b) + 10;
      b--;
    }
    a++;
  }
}

/**
 * @brief draw rectangle
 * 
 * @param x1 specifies the x1 position
 * @param y1 specifies the y1 position
 * @param x2 specifies the x2 position
 * @param y2 specifies the y2 position
 * @param color specifies the RGB color
 */
void NT35510_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  NT35510_DrawLine(x1, y1, x2, y1, color);
  NT35510_DrawLine(x2, y1, x2, y2, color);
  NT35510_DrawLine(x2, y2, x1, y2, color);
  NT35510_DrawLine(x1, y2, x1, y1, color);
}

/**
 * @brief draw a solid rectangle
 * 
 * @param x1 specifies the x1 position
 * @param y1 specifies the y1 position
 * @param x2 specifies the x2 position
 * @param y2 specifies the y2 position
 * @param color specifies the RGB color
 */
void NT35510_DrawRectangleFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  uint16_t width  = ABS(x1 - x2) + 1;
  uint16_t height = ABS(y1 - y2) + 1;

  NT35510_SetWindow(MIN(x1, x2), MIN(y1, y2), width, height);
  NT35510_WriteCmd(NT35510_CMD_RAMWR);
  for (int i = 0; i < width * height; i++) {
    NT35510_WriteData(color);
  }
}

/**
 * @brief draw RGB565 image
 * 
 * @param x specifies the X bottom left position
 * @param y specifies the Y bottom left position
 * @param width specifies the image width
 * @param height specifies the image height
 * @param buffer specifies the image color buffer
 */
void NT35510_DrawRGBImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *buffer) {
  if ((x + width) > nt35510_params.width || (y + height) > nt35510_params.height) {
    // LOG("image size over\n");
    return;
  }
  NT35510_SetWindow(x, y, width, height);
  NT35510_WriteCmd(NT35510_CMD_RAMWR);
  NT35510_WriteMultiData(buffer, width * height);
}

/**
 * @brief fill display area by specifies color buffer
 * 
 * @param x1 specifies the X bottom left position
 * @param y1 specifies the Y bottom left position
 * @param x2 specifies the X top right position
 * @param y2 specifies the Y top right position
 * @param color_p color buffer
 */
void NT35510_ColorFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color_p) {
  NT35510_SetCursor(x1, y1);
  NT35510_SetCursorEnd(x2, y2);
  NT35510_WriteCmd(NT35510_CMD_RAMWR);
  NT35510_WriteMultiData(color_p, (x2 - x1 + 1) * (y2 - y1 + 1));
}

/**
 * @brief read gram color data
 * 
 * @param x specifies the X position
 * @param y specifies the X position
 * @return uint16_t RGB565 color data of specifies point
 */
uint16_t NT35510_ReadPoint(uint16_t x, uint16_t y) {
  uint16_t rgb565;
  NT35510_SetWindow(x, y, 1, 1);
  NT35510_WriteCmd(NT35510_CMD_RAMRD);
  // dummy data
  for (int i = 0; i < 5; i++) {
    (void) NT35510_ReadData();
  }
  // read color data && convert to rgb565
  rgb565  = NT35510_ReadData();
  rgb565  = (rgb565 & 0xF800) | ((rgb565 & 0x00FC) << 3);
  rgb565 |= (NT35510_ReadData() & 0xF800) >> 11;
  return rgb565;
} 

