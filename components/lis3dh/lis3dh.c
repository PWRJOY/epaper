#include "lis3dh.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "lis3dh";

/**
 * @brief LIS3DH 专用 I2C 主机初始化（基于预定义宏配置）
 * @return esp_err_t 成功返回 ESP_OK，失败返回对应错误码（如端口无效、驱动安装失败）
 */
esp_err_t lis3dh_i2c_master_init(void) {
    // 配置 I2C 核心参数（基于用户宏定义）
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,                  // 工作模式：主机模式
        .sda_io_num = LIS3DH_I2C_MASTER_SDA_IO,   // SDA 引脚（12）
        .scl_io_num = LIS3DH_I2C_MASTER_SCL_IO,   // SCL 引脚（14）
        .sda_pullup_en = GPIO_PULLUP_ENABLE,      // 启用 SDA 内部上拉（减少外部电阻依赖）
        .scl_pullup_en = GPIO_PULLUP_ENABLE,      // 启用 SCL 内部上拉
        .master = {
            .clk_speed = LIS3DH_I2C_MASTER_FREQ_HZ  // I2C 时钟频率（400kHz，高速模式）
        },
        .clk_flags = 0,  // 时钟标志：默认（无需特殊配置）
    };

    // 应用 I2C 配置到指定端口
    esp_err_t ret = i2c_param_config(LIS3DH_I2C_MASTER_NUM, &i2c_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config I2C param, err: 0x%x", ret);
        return ret;
    }

    // 安装 I2C 驱动
    // 注：LIS3DH 通信为“主发从收/主收从发”，无需缓冲区，故设为0
    ret = i2c_driver_install(
        LIS3DH_I2C_MASTER_NUM, 
        I2C_MODE_MASTER, 
        0,  // rx_buf_len: 接收缓冲区大小（0=禁用）
        0,  // tx_buf_len: 发送缓冲区大小（0=禁用）
        0   // intr_alloc_flags: 中断分配标志（0=默认优先级）
    );
    if (ret != ESP_OK) {
        // 若驱动已安装（重复初始化），返回明确错误
        if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "I2C driver already installed (port: %d)", LIS3DH_I2C_MASTER_NUM);
            return ESP_OK;  // 兼容重复调用，避免初始化失败
        }
        ESP_LOGE(TAG, "Failed to install I2C driver, err: 0x%x", ret);
        return ret;
    }

    // 5. 初始化成功，打印配置信息（便于调试）
    ESP_LOGI(TAG, "I2C master init success!");
    ESP_LOGI(TAG, "Port: %d | SDA: %d | SCL: %d | Freq: %dkHz",
             LIS3DH_I2C_MASTER_NUM,
             LIS3DH_I2C_MASTER_SDA_IO,
             LIS3DH_I2C_MASTER_SCL_IO,
             LIS3DH_I2C_MASTER_FREQ_HZ / 1000);

    return ESP_OK;
}


/**
 * @brief 向LIS3DH写入一个字节
 * 
 * @param dev 传感器设备结构体指针
 * @param reg 寄存器地址
 * @param data 要写入的数据
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
static esp_err_t lis3dh_write_byte(lis3dh_dev_t *dev, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(dev->i2c_port, dev->i2c_addr, write_buf, 
                                      sizeof(write_buf), 1000 / portTICK_PERIOD_MS);
}

/**
 * @brief 从LIS3DH读取多个字节
 * 
 * @param dev 传感器设备结构体指针
 * @param reg 寄存器地址
 * @param data 读取数据存储缓冲区
 * @param len 要读取的字节数
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
static esp_err_t lis3dh_read_bytes(lis3dh_dev_t *dev, uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(dev->i2c_port, dev->i2c_addr, &reg, 1, 
                                        data, len, 1000 / portTICK_PERIOD_MS);
}

/**
 * @brief 初始化LIS3DH传感器
 * 
 * @param dev 传感器设备结构体指针
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t lis3dh_init(lis3dh_dev_t *dev) {
    esp_err_t ret;
    uint8_t id;

    // 检查设备是否存在
    ret = lis3dh_read_id(dev, &id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read device ID");
        return ret;
    } 
    if (id != 0x33) { // LIS3DH的WHO_AM_I值为0x33
        ESP_LOGE(TAG, "Invalid device ID: 0x%02X", id);
        return ESP_FAIL;
    }
    
    // 软件复位
    ret = lis3dh_soft_reset(dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset device");
        return ret;
    }
    
    // 配置数据速率
    ret = lis3dh_set_data_rate(dev, dev->data_rate);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set data rate");
        return ret;
    }
    
    // 配置量程
    ret = lis3dh_set_full_scale(dev, dev->full_scale);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set full scale");
        return ret;
    }
    
    // 配置CTRL_REG2 - 高通滤波器设置
    ret = lis3dh_write_byte(dev, LIS3DH_REG_CTRL_REG2, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure CTRL_REG2");
        return ret;
    }
    
    // 配置CTRL_REG3 - 中断配置
    ret = lis3dh_write_byte(dev, LIS3DH_REG_CTRL_REG3, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure CTRL_REG3");
        return ret;
    }
    
    // 配置CTRL_REG4 - 使能高分辨率模式
    uint8_t ctrl_reg4 = 0x80; // HR = 1 (高分辨率模式)
    ctrl_reg4 |= (dev->full_scale << 4); // 设置量程
    ret = lis3dh_write_byte(dev, LIS3DH_REG_CTRL_REG4, ctrl_reg4);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure CTRL_REG4");
        return ret;
    }
    
    // 配置CTRL_REG5 - 禁用FIFO，中断不锁存
    ret = lis3dh_write_byte(dev, LIS3DH_REG_CTRL_REG5, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure CTRL_REG5");
        return ret;
    }
    
    ESP_LOGI(TAG, "LIS3DH initialized successfully");
    return ESP_OK;
}

esp_err_t lis3dh_read_id(lis3dh_dev_t *dev, uint8_t *id) {
    return lis3dh_read_bytes(dev, LIS3DH_REG_WHO_AM_I, id, 1);
}

esp_err_t lis3dh_read_accel(lis3dh_dev_t *dev, lis3dh_accel_data_t *data) {
    uint8_t raw_data[6];
    esp_err_t ret = lis3dh_read_bytes(dev, LIS3DH_REG_OUT_X_L | 0x80, raw_data, 6);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read acceleration data");
        return ret;
    }
    
    // 组合16位数据 (左对齐，高分辨率模式下12位有效)
    data->x = (int16_t)(((uint16_t)raw_data[1] << 8) | raw_data[0]) >> 4;
    data->y = (int16_t)(((uint16_t)raw_data[3] << 8) | raw_data[2]) >> 4;
    data->z = (int16_t)(((uint16_t)raw_data[5] << 8) | raw_data[4]) >> 4;
    
    // 转换为g值
    float scale_factor;
    switch (dev->full_scale) {
        case LIS3DH_FULL_SCALE_2G:
            scale_factor = 1.0f / 1024.0f; // 每LSB对应1/1024 g
            break;
        case LIS3DH_FULL_SCALE_4G:
            scale_factor = 1.0f / 512.0f;  // 每LSB对应1/512 g
            break;
        case LIS3DH_FULL_SCALE_8G:
            scale_factor = 1.0f / 256.0f;  // 每LSB对应1/256 g
            break;
        case LIS3DH_FULL_SCALE_16G:
            scale_factor = 1.0f / 128.0f;  // 每LSB对应1/128 g
            break;
        default:
            scale_factor = 1.0f / 1024.0f;
            break;
    }
    
    data->x_g = data->x * scale_factor;
    data->y_g = data->y * scale_factor;
    data->z_g = data->z * scale_factor;
    
    return ESP_OK;
}

esp_err_t lis3dh_read_temp(lis3dh_dev_t *dev, float *temp) {
    esp_err_t ret;
    uint8_t temp_cfg;
    
    // 启用温度传感器
    ret = lis3dh_read_bytes(dev, LIS3DH_REG_TEMP_CFG_REG, &temp_cfg, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read TEMP_CFG_REG");
        return ret;
    }
    
    temp_cfg |= 0x80; // 启用温度传感器
    ret = lis3dh_write_byte(dev, LIS3DH_REG_TEMP_CFG_REG, temp_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable temperature sensor");
        return ret;
    }
    
    // 读取温度数据
    uint8_t raw_data[2];
    ret = lis3dh_read_bytes(dev, LIS3DH_REG_OUT_ADC3_L | 0x80, raw_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature data");
        return ret;
    }
    
    int16_t temp_raw = (int16_t)(((uint16_t)raw_data[1] << 8) | raw_data[0]);
    temp_raw >>= 6; // 温度数据是10位左对齐的
    
    // 转换为摄氏度 (参考数据手册)
    *temp = 25.0f + (temp_raw * 0.125f);
    
    return ESP_OK;
}

esp_err_t lis3dh_set_data_rate(lis3dh_dev_t *dev, lis3dh_data_rate_t rate) {
    uint8_t ctrl_reg1;
    esp_err_t ret = lis3dh_read_bytes(dev, LIS3DH_REG_CTRL_REG1, &ctrl_reg1, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL_REG1");
        return ret;
    }
    
    // 清除原有数据速率设置并设置新值
    ctrl_reg1 &= 0x0F;
    ctrl_reg1 |= (rate << 4);
    ctrl_reg1 |= 0x07; // 启用X, Y, Z轴
    
    ret = lis3dh_write_byte(dev, LIS3DH_REG_CTRL_REG1, ctrl_reg1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write CTRL_REG1");
        return ret;
    }
    
    dev->data_rate = rate;
    return ESP_OK;
}

esp_err_t lis3dh_set_full_scale(lis3dh_dev_t *dev, lis3dh_full_scale_t scale) {
    uint8_t ctrl_reg4;
    esp_err_t ret = lis3dh_read_bytes(dev, LIS3DH_REG_CTRL_REG4, &ctrl_reg4, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL_REG4");
        return ret;
    }
    
    // 清除原有量程设置并设置新值
    ctrl_reg4 &= 0xCF;
    ctrl_reg4 |= (scale << 4);
    
    ret = lis3dh_write_byte(dev, LIS3DH_REG_CTRL_REG4, ctrl_reg4);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write CTRL_REG4");
        return ret;
    }
    
    dev->full_scale = scale;
    return ESP_OK;
}

esp_err_t lis3dh_soft_reset(lis3dh_dev_t *dev) {
    esp_err_t ret = lis3dh_write_byte(dev, LIS3DH_REG_CTRL_REG5, 0x80); // BOOT=1
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initiate soft reset");
        return ret;
    }
    
    // 等待复位完成
    vTaskDelay(pdMS_TO_TICKS(10));
    
    return ESP_OK;
}


void lis3dh_init_task() {
    // 初始化I2C主机
    esp_err_t i2c_ret = lis3dh_i2c_master_init();
    if (i2c_ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed, can't init LIS3DH");
        return;
    }

    // 配置 LIS3DH 设备参数（关联 I2C 端口）
    lis3dh_dev_t lis3dh_dev = {
        .i2c_port = LIS3DH_I2C_MASTER_NUM,          // 绑定已初始化的 I2C0
        .i2c_addr = LIS3DH_I2C_ADDR_1,              // 传感器地址
        .data_rate = LIS3DH_DATA_RATE_100HZ,        // 数据速率：100Hz
        .full_scale = LIS3DH_FULL_SCALE_2G          // 量程：±2G
    };

    // 初始化LIS3DH
    esp_err_t ret = lis3dh_init(&lis3dh_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LIS3DH init failed, err: 0x%x", ret);
        return;
    }

    ESP_LOGI(TAG, "LIS3DH init task completed");
}
