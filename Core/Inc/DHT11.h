/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2026-06-20 21:59:23
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2026-06-20 23:10:01
 * @FilePath: \MDK-ARMd:\Project\DHT11\Core\Inc\DHT11.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __DHT11_H
#define __DHT11_H
 
#include "stm32f1xx_hal.h"  // 根据实际MCU型号修改
#include <stdint.h>
#include <stdbool.h>
 
// 错误码定义
typedef enum {
    DHT11_OK = 0,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_CHECKSUM,
    DHT11_ERROR_NO_RESPONSE
} DHT11_Status;
 
// 温湿度数据结构体
typedef struct {
    float temperature;  // 温度（℃）
    float humidity;     // 湿度（%RH）
} DHT11_Data;
 
// 函数声明
DHT11_Status DHT11_Write(void);
DHT11_Status DHT11_Read(DHT11_Data *data);
void DHT11_DelayUs(uint32_t us);
#endif