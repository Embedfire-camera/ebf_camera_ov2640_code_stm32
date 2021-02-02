#ifndef __BMP_SCREENSHOT_H
#define __BMP_SCREENSHOT_H

#include "stm32f4xx.h"

/* bmp文件头 */
typedef __packed struct
{
  uint16_t bfType;    //文件类型
  uint32_t bfSize;    //文件大小
  uint16_t bfReserved1;//保留字，不考虑
  uint16_t bfReserved2;//保留字，同上
  uint32_t bfOffBits; //实际位图数据的偏移字节数，即前三个部分长度之和
} BITMAPFILEHEADER;

/* bmp信息头 */
typedef __packed struct
{
  uint32_t biSize;       //指定此结构体的长度，为 40
  long biWidth;          //位图宽，说明本图的宽度，以像素为单位
  long biHeight;         //位图高，指明本图的高度，像素为单位
  uint16_t biPlanes;     //平面数，为 1
  uint16_t biBitCount;   //采用颜色位数，可以是 1，2，4，8，16，24 新的可以是 32
  uint32_t biCompression;//压缩方式，可以是 0，1，2，其中 0 表示不压缩
  uint32_t biSizeImage;  //实际位图数据占用的字节数
  long biXPelsPerMeter;  //X 方向分辨率，以像素/米为单位
  long biYPelsPerMeter;  //Y 方向分辨率，像素/米为单位
  uint32_t biClrUsed;    //使用的颜色数，如果为 0，则表示默认值(2^颜色位数)
  uint32_t biClrImportant; //重要颜色数，如果为 0，则表示所有颜色都是重要的
}BITMAPINFOHEADER;

/* 完整bmp头数据 */
typedef __packed struct
{
  BITMAPFILEHEADER bmp_FileHeader;
	BITMAPINFOHEADER bmp_InfoHeader;  
}BITMAPFULLDATA;


uint8_t screen_shot(uint8_t layer);

#endif/*__BMP_SCREENSHOT_H*/
