#ifndef __MPU6050_Reg_H
#define __MPU6050_Reg_H

// MPU6050设备地址
#define MPU6050_ADDR     0x68        // 7位地址
#define MPU6050_ADDR_W   0xD0        // 写地址(0x68 << 1)
#define MPU6050_ADDR_R   0xD1        // 读地址(0x68 << 1 | 0x01)

// 电源管理寄存器
#define MPU6050_PWR_MGMT_1   0x6B    // 电源管理1
#define MPU6050_PWR_MGMT_2   0x6C    // 电源管理2

// 采样率分频
#define MPU6050_SMPLRT_DIV   0x19    // 采样率分频寄存器

// 配置寄存器
#define MPU6050_CONFIG       0x1A    // 配置寄存器

// 陀螺仪配置
#define MPU6050_GYRO_CONFIG  0x1B    // 陀螺仪配置

// 加速度计配置
#define MPU6050_ACCEL_CONFIG 0x1C    // 加速度计配置

// FIFO使能
#define MPU6050_FIFO_EN      0x23    // FIFO使能

// I2C主控
#define MPU6050_I2C_MST_CTRL 0x24    // I2C主控控制

// 中断配置
#define MPU6050_INT_PIN_CFG  0x37    // 中断/旁路配置
#define MPU6050_INT_ENABLE   0x38    // 中断使能
#define MPU6050_INT_STATUS   0x3A    // 中断状态

// 数据寄存器
#define MPU6050_ACCEL_XOUT_H 0x3B    // 加速度计X轴高字节
#define MPU6050_ACCEL_XOUT_L 0x3C    // 加速度计X轴低字节
#define MPU6050_ACCEL_YOUT_H 0x3D    // 加速度计Y轴高字节
#define MPU6050_ACCEL_YOUT_L 0x3E    // 加速度计Y轴低字节
#define MPU6050_ACCEL_ZOUT_H 0x3F    // 加速度计Z轴高字节
#define MPU6050_ACCEL_ZOUT_L 0x40    // 加速度计Z轴低字节
#define MPU6050_TEMP_OUT_H   0x41    // 温度高字节
#define MPU6050_TEMP_OUT_L   0x42    // 温度低字节
#define MPU6050_GYRO_XOUT_H  0x43    // 陀螺仪X轴高字节
#define MPU6050_GYRO_XOUT_L  0x44    // 陀螺仪X轴低字节
#define MPU6050_GYRO_YOUT_H  0x45    // 陀螺仪Y轴高字节
#define MPU6050_GYRO_YOUT_L  0x46    // 陀螺仪Y轴低字节
#define MPU6050_GYRO_ZOUT_H  0x47    // 陀螺仪Z轴高字节
#define MPU6050_GYRO_ZOUT_L  0x48    // 陀螺仪Z轴低字节

// 信号路径复位
#define MPU6050_SIGNAL_PATH_RESET  0x68

// 用户控制
#define MPU6050_USER_CTRL    0x6A    // 用户控制

// 设备ID
#define MPU6050_WHO_AM_I     0x75    // 设备ID寄存器

// 陀螺仪量程定义
#define MPU6050_GYRO_FS_250  0x00    // ±250°/s
#define MPU6050_GYRO_FS_500  0x08    // ±500°/s
#define MPU6050_GYRO_FS_1000 0x10    // ±1000°/s
#define MPU6050_GYRO_FS_2000 0x18    // ±2000°/s

// 加速度计量程定义
#define MPU6050_ACCEL_FS_2   0x00    // ±2g
#define MPU6050_ACCEL_FS_4   0x08    // ±4g
#define MPU6050_ACCEL_FS_8   0x10    // ±8g
#define MPU6050_ACCEL_FS_16  0x18    // ±16g

// 数字低通滤波器带宽
#define MPU6050_DLPF_BW_260  0x00    // 260Hz
#define MPU6050_DLPF_BW_184  0x01    // 184Hz
#define MPU6050_DLPF_BW_94   0x02    // 94Hz
#define MPU6050_DLPF_BW_44   0x03    // 44Hz
#define MPU6050_DLPF_BW_21   0x04    // 21Hz
#define MPU6050_DLPF_BW_10   0x05    // 10Hz
#define MPU6050_DLPF_BW_5    0x06    // 5Hz

// 电源管理1寄存器位定义
#define MPU6050_PWR1_SLEEP   0x40    // 睡眠模式
#define MPU6050_PWR1_CLKSEL  0x07    // 时钟选择

// 数据结构体
typedef struct
{
    int16_t Accel_X;
    int16_t Accel_Y;
    int16_t Accel_Z;
    int16_t Temp;
    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
} MPU6050_Data;

#endif
