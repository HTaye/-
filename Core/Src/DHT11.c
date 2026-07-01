#include "dht11.h"
#include "gpio.h"
#include "usart.h"
 
 
// 微秒级延时
void DHT11_DelayUs(uint32_t us)
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (delay--)
	{
		;
	}
}
 
// 初始化DHT11（配置GPIO为开漏输出）
DHT11_Status DHT11_Write(void) {
    // 1. 配置引脚为推挽输出（发送起始信号）
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 1. 主机发送起始信号（拉低18ms）
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_Delay(18);  // 实际需精确到18ms
    
    // 2. 拉高20-40us，切换为输入模式
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    DHT11_DelayUs(30);
 
    return DHT11_OK;
}
 
 
// 读取DHT11数据
DHT11_Status DHT11_Read(DHT11_Data *data) {
    uint8_t buffer[5] = {0};  // 40位数据：湿度整数/小数，温度整数/小数，校验和
    uint8_t checksum = 0;
    uint16_t timeout = 100;  // 最多等待 100us
 
    // 1. 配置引脚为上拉输入（等待DHT11响应）
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 2. 检测DHT11响应（40-50us低电平 + 40-50us高电平）
    timeout = 100;  // 超时时间约100us
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET) {
        DHT11_DelayUs(1);
        if (--timeout == 0) {
            return DHT11_ERROR_NO_RESPONSE;  // 等待低电平响应超时
        }
    }
     // 检测DHT11的响应低电平结束（40-50us低电平）
    timeout = 60;// 适当增加超时容限
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) {
        DHT11_DelayUs(1);
        if (--timeout == 0) {
            return DHT11_ERROR_TIMEOUT;  // 低电平持续时间过长
        }
    }
    // 检测DHT11的响应高电平结束（40-50us高电平）
    timeout = 60;// 适当增加超时容限
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET) {
        DHT11_DelayUs(1);
        if (--timeout == 0) {
            return DHT11_ERROR_TIMEOUT;  // 高电平持续时间过长
        }
    }
    
    // 3. 读取40位数据（每位：12-14us低电平 + 高电平长度决定0/1: 26-28us / 116-118us）
    for (int i = 0; i < 40; i++) {
        // 等待每位开始的12-14us低电平
        timeout = 30;  // 适当增加超时容限
        while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) {
            if (--timeout == 0) {
                return DHT11_ERROR_TIMEOUT;
            }
            DHT11_DelayUs(1);
        }
        
        // 测量高电平持续时间
        DHT11_DelayUs(30);  // 等待30us后采样
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET) {
            buffer[i / 8] |= (1 << (7 - (i % 8)));  // 高电平持续时间长，表示1
            // 等待高电平结束
            timeout = 100;
            while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET) {
                if (--timeout == 0) {
                    return DHT11_ERROR_TIMEOUT;
                }
                DHT11_DelayUs(1);
            }
        }
        // 如果是0，高电平已经结束（26-28us），不需要额外等待
    }
    
    // 4. 校验数据（前4字节之和 = 校验和）
    checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
    if (checksum != buffer[4]) {
        return DHT11_ERROR_CHECKSUM;
    }
    
    // 5. 填充结果（忽略小数部分）
    data->humidity = buffer[0];
    data->temperature = buffer[2];
    
    return DHT11_OK;
}