#ifndef __LIB_BMP_H__
#define __LIB_BMP_H__

#include <stdint.h>

/* Export macro definition ---------------------------------------------------*/

// if define 1, will enable print log
#define BMP_USE_LOG             1

// if define 1, will enable display buffer
#define BMP_USE_DISP_BUF        1

// if define 1, when the file is existing, it will be truncated and overwritten by encode function
#define BMP_CREATE_ALWAYS       0

// display resolution
#define BMP_DISP_HOR_RES        240U
#define BMP_DISP_VER_RES        320U

// row count of display buffer
#define BMP_DISP_ROWS           320U

// bmp file buffer size (default: max bytes of per row)
#define BMP_FILE_BUF_SIZE       (BMP_DISP_HOR_RES * 4)

// bmp disp buffer size
#define BMP_DISP_BUF_SIZE       (BMP_DISP_HOR_RES * BMP_DISP_ROWS)


/* Export type declaration ---------------------------------------------------*/

// lib_bmp error code
typedef enum {
  BMP_OK,
  BMP_ERROR,
  BMP_ERR_FOPEN,
  BMP_ERR_FREAD,
  BMP_ERR_FWRITE,
  BMP_ERR_FSEEK,
  BMP_ERR_FCLOSE,
  BMP_ERR_FTYPE,
  BMP_ERR_ISIZE,
  BMP_ERR_FBUFF,
}BMP_ErrCode;

// bitmap data scan direction 
typedef enum {
  BMP_DIRECTION_D2U,
  BMP_DIRECTION_U2D,
}BMP_DispDirection;

// bitmap data compression method
typedef enum {
  BI_RGB,
  BI_RLE8,
  BI_RLE4,
  BI_BITFIELDS,
  BI_JPEG,
  BI_PNG,
}BMP_Compression;

// bitmap file header
typedef __unaligned struct __attribute__((packed)) {
  uint16_t FileType;            // File type, always 4D42h ("BM") 
  uint32_t FileSize;            // Size of the file in bytes
  uint16_t Reserved1;           // Always 0
  uint16_t Reserved2;           // Always 0
  uint32_t BitmapOffset;        // Starting position of image data in bytes
} BitmapFileHeader;

// bitmap info header
typedef __unaligned struct __attribute__((packed)) {
  uint32_t InfoHeaderSize;      // Size of this header in bytes (=40)
  int32_t  Width;               // Image width in pixels
  int32_t  Height;              // Image height in pixels
  uint16_t Planes;              // Number of color plane (always=1)
  uint16_t BitsPerPixel;        // Number of bits per pixel
  uint32_t Compression;         // Compression methods used 0 = BI_RGB no compression | 1 = BI_RLE8 8bit RLE encoding | 2 = BI_RLE4 4bit RLE encoding
  uint32_t SizeOfBitmap;        // Size of bitmap in bytes, It is valid to set this =0 if Compression = 0
  int32_t  HorzResolution;      // Horizontal resolution in pixels per meter
  int32_t  VertResolution;      // Vertical resolution in pixels per meter
  uint32_t ColorsUsed;          // Number of colors in the image, For a 8-bit / pixel bitmap this will be 100h or 256.
  uint32_t ColorsImportant;     // Minimum number of important colors 0 = all
} BitmapInfoHeader;

// bitmap palette
typedef __unaligned struct __attribute__((packed)) {
  uint8_t Blue;                 // Blue component
  uint8_t Green;                // Green component
  uint8_t Red;                  // Red component
  uint8_t Reserved;             // Padding (always 0)
} BitmapPaletteElement;

// bitmap color bitfield mask
typedef __unaligned struct __attribute__((packed)) {
  uint32_t RedMask;             // Mask identifying bits of red component
  uint32_t GreenMask;           // Mask identifying bits of green component
  uint32_t BlueMask;            // Mask identifying bits of blue component
} BitmapBitfieldsMasks;


typedef __unaligned struct __attribute__((packed)) {
  BitmapFileHeader FileHeader;
  BitmapInfoHeader InfoHeader;
} BitmapHeader;

typedef __unaligned struct __attribute__((packed)) {
  BitmapFileHeader FileHeader;
  BitmapInfoHeader InfoHeader;
  BitmapBitfieldsMasks RgbMask;
} BitmapHeaderWithMask;

int bmp_decode(uint8_t *filename, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
int bmp_encode(uint8_t *filename, uint16_t width, uint16_t height, uint16_t* mbuf);


#endif