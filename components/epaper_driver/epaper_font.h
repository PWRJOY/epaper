/**
 * @file epaper_font.h
 * @author liying (respire_ly@qq.com)
 * @brief ASCII字符和汉字取模的字库
 * @version 1.0
 * @date 2025-04-12
 * 
 * @copyright Copyright (c) 2025  freelance
 * 
 * @par changelog:
 * | Date | Version | Author | Description |
 * | --- | --- | --- | --- |
 * | 2025-04-12 | 1.0 | liying | ASCII字符和汉字取模的字库 |
 * 
 * @par  取模说明
 * 取模软件：PCtoLCD2002<br>
 * 点阵格式：阴码<br>
 * 点阵方向：逆向(低位在前，LSB)<br>
 * 取模方式：<br>
 * <div style="margin-left: 20px;">
 * ASCII字符：列行式<br>
 * 汉字：逐行式
 * </div>
 */

#ifndef _EPD_FONT_H_
#define _EPD_FONT_H_

// ASCII字符集
extern const unsigned char asc2_0806[][6];
extern const unsigned char asc2_1206[][12];
extern const unsigned char asc2_1608[][16];
extern const unsigned char asc2_2412[][36];
extern const unsigned char asc2_4824[][144];



// 12*12 汉字字库结构体
typedef struct 
{
	unsigned char Index[4];	  /**< 汉字索引，使用UTF8编码，最多4字节 */
	unsigned char Msk[24];    /**< 汉字取模，该字号某汉字的取模数据 */
}typFNT_GB12; 

// 16*16 汉字字库结构体
typedef struct 
{
	unsigned char Index[4];	  /**< 汉字索引，使用UTF8编码，最多4字节 */
	unsigned char Msk[32];    /**< 汉字取模，该字号某汉字的取模数据 */
}typFNT_GB16; 

/** @brief 24*24汉字字库结构体 */
typedef struct 
{
	unsigned char Index[4];	  /**< 汉字索引，使用UTF8编码，最多4字节 */
	unsigned char Msk[72];    /**< 汉字取模，该字号某汉字的取模数据 */
}typFNT_GB24; 

/** @brief 32*32汉字字库结构体 */
typedef struct 
{
	unsigned char Index[4];	  /**< 汉字索引，使用UTF8编码，最多4字节 */
	unsigned char Msk[128];   /**< 汉字取模，该字号某汉字的取模数据 */
}typFNT_GB32; 


// 汉字字库
extern const typFNT_GB12 tfont12[2];
extern const typFNT_GB16 tfont16[2];
extern const typFNT_GB24 tfont24[21];
extern const typFNT_GB32 tfont32[2];


#endif
