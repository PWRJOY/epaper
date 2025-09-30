#ifndef LIS3DH_H
#define LIS3DH_H

#include "esp_err.h"
#include "driver/i2c.h"

// I2C配置
#define LIS3DH_I2C_MASTER_SCL_IO    14
#define LIS3DH_I2C_MASTER_SDA_IO    15
#define LIS3DH_I2C_MASTER_NUM       I2C_NUM_0   // 使用 I2C0，ESP32 支持 I2C_NUM_0 和 I2C_NUM_1
#define LIS3DH_I2C_MASTER_FREQ_HZ   50000      // 50kHz

#define LIS3DH_I2C_ADDR_0     0x18    // SA0引脚接GND时的I2C地址
#define LIS3DH_I2C_ADDR_1     0x19    // SA0引脚接VCC时的I2C地址

// LIS3DH寄存器地址
#define LIS3DH_REG_STATUS_REG_AUX   0x07
#define LIS3DH_REG_OUT_ADC1_L       0x08
#define LIS3DH_REG_OUT_ADC1_H       0x09
#define LIS3DH_REG_OUT_ADC2_L       0x0A
#define LIS3DH_REG_OUT_ADC2_H       0x0B
#define LIS3DH_REG_OUT_ADC3_L       0x0C
#define LIS3DH_REG_OUT_ADC3_H       0x0D
#define LIS3DH_REG_WHO_AM_I         0x0F
#define LIS3DH_REG_TEMP_CFG_REG     0x1F
#define LIS3DH_REG_CTRL_REG1        0x20
#define LIS3DH_REG_CTRL_REG2        0x21
#define LIS3DH_REG_CTRL_REG3        0x22
#define LIS3DH_REG_CTRL_REG4        0x23
#define LIS3DH_REG_CTRL_REG5        0x24
#define LIS3DH_REG_CTRL_REG6        0x25
#define LIS3DH_REG_REFERENCE        0x26
#define LIS3DH_REG_STATUS_REG       0x27
#define LIS3DH_REG_OUT_X_L          0x28
#define LIS3DH_REG_OUT_X_H          0x29
#define LIS3DH_REG_OUT_Y_L          0x2A
#define LIS3DH_REG_OUT_Y_H          0x2B
#define LIS3DH_REG_OUT_Z_L          0x2C
#define LIS3DH_REG_OUT_Z_H          0x2D
#define LIS3DH_REG_FIFO_CTRL_REG    0x2E
#define LIS3DH_REG_FIFO_SRC_REG     0x2F
#define LIS3DH_REG_INT1_CFG         0x30
#define LIS3DH_REG_INT1_SRC         0x31
#define LIS3DH_REG_INT1_THS         0x32
#define LIS3DH_REG_INT1_DURATION    0x33
#define LIS3DH_REG_INT2_CFG         0x34
#define LIS3DH_REG_INT2_SRC         0x35
#define LIS3DH_REG_INT2_THS         0x36
#define LIS3DH_REG_INT2_DURATION    0x37
#define LIS3DH_REG_CLICK_CFG        0x38
#define LIS3DH_REG_CLICK_SRC        0x39
#define LIS3DH_REG_CLICK_THS        0x3A
#define LIS3DH_REG_TIME_LIMIT       0x3B
#define LIS3DH_REG_TIME_LATENCY     0x3C
#define LIS3DH_REG_TIME_WINDOW      0x3D
#define LIS3DH_REG_ACT_THS          0x3E
#define LIS3DH_REG_ACT_DUR          0x3F

// 数据速率枚举
typedef enum {
    LIS3DH_DATA_RATE_POWER_DOWN = 0x00, 
    LIS3DH_DATA_RATE_1HZ                    = 0x01,     // HR / Normal / Low-power mode (1 Hz)
    LIS3DH_DATA_RATE_10HZ                   = 0x02,     // HR / Normal / Low-power mode (10 Hz)
    LIS3DH_DATA_RATE_25HZ                   = 0x03,     // HR / Normal / Low-power mode (25 Hz)
    LIS3DH_DATA_RATE_50HZ                   = 0x04,     // HR / Normal / Low-power mode (50 Hz)
    LIS3DH_DATA_RATE_100HZ                  = 0x05,     // HR / Normal / Low-power mode (100 Hz)
    LIS3DH_DATA_RATE_200HZ                  = 0x06,     // HR / Normal / Low-power mode (200 Hz)
    LIS3DH_DATA_RATE_400HZ                  = 0x07,     // HR / Normal / Low-power mode (400 Hz)
    LIS3DH_DATA_RATE_LOW_1600HZ             = 0x08,     // Low-power mode:1600Hz
    LIS3DH_DATA_RATE_HN_1344HZ_LOW_5376HZ   = 0x09      // HR / Normal mode:1344 Hz; Low-power mode:5376 Hz
} lis3dh_data_rate_t;

// 量程枚举
typedef enum {
    LIS3DH_FULL_SCALE_2G         = 0x00,
    LIS3DH_FULL_SCALE_4G         = 0x01,
    LIS3DH_FULL_SCALE_8G         = 0x02,
    LIS3DH_FULL_SCALE_16G        = 0x03
} lis3dh_full_scale_t;

// 传感器配置结构体
typedef struct {
    i2c_port_t i2c_port;         // I2C端口号
    uint8_t i2c_addr;            // I2C地址
    lis3dh_data_rate_t data_rate;// 数据速率
    lis3dh_full_scale_t full_scale; // 量程
} lis3dh_dev_t;

// 加速度数据结构体
typedef struct {
    int16_t x;                   // X轴原始数据
    int16_t y;                   // Y轴原始数据
    int16_t z;                   // Z轴原始数据
    float x_g;                   // X轴加速度 (g)
    float y_g;                   // Y轴加速度 (g)
    float z_g;                   // Z轴加速度 (g)
} lis3dh_accel_data_t;

/**
 * @brief 初始化LIS3DH传感器
 * 
 * @param dev 传感器设备结构体指针
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_init(lis3dh_dev_t *dev);

/**
 * @brief 读取LIS3DH传感器ID
 * 
 * @param dev 传感器设备结构体指针
 * @param id 读取到的ID存储地址
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_read_id(lis3dh_dev_t *dev, uint8_t *id);

/**
 * @brief 读取加速度数据
 * 
 * @param dev 传感器设备结构体指针
 * @param data 加速度数据存储结构体指针
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_read_accel(lis3dh_dev_t *dev, lis3dh_accel_data_t *data);

/**
 * @brief 读取温度数据
 * 
 * @param dev 传感器设备结构体指针
 * @param temp 温度数据存储地址 (°C)
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_read_temp(lis3dh_dev_t *dev, float *temp);

/**
 * @brief 配置传感器数据速率
 * 
 * @param dev 传感器设备结构体指针
 * @param rate 数据速率
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_set_data_rate(lis3dh_dev_t *dev, lis3dh_data_rate_t rate);

/**
 * @brief 配置传感器量程
 * 
 * @param dev 传感器设备结构体指针
 * @param scale 量程
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_set_full_scale(lis3dh_dev_t *dev, lis3dh_full_scale_t scale);

/**
 * @brief 软件复位传感器
 * 
 * @param dev 传感器设备结构体指针
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_soft_reset(lis3dh_dev_t *dev);

void lis3dh_init_task(void);

#endif // LIS3DH_H
