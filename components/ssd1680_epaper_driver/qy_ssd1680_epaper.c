#include "qy_ssd1680_epaper.h"
#include "qy_ssd1680_font.h"

// 4灰阶的波形驱动设置 Waveform Setting，可对照datasheet Figure 6-6
const unsigned char LUT_DATA_4Gray[159] = {											
0x40,	0x48,	0x80,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x8,	0x48,	0x10,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x2,	0x48,	0x4,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x20,	0x48,	0x1,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0xA,	0x19,	0x0,	0x3,	0x8,	0x0,	0x0,					
0x14,	0x1,	0x0,	0x14,	0x1,	0x0,	0x3,					
0xA,	0x3,	0x0,	0x8,	0x19,	0x0,	0x0,					
0x1,	0x0,	0x0,	0x0,	0x0,	0x0,	0x1,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	
0x0,	0x0,	0x0,			
0x22,	0x17,	0x41,	0x0,	0x32,	0x1C};		

// GPIO初始化
static void QY_SSD1680_GPIOInit(void)
{
    gpio_config_t io_conf = {};

    io_conf.pin_bit_mask =  1ULL << QY_SSD1680_GPIO_BUSY;
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask =  (1ULL << QY_SSD1680_GPIO_RST) | (1ULL << QY_SSD1680_GPIO_DC) | (1ULL << QY_SSD1680_GPIO_CS) | (1ULL << QY_SSD1680_GPIO_SCL) | (1ULL << QY_SSD1680_GPIO_SDA);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);    
}

// 忙等待
void QY_SSD1680_READBUSY(void)
{ 
    while(1){	 
        if(gpio_get_level(QY_SSD1680_GPIO_BUSY)==0) 
            break;
         vTaskDelay(10 / portTICK_PERIOD_MS);    
    }     
}

// 向总线上写一个字节
void QY_SSD1680_WR_Bus(unsigned char TxData)
{				   			 
	unsigned char TempData;
	unsigned char scnt;
	TempData=TxData;

    QY_SSD1680_EPD_SCL_Clr();  
	for(scnt=0;scnt<8;scnt++){ 
		if(TempData&0x80){
            QY_SSD1680_EPD_SDA_Set();
        }else{
            QY_SSD1680_EPD_SDA_Clr();
        }  
		QY_SSD1680_EPD_SCL_Set();  
	    QY_SSD1680_EPD_SCL_Clr();  
		TempData=TempData<<1;
    }
}

// 写命令
void QY_SSD1680_WR_REG(unsigned char cmd)
{
	QY_SSD1680_EPD_CS_Set();
	QY_SSD1680_EPD_CS_Clr();
	QY_SSD1680_EPD_DC_Clr();  // D/C#   0:command  1:data

	QY_SSD1680_WR_Bus(cmd);
	QY_SSD1680_EPD_CS_Set();
}

// 写数据
void QY_SSD1680_WR_DATA8(unsigned char data)
{
	QY_SSD1680_EPD_CS_Set();
	QY_SSD1680_EPD_CS_Clr();
	QY_SSD1680_EPD_DC_Set();  // D/C#   0:command  1:data

	QY_SSD1680_WR_Bus(data);
	QY_SSD1680_EPD_CS_Set();
}

// 硬件复位：RST引脚
void QY_SSD1680_HW_RESET(void)
{
    vTaskDelay(20 / portTICK_PERIOD_MS);
	QY_SSD1680_EPD_RES_Clr();     
	vTaskDelay(10 / portTICK_PERIOD_MS);  
	QY_SSD1680_EPD_RES_Set(); 			 
	vTaskDelay(10 / portTICK_PERIOD_MS); 
    QY_SSD1680_READBUSY();
}

// 无灰阶初始化
void QY_SSD1680_Init(void)
{
	int XStart,XEnd,YStart_L,YStart_H,YEnd_L,YEnd_H;	

	XStart=0x00;
	XEnd=(EPD_HEIGHT/8-1);

	YStart_L=(EPD_WIDTH-1)%256;
	YStart_H=(EPD_WIDTH-1)/256;
	YEnd_L=0x00;
	YEnd_H=0x00;
	
    QY_SSD1680_GPIOInit();          // GPIO初始化
	QY_SSD1680_HW_RESET();          // 硬件复位：RST引脚
	QY_SSD1680_READBUSY();
	QY_SSD1680_WR_REG(0x12);        // 软件复位
	QY_SSD1680_READBUSY();

	QY_SSD1680_WR_REG(0x01);        // 控制栅极数量和扫描方向     
	QY_SSD1680_WR_DATA8((EPD_WIDTH-1)%256);
	QY_SSD1680_WR_DATA8((EPD_WIDTH-1)/256);
	QY_SSD1680_WR_DATA8(0x00);

    QY_SSD1680_WR_REG(0x11);        // 数据输入模式设置        
	QY_SSD1680_WR_DATA8(0x01);      // Y减, X增、更新X方向
	
	QY_SSD1680_WR_REG(0x44);        // 设置 RAM X 的起始、结束位置 
	QY_SSD1680_WR_DATA8(XStart);		
	QY_SSD1680_WR_DATA8(XEnd);    	

	QY_SSD1680_WR_REG(0x45);        // 设置 RAM Y 的起始、结束位置          
	QY_SSD1680_WR_DATA8(YStart_L);   	
	QY_SSD1680_WR_DATA8(YStart_H);		
	QY_SSD1680_WR_DATA8(YEnd_L);		
	QY_SSD1680_WR_DATA8(YEnd_H);   

	QY_SSD1680_WR_REG(0x3C);        // BorderWavefrom
	QY_SSD1680_WR_DATA8(0x01);	

    QY_SSD1680_WR_REG(0x18);        // 温度传感器选择
	QY_SSD1680_WR_DATA8(0x80);      // 内部温度传感器

	QY_SSD1680_WR_REG(0x21);        // 显示更新控制
    QY_SSD1680_WR_DATA8(0x00);      // 红色RAM:正显，黑白RAM:正显
    QY_SSD1680_WR_DATA8(0x80);      // Available Source from S8 to S167	
	
	QY_SSD1680_WR_REG(0x4E);        // 设置 RAM X 地址
	QY_SSD1680_WR_DATA8(XStart);
	QY_SSD1680_WR_REG(0x4F);        // 设置 RAM Y 地址
	QY_SSD1680_WR_DATA8(YStart_L);
	QY_SSD1680_WR_DATA8(YStart_H);
	QY_SSD1680_READBUSY();

}

// 4灰阶初始化
void QY_SSD1680_Init_4GRAY(void)
{
    unsigned char i;
	int XStart,XEnd,YStart_L,YStart_H,YEnd_L,YEnd_H;	
	
	XStart=0x00;
	XEnd=(EPD_HEIGHT/8-1);
	
	YStart_L=0x00;
	YStart_H=0x00;	
	YEnd_L=(EPD_WIDTH-1)%256;
	YEnd_H=(EPD_WIDTH-1)/256;	
	
    QY_SSD1680_GPIOInit();          // GPIO初始化
	QY_SSD1680_HW_RESET();          // 硬件复位：RST引脚
	QY_SSD1680_READBUSY();
	QY_SSD1680_WR_REG(0x12);        // 软件复位
	QY_SSD1680_READBUSY();

	QY_SSD1680_WR_REG(0x01);        // 控制栅极数量和扫描方向     
	QY_SSD1680_WR_DATA8((EPD_WIDTH-1)%256);
	QY_SSD1680_WR_DATA8((EPD_WIDTH-1)/256);
	QY_SSD1680_WR_DATA8(0x00);    

    QY_SSD1680_WR_REG(0x11);        // 数据输入模式设置   
    QY_SSD1680_WR_DATA8(0x03);      // Y增加、X增加、更新X方向

	QY_SSD1680_WR_REG(0x3C);        //BorderWavefrom
	QY_SSD1680_WR_DATA8(0x00);	

	QY_SSD1680_WR_REG(0x21);        // 显示更新控制
    QY_SSD1680_WR_DATA8(0x00);      // 红色RAM:normal,黑白RAM:normal
    QY_SSD1680_WR_DATA8(0x80);      // Available Source from S8 to S167		

	QY_SSD1680_WR_REG(0x2C);         // 设置 VCOM 电压
	QY_SSD1680_WR_DATA8(LUT_DATA_4Gray[158]);    

	QY_SSD1680_WR_REG(0x3F);                    // Option for LUT end  
	QY_SSD1680_WR_DATA8(LUT_DATA_4Gray[153]);   // EOPT
	
	QY_SSD1680_WR_REG(0x03);                    // 设置栅极驱动电压   
	QY_SSD1680_WR_DATA8(LUT_DATA_4Gray[154]);   // VGH 

	QY_SSD1680_WR_REG(0x04);                    // 设置源极驱动电压      
	QY_SSD1680_WR_DATA8(LUT_DATA_4Gray[155]);   // VSH1   
	QY_SSD1680_WR_DATA8(LUT_DATA_4Gray[156]);   // VSH2   
	QY_SSD1680_WR_DATA8(LUT_DATA_4Gray[157]);   // VSL   
   
    QY_SSD1680_WR_REG(0x32);        // Write LUT register 
    for(i=0;i<152;i++){
        QY_SSD1680_WR_DATA8(LUT_DATA_4Gray[i]);
    }
        
    
	QY_SSD1680_WR_REG(0x44);        // 设置 RAM X 的起始、结束位置
	QY_SSD1680_WR_DATA8(XStart);		
	QY_SSD1680_WR_DATA8(XEnd);      	

	QY_SSD1680_WR_REG(0x45);        // 设置 RAM Y 的起始、结束位置              
	QY_SSD1680_WR_DATA8(YStart_L);   	
	QY_SSD1680_WR_DATA8(YStart_H);		
	QY_SSD1680_WR_DATA8(YEnd_L);		
	QY_SSD1680_WR_DATA8(YEnd_H); 
	
	QY_SSD1680_WR_REG(0x4E);        // 设置 RAM X 地址
	QY_SSD1680_WR_DATA8(XStart);
	QY_SSD1680_WR_REG(0x4F);        // 设置 RAM Y 地址
	QY_SSD1680_WR_DATA8(YStart_L);
	QY_SSD1680_WR_DATA8(YStart_H);
	QY_SSD1680_READBUSY();
	
}

// 全屏刷新并休眠
void QY_SSD1680_Update_and_DeepSleep(void)
{   
    QY_SSD1680_WR_REG(0x22);    // 显示控制2：设置序列
    QY_SSD1680_WR_DATA8(0xF7);  // 使能时钟、使能模拟、加载温度、显示模式1、失能模拟、失能晶振
    QY_SSD1680_WR_REG(0x20);    // 激活序列 
    QY_SSD1680_READBUSY();   
        
    QY_SSD1680_WR_REG(0x10);    // 进入深度睡眠模式1
    QY_SSD1680_WR_DATA8(0x01); 
}

// 局部刷新并休眠
void QY_SSD1680_Update_and_DeepSleep_Part(void)
{
	QY_SSD1680_WR_REG(0x22);    // 显示控制2：设置序列
	QY_SSD1680_WR_DATA8(0xFF);  // 使能时钟、使能模拟、加载温度、显示模式2、失能模拟、失能晶振
	QY_SSD1680_WR_REG(0x20);    // 激活序列
	QY_SSD1680_READBUSY(); 	

    QY_SSD1680_WR_REG(0x10);    // 进入深度睡眠模式1
    QY_SSD1680_WR_DATA8(0x01); 	
}

// 4灰阶刷新并休眠
void QY_SSD1680_Update_and_DeepSleep_4GRAY(void)
{   
    QY_SSD1680_WR_REG(0x22);    // 显示控制2：设置序列
    QY_SSD1680_WR_DATA8(0xC7);  // 使能时钟、使能模拟、显示模式1、失能模拟、失能晶振 
    QY_SSD1680_WR_REG(0x20);    // 激活序列
    QY_SSD1680_READBUSY();  
        
    QY_SSD1680_WR_REG(0x10);    // 进入深度睡眠模式1
    QY_SSD1680_WR_DATA8(0x01); 
}

// 清屏
void QY_SSD1680_Clear(void)
{
   unsigned int k;
    QY_SSD1680_WR_REG(0x24);        //写BW RAM，黑0白1
    for(k=0;k<ALLSCREEN_GRAGHBYTES;k++){
        QY_SSD1680_WR_DATA8(0xFF);
    }
	
    QY_SSD1680_WR_REG(0x26);        //写RED RAM，红1非红0
    for(k=0;k<ALLSCREEN_GRAGHBYTES;k++){
        QY_SSD1680_WR_DATA8(0xFF);
    }	
    QY_SSD1680_Update_and_DeepSleep();
}

// 显示
void QY_SSD1680_Display(const unsigned char *datas)
{
    unsigned int i;
    QY_SSD1680_WR_REG(0x24);        //写BW RAM，黑0白1
    for(i=0;i<ALLSCREEN_GRAGHBYTES;i++){               
        QY_SSD1680_WR_DATA8(*datas);
        datas++;
    }
    QY_SSD1680_Update_and_DeepSleep();	 
}

// 底图
void QY_SSD1680_Display_Part_BaseMap(const unsigned char * datas)
{
	unsigned int i;   
	const unsigned char  *datas_flag;   
	datas_flag=datas;
    QY_SSD1680_WR_REG(0x24);  // 写BW RAM，黑0白1
    for(i=0;i<ALLSCREEN_GRAGHBYTES;i++){               
        QY_SSD1680_WR_DATA8(*datas);
        datas++;
    }
    datas=datas_flag;
    QY_SSD1680_WR_REG(0x26);   // 写RED RAM，红1非红0
    for(i=0;i<ALLSCREEN_GRAGHBYTES;i++){               
        QY_SSD1680_WR_DATA8(*datas);
        datas++;
    }
    QY_SSD1680_Update_and_DeepSleep();      
}

// 局部刷新
void QY_SSD1680_Display_Part(int h_start,int v_start,const unsigned char * datas,int PART_WIDTH,int PART_HEIGHT,unsigned char mode)
{
	int i;  
	int vend,hstart_H,hstart_L,hend,hend_H,hend_L;
	
	v_start=v_start/8;				// X起始坐标		
	vend=v_start+PART_HEIGHT/8-1;   // X结束坐标
	
	h_start=295-h_start;            // Y起始坐标
		hstart_H=h_start/256;
		hstart_L=h_start%256;

	hend=h_start-PART_WIDTH+1;      // Y结束坐标
		hend_H=hend/256;
		hend_L=hend%256;		
	
    QY_SSD1680_WR_REG(0x44);        // 设置 RAM X 的起始、结束
	QY_SSD1680_WR_DATA8(v_start);   // RAM x 起始
	QY_SSD1680_WR_DATA8(vend);      // RAM x 结束

	QY_SSD1680_WR_REG(0x45);        // 设置 RAM Y 的起始、结束
	QY_SSD1680_WR_DATA8(hstart_L);  // RAM y 起始 低位
	QY_SSD1680_WR_DATA8(hstart_H);  // RAM y 起始 高位
	QY_SSD1680_WR_DATA8(hend_L);    // RAM y 结束 低位
	QY_SSD1680_WR_DATA8(hend_H);    // RAM y 结束 高位

	QY_SSD1680_WR_REG(0x4E);   		// 设置 RAM X 地址
	QY_SSD1680_WR_DATA8(v_start); 
	QY_SSD1680_WR_REG(0x4F);   		// 设置 RAM Y 地址
	QY_SSD1680_WR_DATA8(hstart_L);
	QY_SSD1680_WR_DATA8(hstart_H);
	QY_SSD1680_READBUSY();	
	
	QY_SSD1680_WR_REG(0x24);        // 写BW RAM，黑0白1
    for(i=0;i<PART_WIDTH*PART_HEIGHT/8;i++){                         
        if (mode==POS){             // 正显
            QY_SSD1680_WR_DATA8(*datas);
            datas++;
        }
        if (mode==NEG){             // 反显
            QY_SSD1680_WR_DATA8(~(*datas));
            datas++;
        }	
        if (mode==OFF){             // 清除（白色）
            QY_SSD1680_WR_DATA8(0xFF);
        }						
    }

}

// 4灰阶 BW RAM数据处理
static uint8_t In2bytes_Out1byte_RAM1(uint8_t data1,uint8_t data2)
{
    uint8_t i; 
    uint8_t TempData1,TempData2;
    uint8_t outdata=0x00;
    TempData1=data1;
    TempData2=data2;
	
    for(i=0;i<4;i++){ 
        outdata=outdata<<1;
        if( ((TempData1&0xC0)==0xC0) || ((TempData1&0xC0)==0x40))
           outdata=outdata|0x01;
        else 
          outdata=outdata|0x00;

        TempData1=TempData1<<2;
    }

    for(i=0;i<4;i++){ 
        outdata=outdata<<1;
         if((TempData2&0xC0)==0xC0||(TempData2&0xC0)==0x40)
           outdata=outdata|0x01;
        else 
          outdata=outdata|0x00;

        TempData2=TempData2<<2;
    }
    return outdata;
}

// 4灰阶 RED RAM数据处理
static uint8_t In2bytes_Out1byte_RAM2(uint8_t data1,uint8_t data2)
{
    uint8_t i; 
    uint8_t TempData1,TempData2;
    uint8_t outdata=0x00;
    TempData1=data1;
    TempData2=data2;
	
    for(i=0;i<4;i++){ 
        outdata=outdata<<1;
        if( ((TempData1&0xC0)==0xC0) || ((TempData1&0xC0)==0x80))
           outdata=outdata|0x01;
        else 
          outdata=outdata|0x00;

        TempData1=TempData1<<2;
    }

    for(i=0;i<4;i++){ 
        outdata=outdata<<1;
         if((TempData2&0xC0)==0xC0||(TempData2&0xC0)==0x80)
           outdata=outdata|0x01;
        else 
          outdata=outdata|0x00;

        TempData2=TempData2<<2;
    }
    return outdata;
}

// 全屏4灰阶刷新
void QY_SSD1680_Display_4GRAY(const unsigned char *datas)
{
    unsigned int i;
	uint8_t tempOriginal;   
	
    QY_SSD1680_WR_REG(0x24);   // 写BW RAM，黑0白1
    for(i=0;i<ALLSCREEN_GRAGHBYTES*2;i+=2){               
        tempOriginal= In2bytes_Out1byte_RAM1( *(datas+i),*(datas+i+1));
        QY_SSD1680_WR_DATA8(~tempOriginal); 
    }
	 
	QY_SSD1680_WR_REG(0x26);   // 写RED RAM，红1非红0
    for(i=0;i<ALLSCREEN_GRAGHBYTES*2;i+=2){               
        tempOriginal= In2bytes_Out1byte_RAM2( *(datas+i),*(datas+i+1));
        QY_SSD1680_WR_DATA8(~tempOriginal); 
    }
	 
   QY_SSD1680_Update_and_DeepSleep_4GRAY();    // 4灰阶刷新并休眠
}

// 屏幕测试函数
void test_qy_ssd1680_epaper(void)
{
    ESP_LOGI("SSD1680", "奇耘墨水屏测试");

    // 4灰阶图片测试
    QY_SSD1680_Init_4GRAY(); 
    QY_SSD1680_Display_4GRAY(gImage_4Gray11);   
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // 全刷测试
    QY_SSD1680_Init(); 
    // QY_SSD1680_Display(gImage_base);     // 只写黑白RAM，残影特别严重
    QY_SSD1680_Display_Part_BaseMap(gImage_base);   // 写黑白RAM,红色RAM
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // 局刷测试
    // QY_SSD1680_Init();       // 局刷不需要重新初始化，只需要硬件复位
    QY_SSD1680_HW_RESET();

    QY_SSD1680_Display_Part(0,0,gImage_Fahrenheit,47,16,POS);   // 第一行，显示华氏度
    QY_SSD1680_Display_Part(25,104,gImage_Celsius,47,16,NEG);   // 摄氏度，负显
    QY_SSD1680_Display_Part(29,32,gImage_minus1,33,64,OFF);     // 负号，不显示
    QY_SSD1680_Display_Part(0,32,gImage_num1,32,64,POS); 		// 数字1 
    QY_SSD1680_Display_Part(120,32,gImage_num1,33,64,POS);	    // 数字1

    QY_SSD1680_Update_and_DeepSleep_Part();      // 布局刷新，时序，显示模式2
    // QY_SSD1680_Update_and_DeepSleep();              // 全屏刷新，时序，显示模式1 

    ESP_LOGI("SSD1680", "奇耘墨水屏测试完毕");
}
