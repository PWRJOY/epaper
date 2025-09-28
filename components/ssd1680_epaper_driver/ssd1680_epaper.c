#include "ssd1680_epaper.h"
#include "../epaper_driver/epaper_font.h"


SSD1680_PAINT SSD1680_Paint;

// 初始化 GPIO 引脚
void SSD1680_GPIOInit(void)
{
    gpio_config_t io_conf = {};

    io_conf.pin_bit_mask =  1ULL << SSD1680_GPIO_BUSY;
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask =  (1ULL << SSD1680_GPIO_RST) | (1ULL << SSD1680_GPIO_DC) | (1ULL << SSD1680_GPIO_CS) | (1ULL << SSD1680_GPIO_SCL) | (1ULL << SSD1680_GPIO_SDA);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);    
}

// 向总线上写入一个字节的数据
void SSD1680_WR_Bus(uint8_t dat)  
{
	uint8_t i;

	SSD1680_EPD_CS_Clr();           // 拉低CS

	for(i=0;i<8;i++){               // 1个字节的8个位，从最高位开始传输
		SSD1680_EPD_SCL_Set();      // 时钟SCL拉低
    if(dat&0x80){                   // 显示数据SDA
			SSD1680_EPD_SDA_Set();      
		}else{
			SSD1680_EPD_SDA_Clr();
		}
		SSD1680_EPD_SCL_Set();      // 制造SCL上升沿
		dat<<=1;
	}

	SSD1680_EPD_CS_Set();	        // 拉高CS    
}

// 向墨水屏写入指令-Command
void SSD1680_WR_REG(uint8_t reg)  
{
	SSD1680_EPD_DC_Clr();
	SSD1680_WR_Bus(reg);
	SSD1680_EPD_DC_Set();  
}

// 向墨水屏写入 8 位数据-Parameter
void SSD1680_WR_DATA8(uint8_t dat)
{
    SSD1680_EPD_DC_Set();
    SSD1680_WR_Bus(dat);
    SSD1680_EPD_DC_Set();       // 其实这句是可以省略的，但是对应上面写指令的代码，没有去掉
}

// 读取并等待墨水屏的忙碌状态
void SSD1680_READBUSY(void)
{
  while(1)
  {
    if(gpio_get_level(SSD1680_GPIO_BUSY)==0)
    {
      break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);    
  }
}

// 对墨水屏进行硬件复位操作
void SSD1680_HW_RESET(void)
{
    vTaskDelay(100 / portTICK_PERIOD_MS);
    SSD1680_EPD_RES_Clr();
    vTaskDelay(20 / portTICK_PERIOD_MS);  // 至少10ms
    SSD1680_EPD_RES_Set();
    vTaskDelay(20 / portTICK_PERIOD_MS);  // 至少10ms
    SSD1680_READBUSY();
    SSD1680_WR_REG(0x12);  //SWRESET
    SSD1680_READBUSY();

}

// 更新墨水屏显示内容
void SSD1680_Update(void)
{
  SSD1680_WR_REG(0x22); //Display Update Control
  SSD1680_WR_DATA8(0xF7);    
  SSD1680_WR_REG(0x20);  //Activate Display Update Sequence
  SSD1680_READBUSY();       
}

void SSD1680_Init(void)
{
	SSD1680_GPIOInit();
    SSD1680_HW_RESET(); 

    SSD1680_WR_REG(0x01); //Driver output control      
    SSD1680_WR_DATA8(0x27);
    SSD1680_WR_DATA8(0x01);
    SSD1680_WR_DATA8(0x01);

    SSD1680_WR_REG(0x11); //data entry mode       
    SSD1680_WR_DATA8(0x01);

    SSD1680_WR_REG(0x44); //set Ram-X address start/end position   
    SSD1680_WR_DATA8(0x00);
    SSD1680_WR_DATA8(0x0F);    //0x0F-->(15+1)*8=128

    SSD1680_WR_REG(0x45); //set Ram-Y address start/end position          
    SSD1680_WR_DATA8(0x27);   //0xF9-->(249+1)=250
    SSD1680_WR_DATA8(0x01);
    SSD1680_WR_DATA8(0x00);
    SSD1680_WR_DATA8(0x00); 

    SSD1680_WR_REG(0x3C); //BorderWavefrom
    SSD1680_WR_DATA8(0x05);  
        

    SSD1680_WR_REG(0x21); //  Display update control
    SSD1680_WR_DATA8(0x00);  
    SSD1680_WR_DATA8(0x80);  

    SSD1680_WR_REG(0x18); //Read built-in temperature sensor
    SSD1680_WR_DATA8(0x80); 

    SSD1680_WR_REG(0x4E);   // set RAM x address count to 0;
    SSD1680_WR_DATA8(0x00);
    SSD1680_WR_REG(0x4F);   // set RAM y address count to 0X199;    
    SSD1680_WR_DATA8(0x27);
    SSD1680_WR_DATA8(0x01);

    SSD1680_READBUSY();	
}


void SSD1680_Paint_NewImage(uint8_t *image,uint16_t Width,uint16_t Height,uint16_t Rotate,uint16_t Color)
{
    SSD1680_Paint.Image = 0x00;
    SSD1680_Paint.Image = image;

    SSD1680_Paint.WidthMemory = Width;
    SSD1680_Paint.HeightMemory = Height;
    SSD1680_Paint.Color = Color;    
    SSD1680_Paint.WidthByte = (Width % 8 == 0)? (Width / 8 ): (Width / 8 + 1);
    SSD1680_Paint.HeightByte = Height;     
    SSD1680_Paint.Rotate = Rotate;
    if(Rotate == SSD1680_ROTATE_0 || Rotate == SSD1680_ROTATE_180) {
        SSD1680_Paint.Width = Width;
        SSD1680_Paint.Height = Height;
    } else {
        SSD1680_Paint.Width = Height;
        SSD1680_Paint.Height = Width;
    }
}

void SSD1680_Paint_SetPixel(uint16_t Xpoint,uint16_t Ypoint,uint16_t Color)
{
	uint16_t X, Y;
	uint32_t Addr;
	uint8_t Rdata;
    switch(SSD1680_Paint.Rotate) {
        case 0:
            X = SSD1680_Paint.WidthMemory - Ypoint - 1;
            Y = Xpoint;		
            break;
        case 90:
            X = SSD1680_Paint.WidthMemory - Xpoint - 1;
            Y = SSD1680_Paint.HeightMemory - Ypoint - 1;
            break;
        case 180:
            X = Ypoint;
            Y = SSD1680_Paint.HeightMemory - Xpoint - 1;
            break;
        case 270:
            X = Xpoint;
            Y = Ypoint;
            break;
        default:
            return;
    }
	Addr = X / 8 + Y * SSD1680_Paint.WidthByte;
    Rdata = SSD1680_Paint.Image[Addr];
    if(Color == SSD1680_BLACK)
    {    
			SSD1680_Paint.Image[Addr] = Rdata & ~(0x80 >> (X % 8)); //置0
		}
    else
        SSD1680_Paint.Image[Addr] = Rdata | (0x80 >> (X % 8));   //置1  
}

// 清屏，用指定数据填充墨水屏整个显示区域
void SSD1680_Clear(uint16_t Color)
{
	uint16_t X,Y;
	uint32_t Addr;
    for (Y = 0; Y < SSD1680_Paint.HeightByte; Y++) {
        for (X = 0; X < SSD1680_Paint.WidthByte; X++) {//8 pixel =  1 byte
            Addr = X + Y*SSD1680_Paint.WidthByte;
            SSD1680_Paint.Image[Addr] = Color;
        }
    }   
}

// 画点
void SSD1680_DrawPoint(uint16_t Xpoint,uint16_t Ypoint,uint16_t Color)
{
    //SSD1680_Paint_SetPixel(Xpoint, Ypoint, Color);   
    SSD1680_Paint_SetPixel(Xpoint-1, Ypoint-1, Color);  
}

// 画线
void SSD1680_DrawLine(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color)
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
        SSD1680_DrawPoint(Xpoint, Ypoint, Color);
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

//  画矩形
void SSD1680_DrawRectangle(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color,uint8_t mode)
{
	uint16_t i;
    if (mode){
        for(i = Ystart; i < Yend; i++) {
            SSD1680_DrawLine(Xstart,i,Xend,i,Color);
        }
    }else {
        SSD1680_DrawLine(Xstart, Ystart, Xend, Ystart, Color);
        SSD1680_DrawLine(Xstart, Ystart, Xstart, Yend, Color);
        SSD1680_DrawLine(Xend, Yend, Xend, Ystart, Color);
        SSD1680_DrawLine(Xend, Yend, Xstart, Yend, Color);
	}
}

// 画圆
void SSD1680_DrawCircle(uint16_t X_Center,uint16_t Y_Center,uint16_t Radius,uint16_t Color,uint8_t mode)
{
	uint16_t Esp, sCountY;
	uint16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;
    Esp = 3 - (Radius << 1 );
    if (mode) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                SSD1680_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color);//1
                SSD1680_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color);//2
                SSD1680_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color);//3
                SSD1680_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color);//4
                SSD1680_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color);//5
                SSD1680_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color);//6
                SSD1680_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color);//7
                SSD1680_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color);
            }
            if ((int)Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    } else { //Draw a hollow circle
        while (XCurrent <= YCurrent ) {
            SSD1680_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color);//1
            SSD1680_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color);//2
            SSD1680_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color);//3
            SSD1680_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color);//4
            SSD1680_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color);//5
            SSD1680_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color);//6
            SSD1680_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color);//7
            SSD1680_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color);//0
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

// 显示一个字符
void SSD1680_ShowChar(uint16_t x,uint16_t y,uint16_t chr,uint16_t size1,uint16_t color)
{
	uint16_t i,m,temp,size2,chr1;
	uint16_t x0,y0;
	x+=1,y+=1,x0=x,y0=y;
	if(size1==8)size2=6;
	else size2=(size1/8+((size1%8)?1:0))*(size1/2);
	chr1=chr-' ';
	for(i=0;i<size2;i++)
	{
		if(size1==8)        {temp=asc2_0806[chr1][i];}
		else if(size1==12)  {temp=asc2_1206[chr1][i];}
		else if(size1==16)  {temp=asc2_1608[chr1][i];}
		else if(size1==24)  {temp=asc2_2412[chr1][i];}
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)SSD1680_DrawPoint(x,y,color);
			else SSD1680_DrawPoint(x,y,!color);
			temp>>=1;
			y++;
		}
		x++;
		if((size1!=8)&&((x-x0)==size1/2))
		{x=x0;y0=y0+8;}
		y=y0;
  }
}

// 显示字符串
void SSD1680_ShowString(uint16_t x,uint16_t y,uint8_t *chr,uint16_t size1,uint16_t color)
{
	while(*chr!='\0')
	{
		SSD1680_ShowChar(x,y,*chr,size1,color);
		chr++;
		x+=size1/2;
  }
}


void SSD1680_Display(unsigned char *Image)
{
    unsigned int Width, Height,i,j;
	uint32_t k=0;
    Width = 296;
    Height = 16;
    SSD1680_WR_REG(0x24);
    for ( j = 0; j < Height; j++) {
      for ( i = 0; i < Width; i++) {
        SSD1680_WR_DATA8(Image[k]);
		k++;
      }
    }
    SSD1680_Update();		 
}

// 测试代码
// uint8_t  SSD1680_Image_BW[4736];

// void test_ssd1680_epaper(void)
// {
//     ESP_LOGI("SSD1680", "初始化");
//     SSD1680_Init();                                   // 墨水屏初始化
//     SSD1680_Paint_NewImage(SSD1680_Image_BW,SSD1680_W,SSD1680_H,0,SSD1680_WHITE);  // 创建画布，画布数据存放于数组ImageBW
//     SSD1680_Clear(SSD1680_WHITE);                           // 画布清屏   
//     SSD1680_ShowString(10,10,(uint8_t*)"Hello World!",24,SSD1680_BLACK);
//     SSD1680_Display(SSD1680_Image_BW);                         // 将画布内容发送到SRAM
    
//     ESP_LOGI("SSD1680", "显示完毕");
// }