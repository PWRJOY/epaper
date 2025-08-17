/**
 * @file epaper_gui.h
 * @author liying (respire_ly@qq.com)
 * @brief 墨水屏GUI驱动
 * @version 1.0
 * @date 2025-04-18
 * 
 * @copyright Copyright (c) 2025  freelance
 * 
 * @par changelog:
 * | Date | Version | Author | Description |
 * | --- | --- | --- | --- |
 * | 2025-04-18 | 1.0 | liying | 墨水屏GUI驱动 |
 */

#ifndef EPAPER_GUI_H
#define EPAPER_GUI_H

#include "epaper.h" 

/**
 * @brief 结构体：画布，具有一系列属性
 * @details 以实际数据举例来解释 			\n
 * widthMemory = 180，heightMemory = 384  \n
 * widthByte = 180/4=45，heightByte = 384 \n
 * -------------------------- height	  \n
 * |									  \n
 * |									  \n
 * |									  \n
 * |									  \n
 * width								  \n
 * 										  \n
 * 每个"|"是4个像素，每个"-"是一个像素		
 */
typedef struct {
	uint8_t *Image;			/**< 画布像素数据 */
	uint16_t width;			/**< 记录旋转后的宽width */
	uint16_t height;		/**< 记录旋转后的高height */
	uint16_t widthMemory;	/**< 创建画布时的宽 */
	uint16_t heightMemory;	/**< 创建画布时的高 */
	uint16_t color;			/**< 颜色 */
	uint16_t rotate;		/**< 显示方向 */
	uint16_t widthByte;		/**< widthByte =  widthMemory/4，如果有余数再加一个字节来存储 */
	uint16_t heightByte;	/**< heightByte = heightMemory */
}PAINT;

extern PAINT Paint;

/** 
 * @def Rotation
 * @brief 墨水屏显示方向，可选0、90、180、270
*/
#define Rotation 0  

/** @brief  函数功能：创建画布 */
void Paint_NewImage(uint8_t *image,uint16_t Width,uint16_t Height,uint16_t Rotate,uint16_t Color); 

/** @brief  函数功能：设置某个坐标像素点的颜色 */
void Paint_SetPixel(uint16_t Xpoint,uint16_t Ypoint,uint16_t Color);

/** @brief  函数功能：清除画布 */
void Paint_Clear(uint8_t Color);

/** @brief  函数功能：画直线 */
void EPD_DrawLine(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color);

/** @brief  函数功能：画矩形*/
void EPD_DrawRectangle(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color,uint8_t mode);  

/** @brief  函数功能：画圆*/
void EPD_DrawCircle(uint16_t X_Center,uint16_t Y_Center,uint16_t Radius,uint16_t Color,uint8_t mode);      

/** @brief  函数功能：显示字符*/
void EPD_ShowChar(uint16_t x,uint16_t y,uint16_t chr,uint16_t size1,uint8_t fc,uint8_t bc);      

/** @brief  函数功能：显示字符串*/
void EPD_ShowString(uint16_t x,uint16_t y,uint8_t *chr,uint16_t size1,uint8_t fc,uint8_t bc);      

/** @brief  函数功能：显示数字*/
void EPD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint16_t len,uint16_t size1,uint8_t fc,uint8_t bc);    

/** @brief  函数功能：显示浮点数*/
void EPD_ShowFloatNum1(uint16_t x,uint16_t y,float num,uint8_t len,uint8_t pre,uint8_t sizey,uint8_t fc,uint8_t bc);

/** @brief  函数功能：显示时间*/
void EPD_ShowWatch(uint16_t x,uint16_t y,float num,uint8_t len,uint8_t pre,uint8_t sizey,uint8_t fc,uint8_t bc);

/** @brief  函数功能：显示图片*/
void EPD_ShowPicture(uint16_t x,uint16_t y,uint16_t sizex,uint16_t sizey,const uint8_t BMP[],uint16_t fc,uint16_t bc);	

/** @brief  函数功能：显示4色图片*/
void EPD_ShowFourColorPicture(uint16_t x,uint16_t y,uint16_t sizex,uint16_t sizey,const uint8_t BMP[]);

/** @brief  函数功能：显示汉字 */
void EPD_ShowChinese(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color);

/** @brief  函数功能：显示汉字，12*12字号 */
void EPD_ShowChinese12x12(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum);

/** @brief  函数功能：显示汉字，16*16字号 */
void EPD_ShowChinese16x16(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum);

/** @brief  函数功能：显示汉字，24*24字号 */
void EPD_ShowChinese24x24(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum);

/** @brief  函数功能：显示汉字，32*32字号 */
void EPD_ShowChinese32x32(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum);

/** @brief  函数功能：将位图绘制到缓冲区 */
void DrawBitmapToBuffer(uint16_t x, uint16_t y, const uint8_t *bitmap, uint8_t width, uint8_t height, uint16_t color);

// 函数功能：获取UTF-8字符长度
int get_utf8_char_length(uint8_t first_byte);


#endif