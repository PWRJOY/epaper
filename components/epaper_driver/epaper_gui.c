/**
 * @file epaper_gui.c
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
#include "epaper_gui.h"
#include "epaper_font.h"

/** 画布  */
PAINT Paint;


/**
 * @brief 函数功能：创建画布 
 *
 * @param image   像素数据
 * @param Width   宽
 * @param Height  高
 * @param Rotate  显示方向
 * @param Color   颜色
 */
void Paint_NewImage(uint8_t *image,uint16_t Width,uint16_t Height,uint16_t Rotate,uint16_t Color)
{
  Paint.Image = 0x00;
  Paint.Image = image;

  Paint.color = Color;  

  Paint.widthMemory = Width;
  Paint.heightMemory = Height;  

  Paint.widthByte = (EPD_W%4==0)?(EPD_W/4):(EPD_W/4+1); 
  Paint.heightByte = EPD_H;

  Paint.rotate = Rotate;
  if(Rotate==0||Rotate==180)    // 0或180度，宽高直接赋值
  {
    Paint.width = Width;
    Paint.height = Height;
  } 
  else                          // 90或270度，宽高调换
  {
    Paint.width=Height;
    Paint.height=Width; 
  }
}         

/**
 * @brief 函数功能：清除画布
 * 
 * @param Color 颜色
 */
void Paint_Clear(uint8_t Color)
{
  uint16_t X,Y;
  uint32_t Addr;
  for(Y=0;Y<Paint.heightByte;Y++) 
  {
    for(X=0;X<Paint.widthByte;X++) 
    {   
      Addr=X+Y*Paint.widthByte;
      Paint.Image[Addr]=(Color<<6)|(Color<<4)|(Color<<2)|Color; //将Paint.Image数组的所有数据清除，赋值Color
    }
  }
}


/**
 * @brief 函数功能：设置某个坐标像素点的颜色
 * 
 * @param Xpoint X坐标
 * @param Ypoint Y坐标
 * @param Color  标像素点的颜色 
 */
void Paint_SetPixel(uint16_t Xpoint,uint16_t Ypoint,uint16_t Color)
{
  uint16_t X, Y;
  uint32_t Addr;
  uint8_t Rdata;    

  switch(Paint.rotate)      // 计算旋转后的坐标
  {
      case 0:
          X=Paint.widthMemory-Ypoint-1;
          Y=Xpoint;
          break;
      case 90:
          X=Paint.widthMemory-Xpoint-1;
          Y=Paint.heightMemory-Ypoint-1;
          break;
      case 180:
          X=Ypoint;
          Y=Paint.heightMemory-Xpoint-1;
          break;
      case 270:
          X=Xpoint;
          Y=Ypoint;
          break;
        default:
            return;
    }
  Addr=X/4+Y*Paint.widthByte;
  Color = Color % 4;
  Rdata = Paint.Image[Addr];
  Rdata = Rdata & (~(0xC0 >> ((X % 4)*2)));
  Paint.Image[Addr]=Rdata|((Color<<6)>>((X%4)*2));
}


/**
 * @brief 函数功能：画直线
 * 
 * @param Xstart X坐标起始
 * @param Ystart Y坐标起始
 * @param Xend   X坐标结束 
 * @param Yend   Y坐标结束
 * @param Color  直线的颜色
 */
void EPD_DrawLine(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color)
{   
  uint16_t Xpoint, Ypoint;
  int dx, dy;
  int XAddway,YAddway;
  int Esp;
  char Dotted_Len;
  Xpoint = Xstart;
  Ypoint = Ystart;
  dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
  dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;
  XAddway = Xstart < Xend ? 1 : -1;
  YAddway = Ystart < Yend ? 1 : -1;
  Esp = dx + dy;
  Dotted_Len = 0;
  for (;;) {
        Dotted_Len++;
        Paint_SetPixel(Xpoint, Ypoint, Color);
        if (2 * Esp >= dy) {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx) {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

/**
 * @brief 函数功能：画矩形
 * 
 * @param Xstart X坐标起始
 * @param Ystart Y坐标起始
 * @param Xend   X坐标结束
 * @param Yend   Y坐标结束
 * @param Color  颜色
 * @param mode   矩形模式：\n
 * mode = 1,画实心矩形 \n
 * mode = 0,画空心矩形
 */
void EPD_DrawRectangle(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color,uint8_t mode)
{
  uint16_t i;

  if (mode)
  {
    for(i = Ystart; i < Yend; i++) 
    {
      EPD_DrawLine(Xstart,i,Xend,i,Color);
    }
  }
  else 
  {
    EPD_DrawLine(Xstart, Ystart, Xend, Ystart, Color);
    EPD_DrawLine(Xstart, Ystart, Xstart, Yend, Color);
    EPD_DrawLine(Xend, Yend, Xend, Ystart, Color);
    EPD_DrawLine(Xend, Yend, Xstart, Yend, Color);
  }
}

/**
 * @brief 函数功能：画圆
 * 
 * @param X_Center 圆心坐标X
 * @param Y_Center 圆心坐标Y
 * @param Radius   半径
 * @param Color    颜色
 * @param mode     模式：\n
 * mode = 1,画实心圆 \n
 * mode = 0,画空心圆
 */
void EPD_DrawCircle(uint16_t X_Center,uint16_t Y_Center,uint16_t Radius,uint16_t Color,uint8_t mode)
{
  int Esp, sCountY;
  uint16_t XCurrent, YCurrent;
  XCurrent = 0;
  YCurrent = Radius;
  Esp = 3 - (Radius << 1 );
    if (mode) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                Paint_SetPixel(X_Center + XCurrent, Y_Center + sCountY, Color);//1
                Paint_SetPixel(X_Center - XCurrent, Y_Center + sCountY, Color);//2
                Paint_SetPixel(X_Center - sCountY, Y_Center + XCurrent, Color);//3
                Paint_SetPixel(X_Center - sCountY, Y_Center - XCurrent, Color);//4
                Paint_SetPixel(X_Center - XCurrent, Y_Center - sCountY, Color);//5
                Paint_SetPixel(X_Center + XCurrent, Y_Center - sCountY, Color);//6
                Paint_SetPixel(X_Center + sCountY, Y_Center - XCurrent, Color);//7
                Paint_SetPixel(X_Center + sCountY, Y_Center + XCurrent, Color);
            }
            if ((int)Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    } else {        //Draw a hollow circle
        while (XCurrent <= YCurrent ) {
            Paint_SetPixel(X_Center + XCurrent, Y_Center + YCurrent, Color);//1
            Paint_SetPixel(X_Center - XCurrent, Y_Center + YCurrent, Color);//2
            Paint_SetPixel(X_Center - YCurrent, Y_Center + XCurrent, Color);//3
            Paint_SetPixel(X_Center - YCurrent, Y_Center - XCurrent, Color);//4
            Paint_SetPixel(X_Center - XCurrent, Y_Center - YCurrent, Color);//5
            Paint_SetPixel(X_Center + XCurrent, Y_Center - YCurrent, Color);//6
            Paint_SetPixel(X_Center + YCurrent, Y_Center - XCurrent, Color);//7
            Paint_SetPixel(X_Center + YCurrent, Y_Center + XCurrent, Color);//0
            if ((int)Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    }
}


/**
 * @brief 判断UTF-8字符的字节长度
 * 
 * @param first_byte UTF-8字符的第一个字节
 * @return int 返回UTF-8字符的字节长度，正常是1~4，0代表无效
 */
int get_utf8_char_length(uint8_t first_byte) {
  if ((first_byte & 0x80) == 0) {
      return 1;     /**<  单字节字符（ASCII）*/ 
  } else if ((first_byte & 0xE0) == 0xC0) {
      return 2;
  } else if ((first_byte & 0xF0) == 0xE0) {
      return 3;
  } else if ((first_byte & 0xF8) == 0xF0) {
      return 4;
  }
  return 0;  // 无效的 UTF - 8 起始字节
}

/**
 * @brief 函数功能：显示汉字
 * 
 * @param x 起始坐标X
 * @param y 起始坐标Y
 * @param s 汉字字符串地址
 * @param sizey 字号
 * @param color 颜色
 */
void EPD_ShowChinese(uint16_t x, uint16_t y, uint8_t *s, uint8_t sizey, uint16_t color) {

  while (*s != 0) {         // 逐个显示汉字

      int length = get_utf8_char_length(*s);    // 得到当前汉字的字节数
      if (length == 0) {
          printf("invalid UTF-8 coding.\n");
          break;
      }

      // 根据字号大小调用单个汉字显示函数
      if (sizey == 12) {
          EPD_ShowChinese12x12(x, y, s, sizey, color, length);
      } else if (sizey == 16) {
          EPD_ShowChinese16x16(x, y, s, sizey, color, length);
      } else if (sizey == 24) {
          EPD_ShowChinese24x24(x, y, s, sizey, color, length);
      } else if (sizey == 32) {
          EPD_ShowChinese32x32(x, y, s, sizey, color, length);
      } else {
          return;
      }

      s += length;  // 移动到下一个字符
      x += sizey;   // 向右移动一个位置来显示
  }
}

/*
// 例程是使用固定2字节的GBK编码，但是GBK编码的适用性不是很广
void EPD_ShowChinese(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color)
{
  while(*s!=0)
  {
    printf("s=%x,s+1=%x\n",*s,*(s+1));

    if(sizey==12) EPD_ShowChinese12x12(x,y,s,sizey,color);
    else if(sizey==16) EPD_ShowChinese16x16(x,y,s,sizey,color);
    else if(sizey==24) EPD_ShowChinese24x24(x,y,s,sizey,color);
    else if(sizey==32) EPD_ShowChinese32x32(x,y,s,sizey,color);
    else return;
    s+=2;       // 下一个汉字
    x+=sizey;   // 向右移动一个位置来显示
  }
}*/

/**
 * @brief 函数功能：显示汉字，12*12字号
 * 
 * @param x 起始坐标X
 * @param y 起始坐标Y
 * @param s 汉字
 * @param sizey 字号
 * @param color 颜色
 */
void EPD_ShowChinese12x12(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum)
{
  uint8_t i,j;
  uint16_t k;
  uint16_t HZnum;
  uint16_t TypefaceNum;
  uint16_t x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;                    
  HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);  
  for(k=0;k<HZnum;k++) 
  {
    // 根据utf8_charnum判断对应字节数是否匹配
    uint8_t match = 1;
    for (uint8_t m = 0; m < utf8_charnum; m++) 
    {
      if (tfont12[k].Index[m] != *(s + m)) 
      {
        match = 0;
        break;
      }
    }    
    if(match)
    {   
      for(i=0;i<TypefaceNum;i++)
      {
        for(j=0;j<8;j++)
        {  
            if(tfont12[k].Msk[i]&(0x01<<j))  Paint_SetPixel(x,y,color);
            x++;
            if((x-x0)==sizey)
            {
              x=x0;
              y++;
              break;
            }
        }
      }
    }            
    continue;  
  }
} 

/**
 * @brief 函数功能：显示汉字，16*16字号
 * 
 * @param x 起始坐标X
 * @param y 起始坐标Y
 * @param s 汉字
 * @param sizey 字号
 * @param color 颜色
 */
void EPD_ShowChinese16x16(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum)
{
  uint8_t i,j;
  uint16_t k;
  uint16_t HZnum;         //汉字数量
  uint16_t TypefaceNum;   //一个字符所占字节
  uint16_t x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;  //显示16字号需32个字节
  HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);    //计算字库中汉字数目

  for(k=0;k<HZnum;k++)    //遍历字库
  {
    // 根据utf8_charnum判断对应字节数是否匹配
    uint8_t match = 1;
    for (uint8_t m = 0; m < utf8_charnum; m++) 
    {
      if (tfont16[k].Index[m] != *(s + m)) 
      {
        match = 0;
        break;
      }
    }    
    if(match)
    {   
      for(i=0;i<TypefaceNum;i++)  //显示字符
      {
        for(j=0;j<8;j++){  
            if(tfont16[k].Msk[i]&(0x01<<j))  
              Paint_SetPixel(x,y,color);    //画点
            x++;
            if((x-x0)==sizey){    //换行
              x=x0;
              y++;
              break;
            }
        }
      }
    }            
    continue;  
  }
} 

/**
 * @brief 函数功能：显示汉字，24*24字号
 * 
 * @param x 起始坐标X
 * @param y 起始坐标Y
 * @param s 汉字
 * @param sizey 字号
 * @param color 颜色
 */
void EPD_ShowChinese24x24(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum)
{
  uint8_t i,j;
  uint16_t k;
  uint16_t HZnum;
  uint16_t TypefaceNum;
  uint16_t x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
  HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);  
  for(k=0;k<HZnum;k++) 
  {
    // 根据utf8_charnum判断对应字节数是否匹配
    uint8_t match = 1;
    for (uint8_t m = 0; m < utf8_charnum; m++) 
    {
      if (tfont24[k].Index[m] != *(s + m)) 
      {
        match = 0;
        break;
      }
    }    
    if(match)
    {   
      for(i=0;i<TypefaceNum;i++)
      {
        for(j=0;j<8;j++)
        {  
          if(tfont24[k].Msk[i]&(0x01<<j))  Paint_SetPixel(x,y,color);
          x++;
          if((x-x0)==sizey)
          {
            x=x0;
            y++;
            break;
          }
        }
      }
    }            
    continue;
  }
} 

/**
 * @brief 函数功能：显示汉字，32*32字号
 * 
 * @param x 起始坐标X
 * @param y 起始坐标Y
 * @param s 汉字
 * @param sizey 字号
 * @param color 颜色
 */
void EPD_ShowChinese32x32(uint16_t x,uint16_t y,uint8_t *s,uint8_t sizey,uint16_t color,uint8_t utf8_charnum)
{
  uint8_t i,j;
  uint16_t k;
  uint16_t HZnum;
  uint16_t TypefaceNum;
  uint16_t x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
  HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);  
  for(k=0;k<HZnum;k++) 
  {
    // 根据utf8_charnum判断对应字节数是否匹配
    uint8_t match = 1;
    for (uint8_t m = 0; m < utf8_charnum; m++) 
    {
      if (tfont32[k].Index[m] != *(s + m)) 
      {
        match = 0;
        break;
      }
    }    
    if(match)
    {   
      for(i=0;i<TypefaceNum;i++)
      {
        for(j=0;j<8;j++)
        {  
            if(tfont32[k].Msk[i]&(0x01<<j))  Paint_SetPixel(x,y,color);
            x++;
            if((x-x0)==sizey)
            {
              x=x0;
              y++;
              break;
            }
        }
      }
      break; 
    }
  }
}

/**
 * @brief 函数功能：显示字符
 * 
 * @param x 起始坐标X
 * @param y 起始坐标Y
 * @param chr 字符
 * @param size1 字号
 * @param fc 前景色
 * @param bc 背景色
 */
void EPD_ShowChar(uint16_t x,uint16_t y,uint16_t chr,uint16_t size1,uint8_t fc,uint8_t bc)
{
  uint16_t i,m,temp,size2,chr1;
  uint16_t x0,y0;
  x0=x,y0=y;
  if(size1==8)size2=6;
  else size2=(size1/8+((size1%8)?1:0))*(size1/2);  
  chr1=chr-' '; 
  for(i=0;i<size2;i++)
  {
    if(size1==16)
        {temp=asc2_1608[chr1][i];} 
    else if(size1==24)
        {temp=asc2_2412[chr1][i];} 
    else if(size1==48)
        {temp=asc2_4824[chr1][i];} 
    else return;
    for(m=0;m<8;m++)
    {
      if(temp&0x01)Paint_SetPixel(x,y,fc);
      else Paint_SetPixel(x,y,bc);
      temp>>=1;
      y++;
    }
    x++;
    if((size1!=8)&&((x-x0)==size1/2))
    {x=x0;y0=y0+8;}
    y=y0;
  }
}

/**
 * @brief 函数功能：显示字符串
 * 
 * @param x 起始坐标X
 * @param y 起始坐标Y
 * @param chr 字符串地址
 * @param size1 字号
 * @param fc 前景色
 * @param bc 背景色
 */
void EPD_ShowString(uint16_t x,uint16_t y,uint8_t *chr,uint16_t size1,uint8_t fc,uint8_t bc)
{
  while(*chr!='\0')
  {
    EPD_ShowChar(x,y,*chr,size1,fc,bc);
    chr++;
    x+=size1/2;
  }
}

/**
 * @brief m的n次方
 * 
 * @param m 底数
 * @param n 幂
 * @return uint32_t 
 */
uint32_t EPD_Pow(uint16_t m,uint16_t n)
{
  uint32_t result=1;
  while(n--)
  {
    result*=m;
  }
  return result;
}

/**
 * @brief 函数功能：显示数字
 * 
 * @param x     起始坐标X
 * @param y     起始坐标Y
 * @param num   数字
 * @param len   数字的位数
 * @param size1 字号
 * @param fc    前景色
 * @param bc    背景色
 */
void EPD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint16_t len,uint16_t size1,uint8_t fc,uint8_t bc)
{
  uint8_t t,temp,m=0;
  if(size1==8)m=2;
  for(t=0;t<len;t++)
  {
    temp=(num/EPD_Pow(10,len-t-1))%10;
      if(temp==0)
      {
        EPD_ShowChar(x+(size1/2+m)*t,y,'0',size1,fc,bc);
      }
      else 
      {
        EPD_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,fc,bc);
      }
  }
}

/**
 * @brief 函数功能：显示浮点数
 * 
 * @param x   起始坐标X
 * @param y   起始坐标Y
 * @param num 浮点数
 * @param len 长度，不包括小数点
 * @param pre 浮点数的精度
 * @param sizey 字号
 * @param fc 前景色
 * @param bc 背景色
 */
void EPD_ShowFloatNum1(uint16_t x,uint16_t y,float num,uint8_t len,uint8_t pre,uint8_t sizey,uint8_t fc,uint8_t bc)
{           
  uint8_t t,temp,sizex;
  uint16_t num1;
  sizex=sizey/2;
  num1=num*EPD_Pow(10,pre);
  for(t=0;t<len;t++)
  {
    temp=(num1/EPD_Pow(10,len-t-1))%10;
    if(t==(len-pre))
    {
      EPD_ShowChar(x+(len-pre)*sizex,y,'.',sizey,fc,bc);
      t++;
      len+=1;
    }
     EPD_ShowChar(x+t*sizex,y,temp+48,sizey,fc,bc);
  }
}

/**
 * @brief 函数功能：显示时间
 * 
 * @param x   起始坐标X
 * @param y   起始坐标Y
 * @param num 以浮点数的形式显示时间，12.05，就代表12:05
 * @param len 长度，不包括小数点
 * @param pre 浮点数的精度
 * @param sizey 字号
 * @param fc  前景色
 * @param bc  背景色
 */
void EPD_ShowWatch(uint16_t x,uint16_t y,float num,uint8_t len,uint8_t pre,uint8_t sizey,uint8_t fc,uint8_t bc)
{           
  uint8_t t,temp,sizex;
  uint16_t num1;
  sizex=sizey/2;
  num1=num*EPD_Pow(10,pre);
  for(t=0;t<len;t++)
  {
    temp=(num1/EPD_Pow(10,len-t-1))%10;
    if(t==(len-pre))
    {
      EPD_ShowChar(x+(len-pre)*sizex+(sizex/2-2),y-6,':',sizey,fc,bc);
      t++;
      len+=1;
    }
     EPD_ShowChar(x+t*sizex,y,temp+48,sizey,fc,bc);
  }
}

/**
 * @brief 颜色数据转换
 * 
 * @param color 
 * @return uint8_t 
 */
uint8_t PicDATA_Conversion(uint8_t color)
{
  uint8_t datas = 0;
  switch(color)
  {
    case 0x00:
      datas=WHITE;  
    break;    
    case 0x01:
      datas=YELLOW; 
    break;
    case 0x02:
      datas=RED; 
    break;    
    case 0x03:
      datas=BLACK; 
    break;      
    default:
    break;      
  }
   return datas;  
}

/**
 * @brief 函数功能：显示4色图片
 * 
 * @param x 
 * @param y 
 * @param sizex 
 * @param sizey 
 * @param BMP 
 */
void EPD_ShowFourColorPicture(uint16_t x,uint16_t y,uint16_t sizex,uint16_t sizey,const uint8_t BMP[])
{
  uint8_t color,data_H1,data_H2,data_L1,data_L2;
  uint16_t j=0;
  uint16_t i,temp,y0,TypefaceNum;
  TypefaceNum=sizex*(sizey/4+((sizey%4)?1:0));  // 计算图片数据总字节数
  y0=y;
  for(i=0;i<TypefaceNum;i++)      // 循环遍历图片数据数组
  {
    temp=BMP[j];
    color=temp;
    data_H1=PicDATA_Conversion(color>>6&0x03);
    Paint_SetPixel(x,y,data_H1);      
    data_H2=PicDATA_Conversion(color>>4&0x03);
    Paint_SetPixel(x,y+1,data_H2);
    data_L1=PicDATA_Conversion(color>>2&0x03);
    Paint_SetPixel(x,y+2,data_L1);
    data_L2=PicDATA_Conversion(color&0x03);
    Paint_SetPixel(x,y+3,data_L2);  
    
    y+=4;               // y += 4; 每次处理完一个字节的数据后，y 坐标增加 4
    if((y-y0)==sizey)   // 若 y - y0 等于图片的高度 sizey，说明已经到达图片的底部
    {
      y=y0;             // 将 y 坐标重置为起始 y 坐标 y0，并将 x 坐标增加 1
      x++;
    }
    j++;
  } 
}

// 将bitmap字模数据 绘制到画布缓冲区
void DrawBitmapToBuffer(uint16_t x, uint16_t y, const uint8_t *bitmap, uint8_t width, uint8_t height, uint16_t color)
{
    uint16_t x0 = x;
    uint16_t SizeNum = (width / 8 + ((width % 8) ? 1 : 0)) * height;    // 计算该字符需要的字节数

    for(uint8_t i=0;i<SizeNum;i++){             // 遍历字节数组
        for(uint8_t j=0;j<8;j++){               // 遍历每个字节的位
            if(bitmap[i]&(0x80>>j)){
                Paint_SetPixel(x, y, color);    //画点
            }  
            x++;
            if((x-x0)==width){                  //换行
                x=x0;
                y++;
            }
        }
    }
}

