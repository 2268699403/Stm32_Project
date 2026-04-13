#include "stm32f10x.h"  
#include "MPU6050.h"
#include "MPU6050_Reg.h"
#include "Delay.h"


/**
  *函    数：初始化GPIO，I2C等外设
  *参    数：无
  *返 回 值：无
  */
void MPU6050_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	I2C_InitTypeDef I2C_InitStructure;	
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;		
	I2C_Init(I2C2,&I2C_InitStructure);
	I2C_Cmd(I2C2,ENABLE);	
}

/**
  *函    数：I2C写入1个字节
  *参    数：设备地址DevAddr
  *参    数：寄存器地址RegAddr
  *参    数：要发送的数据Data
  *返 回 值：无
  */
void I2C_WriteByte(uint8_t DevAddr,uint8_t RegAddr,uint8_t Data)
{
	/*发送起始条件*/
	I2C_GenerateSTART(I2C2,ENABLE);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);
	
	/*发送设备地址*/
	I2C_Send7bitAddress(I2C2,DevAddr,I2C_Direction_Transmitter);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
	
	/*发送寄存器地址*/
	I2C_SendData(I2C2,RegAddr);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

	/*发送字节数据*/
	I2C_SendData(I2C2,Data);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	
	/*发送停止条件*/	
	I2C_GenerateSTOP(I2C2,ENABLE);
}

/**
  *函    数：I2C读取1个字节
  *参    数：设备地址DevAddr
  *参    数：寄存器地址RegAddr
  *返 回 值：接收到的字节数据
  */
uint8_t I2C_ReadByte(uint8_t DevAddr,uint8_t RegAddr)
{
	uint8_t Data = 0;
	
	/*发送起始条件*/
	I2C_GenerateSTART(I2C2,ENABLE);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);
	
	/*发送设备地址*/
	I2C_Send7bitAddress(I2C2,DevAddr,I2C_Direction_Transmitter);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
	
	/*发送寄存器地址*/
	I2C_SendData(I2C2,RegAddr);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);
	
	/*发送重复起始条件*/
	I2C_GenerateSTART(I2C2,ENABLE);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

	/*发送设备地址*/
	I2C_Send7bitAddress(I2C2,DevAddr,I2C_Direction_Receiver);
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);
	
	/*禁止ACK*/
	I2C_AcknowledgeConfig(I2C2,DISABLE);
	
	/*发送停止条件*/
	I2C_GenerateSTOP(I2C2, ENABLE);
	
	/*等待数据读取*/
	while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);
	Data = I2C_ReceiveData(I2C2);

	/*恢复ACK*/
	I2C_AcknowledgeConfig(I2C1,ENABLE);
	
	return Data;
}

/**
  *函    数：I2C读取多个字节
  *参    数：设备地址DevAddr
  *参    数：寄存器地址RegAddr
  *参    数：数据缓冲区Data
  *参    数：要读取的字节数Len
  *返 回 值：0成功 1失败
  */
uint8_t I2C_ReadBytes(uint8_t DevAddr, uint8_t RegAddr, uint8_t *Data, uint8_t Len)
{
    if(Len == 0) return 0;
    
    /*发送起始条件*/
    I2C_GenerateSTART(I2C2, ENABLE);
    while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);
    
    /*发送设备地址*/
    I2C_Send7bitAddress(I2C2, DevAddr, I2C_Direction_Transmitter);
    while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    
    /*发送寄存器地址*/
    I2C_SendData(I2C2, RegAddr);
    while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
    
    /*发送重复起始条件*/
    I2C_GenerateSTART(I2C2, ENABLE);
    while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);
    
    /*发送设备地址*/
    I2C_Send7bitAddress(I2C2, DevAddr, I2C_Direction_Receiver);
    while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);
    
    /*读取数据*/
    if(Len > 1)
    {
        I2C_AcknowledgeConfig(I2C2, ENABLE);
        
        for(uint8_t i = 0; i < Len - 1; i++)
        {
            while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);
            Data[i] = I2C_ReceiveData(I2C2);
        }
    }
    
    /*最后一个字节*/
    I2C_AcknowledgeConfig(I2C2, DISABLE);
    I2C_GenerateSTOP(I2C2, ENABLE);
    
    while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);
    Data[Len-1] = I2C_ReceiveData(I2C2);
    
    /*恢复ACK*/
    I2C_AcknowledgeConfig(I2C2, ENABLE);
    
    return 0;
}

/**
  *函    数：MPU6050配置函数
  *参    数：无
  *返 回 值：0成功 1失败
  */
uint8_t MPU6050_Config(void)
{
    /*检查设备ID*/
    if(I2C_ReadByte(MPU6050_ADDR_W, MPU6050_WHO_AM_I) != MPU6050_ADDR)
    {
        return 1;  // 设备不存在
    }
    
    /*唤醒设备*/
    I2C_WriteByte(MPU6050_ADDR_W, MPU6050_PWR_MGMT_1, 0x00);
    Delay_ms(10);
    
    /*设置采样率*/
    I2C_WriteByte(MPU6050_ADDR_W, MPU6050_SMPLRT_DIV, 0x07);  // 125Hz
    
    /*设置数字低通滤波器*/
    I2C_WriteByte(MPU6050_ADDR_W, MPU6050_CONFIG, 0x00);  // 禁用
    
    /*设置陀螺仪量程*/
    I2C_WriteByte(MPU6050_ADDR_W, MPU6050_GYRO_CONFIG, MPU6050_GYRO_FS_250);
    
    /*设置加速度计量程*/
    I2C_WriteByte(MPU6050_ADDR_W, MPU6050_ACCEL_CONFIG, MPU6050_ACCEL_FS_2);
    
    return 0;
}

/**
  *函    数:读取MPU6050原始数据
  *参    数：pData: 数据指针
  *返 回 值：0:成功 1:失败
  */
uint8_t MPU6050_GetRawData(MPU6050_Data *pData)
{
    uint8_t buffer[14];
    
    // 从0x3B开始读取14个字节
    if(I2C_ReadBytes(MPU6050_ADDR_W, MPU6050_ACCEL_XOUT_H, buffer, 14) != 0)
        return 1;
    
    // 组合成16位数据
    pData->Accel_X = (buffer[0] << 8) | buffer[1];
    pData->Accel_Y = (buffer[2] << 8) | buffer[3];
    pData->Accel_Z = (buffer[4] << 8) | buffer[5];
    pData->Temp    = (buffer[6] << 8) | buffer[7];
    pData->Gyro_X  = (buffer[8] << 8) | buffer[9];
    pData->Gyro_Y  = (buffer[10] << 8) | buffer[11];
    pData->Gyro_Z  = (buffer[12] << 8) | buffer[13];
    
    return 0;
}

