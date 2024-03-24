#include "lib_bmp.h"
#include "ff.h"
#include "lcd.h"
#include "string.h"

/* Warning: this lib only support little-end system */

/* Private Macro Definetion --------------------------------------------------*/

/* BMP Log function macro definetion */
#if BMP_USE_LOG == 1
#include <stdio.h>
#define BMP_LOG(...)        printf(__VA_ARGS__)
#else
#define BMP_LOG(...)        ((void)0U)
#endif

#if BMP_CREATE_ALWAYS == 1
#define FA_CREATE_CTRL      FA_CREATE_ALWAYS
#else
#define FA_CREATE_CTRL      FA_CREATE_NEW
#endif

/* BMP display port macro definetion */
#if BMP_USE_DISP_BUF == 1
#define BMP_DRAW_POINT(xs, ys, off_x, off_y, color)   bmp_draw_point(off_x, off_y, color)
#define BMP_FLUSH_BUFFER(xs, ys, off_y, sdir)         bmp_flush_buffer(xs, ys, off_y, sdir)
#else
#define BMP_DRAW_POINT(xs, ys, off_x, off_y, color)   bmp_draw_point(xs + off_x, ys + off_y, color)
#define BMP_FLUSH_BUFFER(xs, ys, off_y, sdir)         ((void)0U)
#endif

/* Caculate absolute value */
#define ABS(n)                    ((n) < 0 ? (-(n)) : ((n)))

/* RGB888 convert to RGB565 */
#define RGB888_TO_RGB565(color)   (((uint16_t)(color[0]) >> 3) |               \
                                  (((uint16_t)(color[1]) << 3) & 0x07E0) |     \
                                  (((uint16_t)(color[2]) << 8) & 0xF800))           

/* RGB555 convert to RGB565 */
#define RGB555_TO_RGB565(color)   ((*((uint16_t *)color) & 0x001F) |           \
                                  ((*((uint16_t *)color) & 0xFFE0) << 1))



/* Private Type Definetion ---------------------------------------------------*/

typedef struct {
  uint16_t width;       // width of bitmap image
  uint16_t height;      // height of bitmap image
  uint32_t bpr;         // number of bytes per row
  uint8_t  padding;     // number of padding bytes 
  uint8_t  bpp;         // number of bits per pixel
  uint8_t  compr;       // compression method
  uint8_t  scandir;     // bitmap data scan direction
}bmp_info_t;



/* Private variable definition --------------------------------------------------*/

/* File buffer for lib_bmp */
static uint8_t    bmp_file_buf[BMP_FILE_BUF_SIZE];

/* Display buffer for bmp decode */
#if BMP_USE_DISP_BUF == 1
static uint16_t   bmp_disp_buf[BMP_DISP_BUF_SIZE];
#endif

/* Other private var */
static FIL        f_bmp;
static FRESULT    f_res;
static bmp_info_t bmp_info;

/* Port function declaration -------------------------------------------------*/

#if BMP_USE_DISP_BUF == 1

/**
 * @brief fill color data to display window
 * 
 * @param x x start positon
 * @param y x start positon
 * @param w width of image
 * @param h height of image
 * @param pbuf buffer of color data
 */
static void bmp_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color) {

  /* add display port */
  lcd_drv->DrawRGBImage(x, y, w, h, color);

}

/**
 * @brief flush display buffer data to lcd display 
 * 
 * @param xs x start positon
 * @param ys y start positon
 * @param off_y y offset 
 * @param scandir scan direction
 */
static void bmp_flush_buffer(uint16_t xs, uint16_t ys, uint16_t off_y, uint8_t scandir) {
  uint16_t row_cnt;
  if (scandir == BMP_DIRECTION_D2U) {
    if (off_y % BMP_DISP_ROWS == 0) {
      row_cnt = (off_y + BMP_DISP_ROWS) > bmp_info.height ? (bmp_info.height - off_y) : BMP_DISP_ROWS;
      bmp_draw_image(xs, ys + off_y, bmp_info.width, row_cnt, bmp_disp_buf);
    }
  }
  else if (scandir == BMP_DIRECTION_U2D) {
    if ((off_y + 1) % BMP_DISP_ROWS == 0) {
      row_cnt = BMP_DISP_ROWS;
      bmp_draw_image(xs, ys + off_y - BMP_DISP_ROWS, bmp_info.width, row_cnt, bmp_disp_buf);
    }
    else if (off_y == (bmp_info.height - 1)) {
      row_cnt = bmp_info.height - (off_y / BMP_DISP_ROWS) * BMP_DISP_ROWS;
      bmp_draw_image(xs, (off_y / BMP_DISP_ROWS) * BMP_DISP_ROWS, bmp_info.width, row_cnt, bmp_disp_buf);
    }
  }
}

#endif

/**
 * @brief write point to display or display buffer
 * 
 * @param x x position
 * @param y y position
 * @param color color data
 */
static void bmp_draw_point(uint16_t x, uint16_t y, uint16_t color) {
  #if BMP_USE_DISP_BUF == 1

  y %= BMP_DISP_ROWS; 
  bmp_disp_buf[y * bmp_info.width + x] = color;
  
  #else

  /* add display port */
  lcd_drv->WritePixel(x, y, color);

  #endif
}


/* Exported function definition ----------------------------------------------*/

/**
 * @brief Decoder for bmp-format file, show bmp image on lcd display (RGB565)
 * 
 * @param filename bitmap file name
 * @param xs x position start
 * @param ys y position start
 * @param xe x position maximum
 * @param ye y position maximum
 * @return int BMP_OK on success | others on error
 */
int bmp_decode(uint8_t *filename, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye) {
   
  uint16_t row_cnt;           // number of rows  to read
  uint16_t btr;               // number of bytes to read
  uint32_t br;                // number of bytes read             
  uint16_t off_x, off_y;      // x, y offset of position
  uint16_t color;             // color data：RGB565 format 
  uint8_t *pbmp;              // pointer of bitmap data

  int i, j;
  int retval = BMP_OK;        // function return value

  BitmapHeader *bmp_header = (BitmapHeader *)bmp_file_buf;    // pointer of bitmap header  

  /* Open bmp file */
  f_res = f_open(&f_bmp, (const TCHAR*)filename, FA_READ);
  if (f_res != FR_OK) {
    BMP_LOG("[bmp decode] error: open file failed, ret code: %u\r\n", f_res);
    return BMP_ERR_FOPEN;
  }
  
  /* Read bmp header */
  f_res = f_read(&f_bmp, (uint8_t *)bmp_header, sizeof(BitmapHeader), &br);  // default: 54 byte
  if (f_res != FR_OK) {
    BMP_LOG("[bmp decode] error: read bitmap header failed, ret code: %u\r\n", f_res);
    retval = BMP_ERR_FREAD;
    goto decode_exit;
  }

  /* Check file type */
  if (bmp_header->FileHeader.FileType != 0x4D42) {
    BMP_LOG("[bmp decode] error: file type error, magic word: 0x%04X\r\n", bmp_header->FileHeader.FileType);
    retval = BMP_ERR_FTYPE;
    goto decode_exit;
  }

  /* Get bmp image info */
  bmp_info.width  = bmp_header->InfoHeader.Width;
  bmp_info.height = ABS(bmp_header->InfoHeader.Height);
  bmp_info.bpp    = bmp_header->InfoHeader.BitsPerPixel;
  bmp_info.compr  = bmp_header->InfoHeader.Compression;
  BMP_LOG("[bmp decode] bitmap info: width->%u height->%u bpp->%u compr->%u\r\n",
            bmp_info.width, bmp_info.height, bmp_info.bpp, bmp_info.compr);

  /* Check bpp ,only support 16bit | 24bit | 32bit bitmap */
  if (bmp_info.bpp != 16 && bmp_info.bpp != 24 && bmp_info.bpp != 32) {
    BMP_LOG("[bmp decode] error: color depth not supported, bpp: %ubit\r\n", bmp_info.bpp);
    retval = BMP_ERR_FTYPE;
    goto decode_exit;
  }

  /* Check image direction */
  bmp_info.scandir = bmp_header->InfoHeader.Height < 0 ? BMP_DIRECTION_U2D : BMP_DIRECTION_D2U;

  /* Check image size */
  if ((xs + bmp_info.width > xe) || (ys + bmp_info.height > ye)) {
    BMP_LOG("[bmp decode] error: image size out of display area\r\n");
    retval = BMP_ERR_ISIZE;
    goto decode_exit;
  }

  /* Caculate number of bytes per line and number of padding bytes*/
  uint32_t bpr_tmp = bmp_info.width * (bmp_info.bpp >> 3);     // caculate num of bytes per row (before aligned to 4-byte)
  bmp_info.bpr = (bpr_tmp + 3U) & (~3U);            // caculate num of bytes per row (aligned to 4-byte)
  bmp_info.padding = bmp_info.bpr - bpr_tmp;        // caculate padding bytes

  if (BMP_FILE_BUF_SIZE < bmp_info.bpr) {
    BMP_LOG("[bmp decode] error: bmp file buffer overflow\r\n");
    retval = BMP_ERR_FBUFF;
    goto decode_exit;
  }

  row_cnt = BMP_FILE_BUF_SIZE / bmp_info.bpr;      // caculate num of rows to read
  btr = row_cnt * bmp_info.bpr;                    // caculate num of bytes to read

  /* Init offset of pixel position */
  off_y = (bmp_info.scandir == BMP_DIRECTION_D2U) ? (bmp_info.height - 1) : 0;
  off_x = 0;


  /* Bitmap data decode */
  f_res = f_lseek(&f_bmp, bmp_header->FileHeader.BitmapOffset);
  if (f_res != FR_OK) {
    BMP_LOG("[bmp decode] error: file seek failed, ret code: %u\r\n", f_res);
    retval = BMP_ERR_FSEEK;
    goto decode_exit;
  }

  while (1) {
    /* Read bitmap data */
    f_res = f_read(&f_bmp, bmp_file_buf, btr, &br);
    if (f_res != FR_OK) {
      BMP_LOG("[bmp decode] error: read bitmap data failed, ret code: %u\r\n", f_res);
      retval = BMP_ERR_FREAD;
      goto decode_exit;
    }

    /* Last line */
    if (br != btr) {
      row_cnt = br / bmp_info.bpr;
    }

    pbmp = bmp_file_buf;
    /* 32bit bitmap */
    if (bmp_info.bpp == 32) { 
      for (j = 0; j < row_cnt; j++) {
        if (bmp_info.compr == BI_RGB) {
          for (i = 0; i < bmp_info.width; i++, off_x++, pbmp += 4) {
            color = RGB888_TO_RGB565(pbmp);
            BMP_DRAW_POINT(xs, ys, off_x, off_y, color);
          }
        }
        else {
          BMP_LOG("[bmp decode] error: compression method not supported\r\n");
          retval = BMP_ERR_FTYPE;
          goto decode_exit;
        }

        pbmp += bmp_info.padding;  // skip padding data
        off_x = 0;   // reset x offset and turn to next line

        /* Flush data to lcd */
        BMP_FLUSH_BUFFER(xs, ys, off_y, bmp_info.scandir);

        /* Increase y offset */
        off_y = (bmp_info.scandir == BMP_DIRECTION_D2U) ? (off_y - 1) : (off_y + 1);    
      }    
    }

    /* 24bit bitmap */
    else if (bmp_info.bpp == 24) {
      for (j = 0; j < row_cnt; j++) {
        for (i = 0; i < bmp_info.width; i++, off_x++, pbmp += 3) {
          color = RGB888_TO_RGB565(pbmp);
          BMP_DRAW_POINT(xs, ys, off_x, off_y, color);
        }
        pbmp += bmp_info.padding;  // skip padding data
        off_x = 0;   // reset x offset and turn to next line

        /* Flush data to lcd */
        BMP_FLUSH_BUFFER(xs, ys, off_y, bmp_info.scandir);

        /* Increase y offset */
        off_y = (bmp_info.scandir == BMP_DIRECTION_D2U) ? (off_y - 1) : (off_y + 1);        
      }
    }

    /* 16bit bitmap */
    else if (bmp_info.bpp == 16) {
      for (j = 0; j < row_cnt; j++) {
        /* RGB555 */
        if (bmp_info.compr == BI_RGB) {
          for (i = 0; i < bmp_info.width; i++, off_x++, pbmp += 2) {
            color = RGB555_TO_RGB565(pbmp);
            BMP_DRAW_POINT(xs, ys, off_x, off_y, color);    
          }
        }
        /* RGB565 */
        else if (bmp_info.compr == BI_BITFIELDS){
          for (i = 0; i < bmp_info.width; i++, off_x++, pbmp += 2) {
            color = *((uint16_t *)pbmp);
            BMP_DRAW_POINT(xs, ys, off_x, off_y, color);
          }
        }
        else {
          BMP_LOG("[bmp decode] error: compression method not supported\r\n");
          retval = BMP_ERR_FTYPE;
          goto decode_exit;
        }
        pbmp += bmp_info.padding;  // skip padding data
        off_x = 0;            // reset x offset and turn to next line

        /* Flush data to lcd */
        BMP_FLUSH_BUFFER(xs, ys, off_y, bmp_info.scandir);

        /* Increase y offset */
        off_y = (bmp_info.scandir == BMP_DIRECTION_D2U) ? (off_y - 1) : (off_y + 1); 
      }
    }

    if (br != btr) break;
  }

decode_exit:
  f_res = f_close(&f_bmp);
  if (f_res != FR_OK) {
    BMP_LOG("[bmp decode] error: close file failed, ret code: %u\r\n", f_res);
    retval = (retval == BMP_OK) ? BMP_ERR_FCLOSE : retval;
  }
  return retval;  
}

/**
 * @brief encoding color data to bitmap file
 * 
 * @param filename name of bitmap file
 * @param width width of bitmap image
 * @param height height of bitmap image
 * @param mbuf pointer of color data buffer
 * @return int BMP_OK on success | Others on error
 */
int bmp_encode(uint8_t *filename, uint16_t width, uint16_t height, uint16_t* mbuf) {
  BitmapHeaderWithMask *bmp_header;   // pointer of bitmap header
  uint16_t *bitmap_data;

  bmp_header = (BitmapHeaderWithMask *)bmp_file_buf;
  memset((void *)bmp_header, 0, sizeof(BitmapHeaderWithMask));

  int retval = BMP_OK;
  int x, y;
  uint32_t bw;
  uint32_t bpr = width << 1;      // caculate num of bytes per row (before aligned to 4-byte)
  bpr = (bpr + 3U) & (~3U);       // caculate num of bytes per row (aligned to 4-byte)

  bmp_header->InfoHeader.InfoHeaderSize = sizeof(BitmapInfoHeader);
  bmp_header->InfoHeader.Width  = width;
  bmp_header->InfoHeader.Height = height;
  bmp_header->InfoHeader.Planes = 1;
  bmp_header->InfoHeader.BitsPerPixel = 16;
  bmp_header->InfoHeader.Compression = BI_BITFIELDS;
  bmp_header->InfoHeader.SizeOfBitmap = bpr * height;

  bmp_header->FileHeader.FileType = 0x4D42;
  bmp_header->FileHeader.FileSize = sizeof(BitmapHeaderWithMask) + bmp_header->InfoHeader.SizeOfBitmap;
  bmp_header->FileHeader.BitmapOffset = sizeof(BitmapHeaderWithMask);

  bmp_header->RgbMask.RedMask   = 0X00F800;
  bmp_header->RgbMask.GreenMask = 0X0007E0;
  bmp_header->RgbMask.BlueMask  = 0X00001F;

  f_res = f_open(&f_bmp, (const TCHAR *)filename, FA_WRITE | FA_CREATE_CTRL);
  if (f_res != FR_OK) {
    BMP_LOG("[bmp encode] error: create bmp file failed, ret code: %u\r\n", f_res);
    return BMP_ERR_FOPEN;
  }

  f_res = f_lseek(&f_bmp, 0);
  if (f_res != FR_OK) {
    BMP_LOG("[bmp encode] error: file seek failed, ret code: %u\r\n", f_res);
    retval = BMP_ERR_FSEEK;
    goto encode_exit;
  }

  // Write bitmap header
  f_res = f_write(&f_bmp, (const void*)bmp_header, sizeof(BitmapHeaderWithMask), &bw);
  if (f_res != FR_OK) {
    BMP_LOG("[bmp encode] error: write bitmap header failed, ret code: %u\r\n", f_res);
    retval = BMP_ERR_FWRITE;
    goto encode_exit;
  }

  // Write bitmap data
  bitmap_data = (uint16_t *)bmp_file_buf;
  for (y = height - 1; y >= 0; y--) {
    for (x = 0; x != (bpr >> 1); x++) {
        bitmap_data[x] = x < width ? mbuf[y * width + x] : 0xFFFF; 
    }
    f_res = f_write(&f_bmp, (uint8_t *)bitmap_data, bpr, &bw);
    if (f_res != FR_OK) {
      BMP_LOG("[bmp encode] error: write bitmap data failed, ret code: %u\r\n", f_res);
      retval = BMP_ERR_FWRITE;
      goto encode_exit;
    }
  }

encode_exit:
  f_res = f_close(&f_bmp);
  if (f_res != FR_OK) {
    BMP_LOG("[bmp encode] error: close file failed, ret code: %u\r\n", f_res);
    retval = (retval == BMP_OK) ? BMP_ERR_FCLOSE : retval;
  }
  return retval;
}
